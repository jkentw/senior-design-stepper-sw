#ifndef CAMERAMODULE_HPP
#define CAMERAMODULE_HPP

#include <QPixmap>
#include <QQuickImageProvider>
#include <QWidget>
#include "amcam.h"

#include "DynamicImage.h"

namespace camera_module {
    extern DynamicImage *cameraImage;

    bool openCamera();
    void closeCamera();
    bool captureImage();
}

#endif // CAMERAMODULE_HPP
