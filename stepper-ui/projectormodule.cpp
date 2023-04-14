#include "config.hpp"

#include <linux/fb.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <cstdio>

#include "projectormodule.hpp"
#include "DynamicImage.h"

#ifdef DEBUG_MODE_GLOBAL
#define DEBUG_MODE_PROJECTOR
#endif

namespace projector_module {

static int projectorFd = -1; //projector file descriptor

DynamicImage *projectedImage;
static QImage *blankImage = nullptr;
static QImage *patternImage = nullptr;
static int width = 0;
static int height = 0;

bool isOpen();
bool openProjector();
void closeProjector();
void setPattern(QImage image);
void show();
void hide();
static void printErrno(int num);

bool isOpen() {
    return projectorFd >= 0;
}

bool openProjector() {
    __s32 res;
    struct fb_fix_screeninfo fixedScrInfo;
    struct fb_var_screeninfo varScrInfo;

    if(isOpen())
        return true;

    //get file descriptor of hdmi framebuffer
    projectorFd = open("/dev/fb0", O_RDWR); //make sure this is the correct framebuffer

    if(projectorFd < 0) {
#ifdef DEBUG_MODE_PROJECTOR
        printf("[ProjectorModule] Device file not found\n");
        fflush(stdout);
#endif
        return false;
    }

#ifdef DEBUG_MODE_PROJECTOR
    printf("[ProjectorModule] Device file found\n");
    fflush(stdout);
#endif

    //get variable screen info
    res = ioctl(projectorFd, FBIOGET_VSCREENINFO, &varScrInfo);
    int errsv = errno;

    if(res == -1) {
#ifdef DEBUG_MODE_PROJECTOR
        printf("[ProjectorModule] Could not read variable screen info:\n\t");
        printErrno(errsv);
        fflush(stdout);
#endif
        closeProjector();
        return false;
    }

    //get fixed screen info
    res = ioctl(projectorFd, FBIOGET_FSCREENINFO, &fixedScrInfo);
    errsv = errno;

    if(res < 0) {
#ifdef DEBUG_MODE_PROJECTOR
        printf("[ProjectorModule] Could not read fixed screen info:\n\t");
        printErrno(errsv);
        fflush(stdout);
#endif
        closeProjector();
        return false;
    }

#ifdef DEBUG_MODE_PROJECTOR
    //print info
    printf("[ProjectorModule] Variable display info: %dx%d, %d bpp\n", varScrInfo.xres, varScrInfo.yres, varScrInfo.bits_per_pixel);
    printf("  Virtual size:        %dx%d\n", varScrInfo.xres_virtual, varScrInfo.yres_virtual);
    printf("  Visible offset:      %dx%d\n", varScrInfo.xoffset, varScrInfo.yoffset);
    fflush(stdout);
#endif

    //update width and height here
    //width = varScrInfo.xres;
    //height = varScrInfo.yres;

    height = 1080;
    width = 1920;

    blankImage = new QImage(projector_module::width, projector_module::height, QImage::Format::Format_RGB888);
    blankImage->fill(Qt::black);
    patternImage = blankImage;
    projectedImage = new DynamicImage();
    projectedImage->setImage(blankImage);

    return true;
}

void closeProjector() {
    if(isOpen()) {
        close(projectorFd);
        projectorFd = -1;
    }

    if(blankImage != nullptr) {
        delete blankImage;
        blankImage = nullptr;
        patternImage = nullptr;
    }
}

void setPattern(QImage *pattern) {
    patternImage = pattern;
}

void show() {
#ifdef DEBUG_MODE_PROJECTOR
    printf("[ProjectorModule] Showing pattern image\n");
    fflush(stdout);
#endif
    projectedImage->setImage(patternImage);
}

void hide() {
#ifdef DEBUG_MODE_PROJECTOR
    printf("[ProjectorModule] Showing blank image\n");
    fflush(stdout);
#endif
    projectedImage->setImage(blankImage);
}

static void printErrno(int num) {
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
    fflush(stdout);
}

}
