#include "config.hpp"

#include "cameramodule.hpp"

#ifdef DEBUG_MODE_GLOBAL
#define DEBUG_MODE_CAMERA
#endif

#ifdef DEBUG_MODE_CAMERA
#include <cstdio>
#endif

namespace camera_module {

static const int liveIndex = 1;
static const int stillIndex = 0;

static HAmcam cameraHandle = NULL;

static int liveWidth = 0;
static int liveHeight = 0;
DynamicImage *liveImage = NULL;
static void *liveData = NULL;
static QImage *livePtr = NULL;

static int stillWidth = 0;
static int stillHeight = 0;
DynamicImage *stillImage = NULL;
static void *stillData = NULL;
static QImage *stillPtr = NULL;

static void __stdcall callback(unsigned nEvent, void* pCallbackCtx);

bool isOpen() {
    return cameraHandle != NULL;
}

bool openCamera() {
    HRESULT hr;

    if(isOpen()) {
#ifdef DEBUG_MODE_CAMERA
        printf("[CameraModule] Camera is already open\n");
        fflush(stdout);
#endif
        return true;
    }

    cameraHandle = Amcam_Open(NULL);
    liveData = NULL;
    livePtr = NULL;
    stillData = NULL;
    stillPtr = NULL;

    if(cameraHandle == NULL) {
#ifdef DEBUG_MODE_CAMERA
        printf("[CameraModule] No camera found or open failed\n");
        fflush(stdout);
#endif
        return false;
    }

    hr = Amcam_put_eSize(cameraHandle, stillIndex);
    if(SUCCEEDED(hr))
        hr = Amcam_get_Size(cameraHandle, &stillWidth, &stillHeight);

    if (FAILED(hr)) {
#ifdef DEBUG_MODE_CAMERA
        printf("[CameraModule] Failed to set or get still size; hr = %d\n", hr);
        fflush(stdout);
#endif
        return false;
    }
    else {
        stillData = malloc(TDIBWIDTHBYTES(24 * stillWidth) * stillHeight);
        if (NULL == stillData) {
#ifdef DEBUG_MODE_CAMERA
            printf("[CameraModule] Failed to allocate memory for still image data\n");
            fflush(stdout);
#endif
            return false;
        }
    }

    hr = Amcam_put_eSize(cameraHandle, liveIndex);
    if(SUCCEEDED(hr))
        hr = Amcam_get_Size(cameraHandle, &liveWidth, &liveHeight);

    if (FAILED(hr)) {
#ifdef DEBUG_MODE_CAMERA
        printf("[CameraModule] Failed to set or get live size; hr = %d\n", hr);
        fflush(stdout);
#endif
        return false;
    }
    else {
        liveData = malloc(TDIBWIDTHBYTES(24 * liveWidth) * liveHeight);
        if (NULL == liveData) {
#ifdef DEBUG_MODE_CAMERA
            printf("[CameraModule] Failed to allocate memory for live image data\n");
            fflush(stdout);
#endif
            return false;
        }
    }

    livePtr = new QImage(liveWidth, liveHeight, QImage::Format_RGB888);
    livePtr->fill(Qt::black);

    liveImage = new DynamicImage();
    liveImage->setImage(livePtr);

    stillPtr = new QImage(stillWidth, stillHeight, QImage::Format_RGB888);
    stillPtr->fill(Qt::black);

    stillImage = new DynamicImage();
    stillImage->setImage(stillPtr);

#ifdef DEBUG_MODE_CAMERA
    printf("[CameraModule] Camera opened: live resolution %dx%d, still resolution %dx%d\n",
           liveWidth, liveHeight, stillWidth, stillHeight);
    fflush(stdout);
#endif
    return true;
}

void closeCamera() {
    if(isOpen()) {
        Amcam_Close(cameraHandle);
        cameraHandle = NULL;
    }

    if(liveImage) {
        delete liveImage;
        liveImage = NULL;
    }

    if(liveData) {
        free(liveData);
        liveData = NULL;
    }

    if(livePtr) {
        delete livePtr;
        livePtr = NULL;
    }

    if(stillImage) {
        delete stillImage;
        stillImage = NULL;
    }

    if(stillData) {
        free(stillData);
        stillData = NULL;
    }

    if(stillPtr) {
        delete stillPtr;
        stillPtr = NULL;
    }

#ifdef DEBUG_MODE_CAMERA
    printf("[CameraModule] Camera closed\n");
    fflush(stdout);
#endif
}

bool captureImage() {
    HRESULT hr = Amcam_StartPullModeWithCallback(cameraHandle, &callback, NULL);
    HRESULT hr2 = Amcam_Snap(cameraHandle, 0);

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

void callback(unsigned nEvent, void *pCallbackCtx) {
    HRESULT hr;
    AmcamFrameInfoV2 info = {};

    if (AMCAM_EVENT_IMAGE == nEvent) {
        hr = Amcam_PullImageV2(cameraHandle, liveData, 24, &info);

        if (FAILED(hr)) {
#ifdef DEBUG_MODE_CAMERA
            printf("[CameraModule] Failed to pull image, hr = %d\n", hr);
            fflush(stdout);
#endif
        }
        else {
#ifdef DEBUG_MODE_CAMERA
            printf("[CameraModule] Live image captured.\n");
            fflush(stdout);
#endif
            QImage img((const uchar *) liveData, liveWidth, liveHeight, QImage::Format_RGB888);
            *livePtr = img;
            liveImage->setImage(livePtr);
        }
    }
    else if(AMCAM_EVENT_STILLIMAGE == nEvent) {
        hr = Amcam_PullStillImageV2(cameraHandle, stillData, 24, &info);

        if (FAILED(hr)) {
#ifdef DEBUG_MODE_CAMERA
            printf("[CameraModule] Failed to pull image, hr = %d\n", hr);
            fflush(stdout);
#endif
        }
        else {
#ifdef DEBUG_MODE_CAMERA
            printf("[CameraModule] Still image captured.\n");
            fflush(stdout);
#endif
            QImage img((const uchar *) stillData, stillWidth, stillHeight, QImage::Format_RGB888);
            *stillPtr = img;
            stillImage->setImage(stillPtr);
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
