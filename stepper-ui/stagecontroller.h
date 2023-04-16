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

namespace stage_controller {

//status codes
const __u32 GOOD = 0;
const __u32 ERROR_READ = 1 << 31;
const __u32 ERROR_MAGIC = 1 << 30;
const __u32 ERROR_UNKNOWN_COMMAND = 1 << 29;
const __u32 ERROR_UNEXPECTED_COMMAND = 1 << 28;
const __u32 ERROR_CHECKSUM = 1 << 27;
const __u32 ERROR_UNKNOWN_STATUS = 1 << 26;

enum StageStatus {
    STAGE_NOT_READY,
    STAGE_MOVING,
    STAGE_IN_POSITION
};

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
};

//I2C setup
extern bool isOpen();
extern bool openI2c();
extern void closeI2c();

//data frame management
extern int getBufferLength();
extern void clearBuffer();
extern void createFrame(struct frame *dest, CommandID command, __u32 data);
extern bool addFrame(CommandID command, __u32 data);
extern bool addFrame(const struct frame *dest);
extern bool sendFrame(const struct frame *frame);
extern bool sendNextFrame();
extern int readResponse(__u32 *data, __u8 *stageStatus);
extern int processFrame(const struct frame *frame, __u32 *data, __u8 *stageStatus, int expectedCommand);

//motor control convenience functions
extern bool halt();
extern bool getPosition(unsigned &x, unsigned &y, unsigned char &status);
extern bool setPosition(unsigned x, unsigned y);

//unit conversion convenience functions
extern float microstepsToMillimeters(unsigned microsteps);
extern unsigned millimetersToMicrosteps(float mm);

}

#endif // STAGECONTROLLER_H
