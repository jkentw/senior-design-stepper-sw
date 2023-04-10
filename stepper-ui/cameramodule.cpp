#include "config.hpp"

#include "cameramodule.hpp"

#ifdef DEBUG_MODE_GLOBAL
#define DEBUG_MODE_CAMERA
#endif

#ifdef DEBUG_MODE_CAMERA
#include <cstdio>
#endif

namespace camera_module {

DynamicImage *cameraImage;
static HAmcam cameraHandle = NULL;
static int width = 0;
static int height = 0;
static void* imageData = NULL;
static QImage *imagePtr = NULL;

static void __stdcall callback(unsigned nEvent, void* pCallbackCtx);

bool openCamera()
{
    if(cameraHandle) {
#ifdef DEBUG_MODE_CAMERA
        printf("[CameraModule] Camera is already open\n");
        fflush(stdout);
#endif
        return true;
    }

    cameraHandle = Amcam_Open(NULL);
    imageData = NULL;
    imagePtr = NULL;

    if(cameraHandle == NULL) {
#ifdef DEBUG_MODE_CAMERA
        printf("[CameraModule] No camera found or open failed\n");
        fflush(stdout);
#endif
        return false;
    }

    HRESULT hr = Amcam_get_Size(cameraHandle, &width, &height);

    if (FAILED(hr)) {
#ifdef DEBUG_MODE_CAMERA
        printf("[CameraModule] Failed to get size; hr = %d\n");
        fflush(stdout);
#endif
        return false;
    }
    else {
        imageData = malloc(TDIBWIDTHBYTES(24 * width) * height);
        if (NULL == imageData) {
#ifdef DEBUG_MODE_CAMERA
            printf("[CameraModule] Failed to allocate memory for image data\n");
            fflush(stdout);
#endif
            return false;
        }
    }

    imagePtr = new QImage(width, height, QImage::Format_RGB888);
    imagePtr->fill(Qt::black);

    cameraImage = new DynamicImage();
    cameraImage->setImage(imagePtr);

#ifdef DEBUG_MODE_CAMERA
    printf("[CameraModule] Camera opened\n");
    fflush(stdout);
#endif
    return true;
}

void closeCamera()
{
    if(cameraHandle) {
        Amcam_Close(cameraHandle);
        cameraHandle = NULL;
    }

    if(imageData) {
        free(imageData);
        imageData = NULL;
    }

    if(imagePtr) {
        delete imagePtr;
        imagePtr = NULL;
    }

#ifdef DEBUG_MODE_CAMERA
    printf("[CameraModule] Camera closed\n");
    fflush(stdout);
#endif
}

bool captureImage()
{
    HRESULT hr = Amcam_StartPullModeWithCallback(cameraHandle, &callback, NULL);
    if (FAILED(hr)) {
#ifdef DEBUG_MODE_CAMERA
        printf("[CameraModule] Failed to start camera, hr = %d\n", hr);
        fflush(stdout);
#endif
        return false;
    }
    else {
#ifdef DEBUG_MODE_CAMERA
        printf("[CameraModule] Waiting... ");
        fflush(stdout);
#endif
        return true;
    }
}

void callback(unsigned nEvent, void *pCallbackCtx)
{
    if (AMCAM_EVENT_IMAGE == nEvent) {
        AmcamFrameInfoV2 info = {};
        HRESULT hr = Amcam_PullImageV2(cameraHandle, imageData, 24, &info);
        if (FAILED(hr)) {
#ifdef DEBUG_MODE_CAMERA
            printf("[CameraModule] failed to pull image, hr = %d\n", hr);
            fflush(stdout);
#endif
        }
        else {
#ifdef DEBUG_MODE_CAMERA
            printf(" Image captured.\n");
            fflush(stdout);
#endif
            QImage img((const uchar *) imageData, width, height, QImage::Format_RGB888);
            *imagePtr = img;
            cameraImage->setImage(imagePtr);
        }
    }
    else {
#ifdef DEBUG_MODE_CAMERA
        printf("[CameraModule] Other callback: %d\n", nEvent);
        fflush(stdout);
#endif
    }
}

}
