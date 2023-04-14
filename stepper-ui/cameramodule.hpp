#ifndef CAMERAMODULE_HPP
#define CAMERAMODULE_HPP

#include "amcam.h"

#include "DynamicImage.h"

namespace camera_module {
    extern DynamicImage *liveImage;
    extern DynamicImage *stillImage;

    extern bool liveImageReady();
    extern bool stillImageReady();
    extern bool isOpen();
    extern bool openCamera();
    extern void closeCamera();
    extern bool captureImage();
}

#endif // CAMERAMODULE_HPP
