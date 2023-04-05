#ifndef PROJECTORMODULE_HPP
#define PROJECTORMODULE_HPP

#ifndef STAGECONTROLLER_H
#define STAGECONTROLLER_H

#include <linux/fb.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <string.h>

#include <cstdio>

void printErrno(int num) {
    switch (num) {
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
        printf("other: %08X\n", num);
    }
}

void testProjector() {
    __s32 res;
    struct fb_fix_screeninfo fixedScrInfo;
    struct fb_var_screeninfo varScrInfo;
    unsigned long screenSize = 0;
    char *fbMap = 0;

    printf("'testProjector()' called\n");
    printf("attempting to open framebuffer file\n");
    fflush(stdout);

    //get file descriptor of hdmi framebuffer
    int projectorFd = open("/dev/fb0", O_RDWR); //make sure this is the correct framebuffer

    if(projectorFd < 0) {
        printf("device file not found\n");
        fflush(stdout);
        return;
    }

    printf("device file found\n");
    fflush(stdout);

    //get variable screen info
    res = ioctl(projectorFd, FBIOGET_VSCREENINFO, &varScrInfo);
    int errsv = errno;

    if(res < 0) {
        printf("could not read variable screen info:\n\t");
        printErrno(errsv);
        fflush(stdout);
        close(projectorFd);
        return;
    }

    //get fixed screen info
    res = ioctl(projectorFd, FBIOGET_FSCREENINFO, &fixedScrInfo);
    errsv = errno;

    if(res < 0) {
        printf("could not read fixed screen info:\n\t");
        printErrno(errsv);
        fflush(stdout);
        close(projectorFd);
        return;
    }

    //print info
    printf("Variable display info: %dx%d, %d bpp\n", varScrInfo.xres, varScrInfo.yres, varScrInfo.bits_per_pixel);
    printf("  Virtual size:        %dx%d\n", varScrInfo.xres_virtual, varScrInfo.yres_virtual);
    printf("  Visible offset:      %dx%d\n", varScrInfo.xoffset, varScrInfo.yoffset);

    screenSize = fixedScrInfo.smem_len;
    fbMap = (char *) mmap(0, screenSize, PROT_READ | PROT_WRITE, MAP_SHARED, projectorFd, 0);

    if((int) fbMap == -1) {
        printf("mmap failed\n");
        fflush(stdout);
        close(projectorFd);
        return;
    }

    for(int i = 0; i < 500; i++) {
        memset(fbMap, 0x7F, screenSize/3);
        memset(fbMap+screenSize/3, 0xBF, screenSize/3);
        memset(fbMap+2*screenSize/3, 0xFF, screenSize/3);
    }

    printf("end of function reached\n");
    fflush(stdout);
    munmap(fbMap, screenSize);
    close(projectorFd);
}

#endif // STAGECONTROLLER_H

#endif // PROJECTORMODULE_HPP
