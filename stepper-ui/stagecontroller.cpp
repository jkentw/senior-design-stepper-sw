#include "config.hpp"

#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <cstdio>
#include <fcntl.h>
#include <ctime>

#include "stagecontroller.h"


#ifdef DEBUG_MODE_GLOBAL
    #define DEBUG_MODE_I2C
#endif

namespace stage_controller {

//configuration constants
static const int BUF_SIZE = 8;
static const int I2C_ADDRESS = 0x0F;
static const int MAGIC = 0x5A;
static const int CHECK_VALUE = 0xFF;

//state information
static int i2cFd = -1;
static int writeIndex = 0; //index of next available frame slot
static int readIndex = 0; //index of next frame to send
static int bufferLength = 0;
static int lastCommand = -1;

static unsigned emulatedX = 0;
static unsigned emulatedY = 0;
static unsigned emulatedTargetX = 0;
static unsigned emulatedTargetY = 0;
static time_t emulatedCompleteTime = 0;

struct frame buffer[BUF_SIZE], response;

//I2C setup
bool isOpen();
bool openI2c();
void closeI2c();
static void printErrorInfo(int errsv);

//data frame management
int getBufferLength();
void clearBuffer();
void createFrame(struct frame *dest, CommandID command, __u32 data);
bool addFrame(CommandID command, __u32 data);
bool addFrame(const struct frame *dest);
bool sendFrame(const struct frame *frame);
bool sendNextFrame();
int readResponse(__u32 *data, __u8 *stageStatus);
int processFrame(const struct frame *frame, __u32 *data, __u8 *stageStatus, int expectedCommand);

static __u8 calcChecksum(const struct frame *frame);
static void emulate(const struct frame *frame);

bool isOpen() {
    return i2cFd != -1;
}

bool openI2c() {
    int adapter_nr = 1; //should dynamically determine this
    char fname[20];

    if(isOpen())
        return true;

    snprintf(fname, 19, "/dev/i2c-%d", adapter_nr);
    i2cFd = open(fname, O_RDWR);

    if(i2cFd == -1) {
#ifdef DEBUG_MODE_I2C
        printf("device file '%-19s' not found\n", fname);
        fflush(stdout);
#endif
        return false;
    }

    if(ioctl(i2cFd, I2C_SLAVE, I2C_ADDRESS) == -1) {
#ifdef DEBUG_MODE_I2C
        printf("error configuring device address\n");
        fflush(stdout);
#endif
        closeI2c();
        return false;
    }

#ifdef DEBUG_MODE_I2C
    printf("I2C communication established.\n");
    fflush(stdout);
#endif
    return true;
}

void closeI2c() {
    if(isOpen()) {
        if(close(i2cFd) == -1) {
#ifdef DEBUG_MODE_I2C
            printf("Could not terminate I2C communication.\n");
            fflush(stdout);
#endif
        }
        else {
            i2cFd = -1;
#ifdef DEBUG_MODE_I2C
    printf("I2C communication terminated.\n");
    fflush(stdout);
#endif
        }
    }
}

//calculates what checksum field should be based frame data
__u8 calcChecksum(const struct frame *frame) {
    __u8 sum = 0;
    __u8 *bytes = (__u8 *) frame;

    for(int i = 0; i < 7; i++) {
        sum += bytes[i];
    }

    return CHECK_VALUE - sum;
}

int getBufferLength() {
    return bufferLength;
}

void clearBuffer() {
    readIndex = (readIndex + bufferLength) % BUF_SIZE; //buffer is circular
    bufferLength = 0;
}

void createFrame(struct frame *dest, CommandID command, __u32 data) {
    dest->magic = MAGIC;
    dest->command = (__u8) command;
    dest->data[0] = data & 0xFF; //store as little endian value
    dest->data[1] = data >> 8 & 0xFF; //store as little endian value
    dest->data[2] = data >> 16 & 0xFF; //store as little endian value
    dest->data[3] = data >> 24 & 0xFF; //store as little endian value
    dest->status = 0;
    dest->checksum = calcChecksum(dest);
}

//creates frame and adds it to the circular queue
bool addFrame(CommandID command, __u32 data) {
    if(getBufferLength() < BUF_SIZE) {
        struct frame *thisFrame = &buffer[writeIndex];
        createFrame(thisFrame, command, data);
#ifdef DEBUG_MODE_I2C
        printf("Added command of ID #%d at index %d\n", thisFrame->command, writeIndex);
#endif
        writeIndex = (writeIndex + 1) % BUF_SIZE; //buffer is circular
        bufferLength++;
        return true;
    }
    else
        return false;
}

//Adds an existing frame to the circular queue. Frame assumed to be valid
bool addFrame(const struct frame *dest) {
    if(getBufferLength() < BUF_SIZE) {
#ifdef DEBUG_MODE_I2C
        printf("Buffer length is now %d\n", getBufferLength());
#endif
        __u8 *thisFrameBytes = (__u8 *) &buffer[writeIndex];

        //deep copy
        for(int i = 0; i < 8; i++) {
            thisFrameBytes[i] = ((__u8 *) dest)[i];
        }

        writeIndex = (writeIndex + 1) % BUF_SIZE; //buffer is circular
        bufferLength++;
        return true;
    }
    else
        return false;
}

//sends arbitrary frame without advancing buffer
bool sendFrame(const struct frame *frame) {
#ifdef EMULATION_MODE_I2C
    __s32 result = 8;
    emulate(frame);
#else
    __s32 result = write(i2cFd, (__u8 *) frame, 8);
#endif

    int errsv = errno;
    if(result == -1) {
#ifdef DEBUG_MODE_I2C
        printf("I2C write failed:\t\n");
        printErrorInfo(errsv);
#endif
    }
    else {
#ifdef DEBUG_MODE_I2C
        printf("I2C write successful\n");
#endif
        lastCommand = frame->command; //update this so we know which response to look for
    }

    return result == 8;
}

//sends next frame data from queue and advances buffer
bool sendNextFrame() {
    if(getBufferLength() > 0) {
        if(!sendFrame(&buffer[readIndex]))
            return false;
#ifdef DEBUG_MODE_I2C
        printf("Sent command of ID #%d from index %d\n", buffer[readIndex].command, readIndex);
#endif
        readIndex = (readIndex + 1) % BUF_SIZE; //buffer is circular
        bufferLength--;
        return true;
    }

    return false;
}

//deletes next frame data from queue without sending and advances buffer
bool deleteNextFrame() {
    if(getBufferLength() > 0) {
#ifdef DEBUG_MODE_I2C
        printf("Removed command of ID #%d from index %d\n", buffer[readIndex].command, readIndex);
#endif
        readIndex = (readIndex + 1) % BUF_SIZE; //buffer is circular
        bufferLength--;
        return true;
    }

    return false;
}

//Returns status code. Set expectedCommand to -1 if check is unnecessary.
int processFrame(const struct frame *frame, __u32 *data, __u8 *stageStatus, int expectedCommand) {
    int status = GOOD;
    __u8 stageStatusTmp;

    //error checking
    if(frame->magic != MAGIC)
        status |= ERROR_MAGIC;

    if(expectedCommand != -1 && frame->command != expectedCommand)
        status |= ERROR_UNEXPECTED_COMMAND;

    if(frame->command >= 8)
        status |= ERROR_UNKNOWN_COMMAND;

    if(frame->checksum != calcChecksum(frame))
        status |= ERROR_CHECKSUM;

    //set status field
    switch(frame->status) {
    case 0:
        stageStatusTmp = STAGE_NOT_READY;
        break;
    case 1:
        stageStatusTmp = STAGE_MOVING;
        break;
    case 2:
        stageStatusTmp = STAGE_IN_POSITION;
        break;
    default:
        stageStatusTmp = -1;
        status |= ERROR_UNKNOWN_STATUS;
    }

    if(stageStatus != NULL) {
        *stageStatus = stageStatusTmp;
    }

#ifdef DEBUG_MODE_I2C
    if(status != GOOD) {
        printf("Error in received packet: code %08X\n", status);
    }
#endif

    if(data != NULL) {
        *data = frame->data[3] << 24 | frame->data[2] << 16 | frame->data[1] << 8 | frame->data[0];
    }
    return status;
}

//returns status code
int readResponse(__u32 *data, __u8 *stageStatus) {
#ifdef EMULATION_MODE_I2C
    __s32 result = 0;
    emulate(nullptr);
#else
    __s32 result = read(i2cFd, (__u8 *) &response, 8);
#endif
    int errsv = errno;
    if(result == -1) {
#ifdef DEBUG_MODE_I2C
        printf("I2C write failed:\t\n");
        printErrorInfo(errsv);
#endif
        return ERROR_READ;
    }
    else {
#ifdef DEBUG_MODE_I2C
        printf("I2C read successful: %02X %02X %02X%02X%02X%02X %02X %02X \n",
               response.magic, response.command,
               response.data[3], response.data[2], response.data[1], response.data[0],
               response.status, response.checksum);
#endif
        return processFrame(&response, data, stageStatus, lastCommand);
    }
}

bool halt() {
    //set up halt command
    bool status = addFrame(CMD_HALT, 0);
    if(!status)  {
        return false;
    }

    //send halt command
    status = sendNextFrame();
    if(!status) {
        deleteNextFrame(); //delete halt
        return false;
    }

    //read response to halt
    if(GOOD != readResponse(NULL, NULL)) {
        return false;
    }

    return true;
}

bool getPosition(unsigned &x, unsigned &y, unsigned char &status) {
    //set up x and y positioning commands
    bool result = addFrame(CMD_GETX, 0);
    if(!result)  {
        return false;
    }

    result = addFrame(CMD_GETY, 0);
    if(!result) {
        deleteNextFrame(); //delete getX
        return false;
    }

    //send setX command
    result = sendNextFrame();
    if(!result) {
        deleteNextFrame(); //delete getX
        deleteNextFrame(); //delete getY
        return false;
    }

    //read response to getX
    if(GOOD != readResponse(&x, &status)) {
        deleteNextFrame(); //delete getY
        return false;
    }

    //send setY command
    result = sendNextFrame();
    if(!result) {
        deleteNextFrame(); //delete getY
        return false;
    }

    //read response to setY
    if(GOOD != readResponse(&y, &status)) {
        return false;
    }

    return true;
}

//moves stage to specified location
bool setPosition(unsigned x, unsigned y) {
    //set up x and y positioning commands
    bool status = addFrame(CMD_SETX, x);
    if(!status)  {
        return false;
    }

    status = addFrame(CMD_SETY, y);
    if(!status) {
        deleteNextFrame(); //delete setX
        return false;
    }

    //send setX command
    status = sendNextFrame();
    if(!status) {
        deleteNextFrame(); //delete setX
        deleteNextFrame(); //delete setY
        return false;
    }

    //read response to setX
    if(GOOD != readResponse(NULL, NULL)) {
        deleteNextFrame(); //delete setY
        halt();
        return false;
    }

    //send setY command
    status = sendNextFrame();
    if(!status) {
        deleteNextFrame(); //delete setY
        halt();
        return false;
    }

    //read response to setY
    if(GOOD != readResponse(NULL, NULL)) {
        halt();
        return false;
    }

    return true;
}

float microstepsToMillimeters(unsigned microsteps) {
    return MOTOR_MILLIMETERS_PER_MICROSTEP * microsteps;
}

unsigned millimetersToMicrosteps(float mm) {
    return (unsigned) (mm / MOTOR_MILLIMETERS_PER_MICROSTEP + 0.5);
}

static void emulate(const struct frame *frame) {
    //process new command, if applicable
    if(frame != nullptr) {
        if(frame->magic != 0x5A)
            return;

        if(frame->checksum != calcChecksum(frame))
            return;

        switch(frame->command) {
        case CMD_SETX:
            time(&emulatedCompleteTime);
            emulatedCompleteTime += 3; //assuming unit is in seconds
            emulatedTargetX = frame->data[3] << 24 | frame->data[2] << 16 | frame->data[1] << 8 | frame->data[0];
            break;
        case CMD_SETY:
            time(&emulatedCompleteTime);
            emulatedCompleteTime += 3; //assuming unit is in seconds
            emulatedTargetY = frame->data[3] << 24 | frame->data[2] << 16 | frame->data[1] << 8 | frame->data[0];
            break;
        case CMD_HALT:
        case CMD_GETWIDTH:
        case CMD_GETHEIGHT:
        case CMD_GETX:
        case CMD_GETY:
        case CMD_CALIB:
        default:
            break;
        }
    }

    //update response packet
    switch(lastCommand) {
    case CMD_HALT:
        createFrame(&response, CMD_HALT, 0);
        time(&emulatedCompleteTime);
        break;
    case CMD_GETWIDTH:
        createFrame(&response, CMD_GETWIDTH, MOTOR_SPAN_IN_MILLIMETERS/MOTOR_MILLIMETERS_PER_MICROSTEP);
        break;
    case CMD_GETHEIGHT:
        createFrame(&response, CMD_GETHEIGHT, MOTOR_SPAN_IN_MILLIMETERS/MOTOR_MILLIMETERS_PER_MICROSTEP);
        break;
    case CMD_GETX:
        createFrame(&response, CMD_GETX, emulatedX);
        break;
    case CMD_GETY:
        createFrame(&response, CMD_GETY, emulatedY);
        break;
    case CMD_SETX:
        createFrame(&response, CMD_SETX, 0);
        break;
    case CMD_SETY:
        createFrame(&response, CMD_SETY, 0);
        break;
    case CMD_CALIB:
        createFrame(&response, CMD_CALIB, 0);
        break;
    default:
        createFrame(&response, (enum CommandID) 0xFF, 0xFFFFFFFF);
        response.status = 0xFF;
    }

    //check if "motors are done moving" and set status accordingly
    if(time(NULL) >= emulatedCompleteTime) {
        emulatedX = emulatedTargetX;
        emulatedY = emulatedTargetY;
        response.status = STAGE_IN_POSITION;
    }
    else {
        response.status = STAGE_MOVING;
    }

    response.checksum = calcChecksum(&response);
}

static void printErrorInfo(int errsv) {
    switch (errsv) {
    case EAGAIN:
        printf("EAGAIN\n");
        break;
    case EBADF:
        printf("EBADF\n");
        break;
    case EDESTADDRREQ:
        printf("EDESTADDRREQ\n");
        break;
    case EDQUOT:
        printf("EDQUOT\n");
        break;
    case EFAULT:
        printf("EFAULT\n");
        break;
    case EFBIG:
        printf("EFBIG\n");
        break;
    case EINTR:
        printf("EINTR\n");
        break;
    case EINVAL:
        printf("EINVAL\n");
        break;
    case EIO:
        printf("EIO\n");
        break;
    case ENOSPC:
        printf("ENOSPC\n");
        break;
    case EPIPE:
        printf("EPIPE\n");
        break;
    case EREMOTEIO:
        printf("EREMOTEIO\n");
        break;
    default:
        printf("other: %08X\n", errsv);
        fflush(stdout);
    }
}

}
