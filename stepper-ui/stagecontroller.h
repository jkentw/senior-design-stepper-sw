#ifndef STAGECONTROLLER_H
#define STAGECONTROLLER_H

#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <cstdio>
#include <fcntl.h>

#ifdef DEBUG_MODE_GLOBAL
    #define DEBUG_MODE_I2C
#endif

namespace stagecontroller {

//configuration constants
static const int BUF_SIZE = 8;
static const int I2C_ADDRESS = 0x0F;
static const int MAGIC = 0x5A;
static const int CHECK_VALUE = 0xFF;

//status codes
const __u32 GOOD = 0;
const __u32 ERROR_READ = 1 << 31;
const __u32 ERROR_MAGIC = 1 << 30;
const __u32 ERROR_UNKNOWN_COMMAND = 1 << 29;
const __u32 ERROR_UNEXPECTED_COMMAND = 1 << 28;
const __u32 ERROR_CHECKSUM = 1 << 27;

//state information
static int i2cFd = 0;
static int writeIndex = 0; //index of next available frame slot
static int readIndex = 0; //index of next frame to send
static int lastCommand = -1;

enum CommandID {
    CMD_HALT = 0,
    CMD_GETWIDTH = 1,
    CMD_GETHEIGHT = 2,
    CMD_GETX = 3,
    CMD_GETY = 4,
    CMD_SETX = 5,
    CMD_SETY = 6,
    CMD_CALIB = 7
};

struct frame {
    __u8 magic;
    __u8 command;
    __u8 data[4]; //representation as array gives control over endianness
    __u8 status;
    __u8 checksum;
} buffer[BUF_SIZE], response;

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

bool openI2c() {
    int adapter_nr = 1; //should dynamically determine this
    char fname[20];

    snprintf(fname, 19, "/dev/i2c-%d", adapter_nr);
    i2cFd = open(fname, O_RDWR);

    if(i2cFd == -1) {
#ifdef DEBUG_MODE_I2C
        printf("device file '%-19s' not found\n", fname);
        fflush(stdout);
#endif
        i2cFd = 0;
        return false;
    }

    if(ioctl(i2cFd, I2C_SLAVE, I2C_ADDRESS) == -1) {
#ifdef DEBUG_MODE_I2C
        printf("error configuring device address\n");
        fflush(stdout);
#endif
        close(i2cFd);
        i2cFd = 0;
        return false;
    }

#ifdef DEBUG_MODE_I2C
    printf("I2C communication established.\n");
    fflush(stdout);
#endif
    return true;
}

void closeI2c() {
    if(i2cFd != 0) {
        if(close(i2cFd) == -1) {
#ifdef DEBUG_MODE_I2C
            printf("Could not terminate I2C communication.\n");
            fflush(stdout);
#endif
        }

        i2cFd = 0;
    }

#ifdef DEBUG_MODE_I2C
    printf("I2C communication terminated.\n");
    fflush(stdout);
#endif
}

//calculates what checksum field should be based frame data
static __u8 calcChecksum(const struct frame *frame) {
    __u8 sum = 0;
    __u8 *bytes = (__u8 *) frame;

    for(int i = 0; i < 7; i++) {
        sum += bytes[i];
    }

    return CHECK_VALUE - sum;
}

static int getBufferLength() {
    return (writeIndex - readIndex + BUF_SIZE) % BUF_SIZE;
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
        writeIndex = (writeIndex + 1) % BUF_SIZE; //buffer is circular
        return true;
    }
    else
        return false;
}

//Adds an existing frame to the circular queue. Frame assumed to be valid
bool addFrame(const struct frame *dest) {
    if(getBufferLength() < BUF_SIZE) {
        __u8 *thisFrameBytes = (__u8 *) &buffer[writeIndex];

        //deep copy
        for(int i = 0; i < 8; i++) {
            thisFrameBytes[i] = ((__u8 *) dest)[i];
        }

        writeIndex = (writeIndex + 1) % BUF_SIZE; //buffer is circular
        return true;
    }
    else
        return false;
}

//sends arbitrary frame without advancing buffer
bool sendFrame(const struct frame *frame) {
    __s32 result = write(i2cFd, (__u8 *) frame, 8);

    int errsv = errno;
    if(result == -1) {
#ifdef DEBUG_MODE_I2C
        printf("I2C write failed:\t\n");
        printErrorInfo(errsv);
#endif
    }
    else {
#ifdef DEBUG_MODE_I2C
        printf("I2C write successful.\n");
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
        readIndex = (readIndex + 1) % BUF_SIZE; //buffer is circular
    }

    return false;
}

//Returns status code. Set expectedCommand to -1 if check is unnecessary.
int processFrame(const struct frame *frame, __u32 *data, int expectedCommand) {
    int status = GOOD;

    if(frame->magic != MAGIC)
        status |= ERROR_MAGIC;

    if(expectedCommand != -1 && frame->command != expectedCommand)
        status |= ERROR_UNEXPECTED_COMMAND;

    if(frame->command < 0 || frame->command >= 8)
        status |= ERROR_UNKNOWN_COMMAND;

    if(frame->checksum != calcChecksum(frame))
        status |= ERROR_CHECKSUM;

#ifdef DEBUG_MODE_I2C
    if(status != GOOD) {
        printf("Error in received packet: code %08X\n", status);
    }
#endif

    *data = frame->data[3] << 24 | frame->data[2] << 16 | frame->data[1] << 8 | frame->data[0];
    return status;
}

//returns status code from CommunicationStatus
int readResponse(__u32 *data) {
    __s32 result = read(i2cFd, (__u8 *) &response, 8);

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
        return processFrame(&response, data, lastCommand);
    }
}

}

#endif // STAGECONTROLLER_H
