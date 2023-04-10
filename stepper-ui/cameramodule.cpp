#include <iostream>
#include "cameramodule.hpp"

HAmcam CameraModule::cameraHandle;
int CameraModule::width;
int CameraModule::height;
void* CameraModule::imageData;
QPixmap *CameraModule::pixmap;
CameraModule *CameraModule::globalInstance;

CameraModule::CameraModule() : QWidget(nullptr), QQuickImageProvider(QQuickImageProvider::Pixmap) {
    globalInstance = this;
}

bool CameraModule::initialize()
{
    cameraHandle = Amcam_Open(NULL);
    imageData = NULL;
    pixmap = NULL;

    if(cameraHandle == NULL) {
        std::cout << "no camera found or open failed" << std::endl;
        return false;
    }

    HRESULT hr = Amcam_get_Size(cameraHandle, &width, &height);

    if (FAILED(hr)) {
        std::cout << "failed to get size, hr = " << hr << std::endl;
        return false;
    }
    else {
        imageData = malloc(TDIBWIDTHBYTES(24 * width) * height);
        if (NULL == imageData) {
            std::cout <<  "failed to malloc\n" << std::endl;
            return false;
        }
    }

    pixmap = new QPixmap;
    std::cout << "camera opened\n" << std::endl;
    return true;
}

void CameraModule::terminate()
{
    Amcam_Close(cameraHandle);
    cameraHandle = NULL;

    if(imageData) {
        free(imageData);
        imageData = NULL;
    }

    if(pixmap) {
        delete pixmap;
    }
}

bool CameraModule::captureImage()
{
    HRESULT hr = Amcam_StartPullModeWithCallback(cameraHandle, &CameraModule::callback, NULL);
    if (FAILED(hr)) {
        std::cout << "failed to start camera, hr = " << hr << std::endl;
        return false;
    }
    else {
        printf("waiting...\n");
        return true;
    }
}

void CameraModule::callback(unsigned nEvent, void *pCallbackCtx)
{
    if (AMCAM_EVENT_IMAGE == nEvent) {
        AmcamFrameInfoV2 info = { 0 };
        HRESULT hr = Amcam_PullImageV2(cameraHandle, imageData, 24, &info);
        if (FAILED(hr)) {
            std::cout << "failed to pull image, hr = " << hr << std::endl;
        }
        else {
            std::cout << "image captured" << std::endl;
            QImage img((const uchar *) imageData, width, height, QImage::Format_RGB888);
            *pixmap = QPixmap::fromImage(img);

            emit globalInstance->imageChanged();
        }
    }
    else {
        std::cout << "other callback: " << nEvent << std::endl;
    }
}

QPixmap CameraModule::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) {
    if(id == "frame") {
        size->setWidth(width);
        size->setHeight(height);
        return *pixmap;
    }
    else {
        return QPixmap();
    }
}
