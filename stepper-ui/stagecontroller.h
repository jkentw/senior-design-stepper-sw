#ifndef STAGECONTROLLER_H
#define STAGECONTROLLER_H

#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>

#include <cstdio>

void something() {
    printf("function 'something' called\n");
    fflush(stdout);
    FILE *i2cDevFile;
    int adapter_nr = 1; //should dynamically determine this
    char fname[20];

    snprintf(fname, 19, "/dev/i2c-%d", adapter_nr);
    printf("device name: '%-19s'\n", fname);
    fflush(stdout);
    i2cDevFile = fopen(fname, "r+");

    if(!i2cDevFile) {
        printf("device file not found\n");
        fflush(stdout);
        fclose(i2cDevFile); //???
        return;
    }

    printf("device file found\n");
    fflush(stdout);

    int address = 0x0F;
    int fd = fileno(i2cDevFile);

    if(ioctl(fd, I2C_SLAVE, address) < 0) {
        printf("error configuring device address\n");
        fflush(stdout);
        fclose(i2cDevFile); //???
        return;
    }

    __u8 reg = 0x10;
    __s32 res;
    __u8 buf[10];

    //set up data frame for write
    buf[0] = 0x5A;
    buf[1] = 0x03; //command (getX)
    buf[2] = 0x00; //data
    buf[3] = 0x00; //data
    buf[4] = 0x00; //data
    buf[5] = 0x00; //data
    buf[6] = 0x00; //status
    buf[7] = 0x00; //CRC

    res = write(fd, buf, 8);
    int errsv = errno;

    if(res < 0) {
        printf("write failed: %08X\n\t", res);

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
    else {
        printf("write succeeded\n");
    }

    buf[0] = 0x5A;
    buf[1] = 0x03; //command (getX)
    buf[2] = 0x00; //data
    buf[3] = 0x00; //data
    buf[4] = 0x00; //data
    buf[5] = 0x00; //data
    buf[6] = 0x00; //status
    buf[7] = 0x00; //CRC

    res = read(fd, buf, 8);

    if(res < 0) {
        printf("read failed\n");
    }
    else {
        printf("read successful: %02X %02X %02X %02X %02X %02X %02X %02X \n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
    }

    fflush(stdout);
    fclose(i2cDevFile);
}

#endif // STAGECONTROLLER_H
