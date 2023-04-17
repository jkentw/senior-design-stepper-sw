#ifndef PROCESSCONTROL_HPP
#define PROCESSCONTROL_HPP

#include "config.hpp"

#include "ProcessControl.hpp"

#include "Recipe.hpp"
#include "cameramodule.hpp"
#include "projectormodule.hpp"
#include "stagecontroller.h"
#include "imageprocessor.hpp"

#include <QImage>
#include <cmath>

#include <cstdio>

namespace process_control {

//main variables
Recipe recipe;
int dieNumber = 0;

ImageProcessor imageProcessor;
ImageProcessor::Point *patternPoints = nullptr;
unsigned numPoints = 0;
ImageProcessor::Point disp = {};

float *kernel = nullptr;
unsigned kernelWidth = 0;
unsigned kernelHeight = 0;

//last read motor position in wafer coordinates (millimeters)
float currentX = 0;
float currentY = 0;

bool shouldStart = false;
bool shouldAbort = false;

//each state should be designed for individual testing
enum ControlState {
    STATE_INVALID,
    STATE_ERROR,
    STATE_RESET,
    STATE_AWAIT_UPLOAD,
    STATE_READY,
    STATE_COARSE_ALIGN,
    STATE_FINE_ALIGN_IMAGE,
    STATE_FINE_ALIGN_MOTOR,
    STATE_EXPOSE,
}
currentState = STATE_INVALID, //force state transition to STATE_RESET
nextState = STATE_RESET;

enum ControlResult {
    RESULT_GOOD,
    RESULT_ABORTED,
    RESULT_ERROR,
    RESULT_PROJECTOR_ERROR,
    RESULT_CAMERA_ERROR,
    RESULT_I2C_COMM_ERROR,
    RESULT_I2C_FRAME_ERROR,
    RESULT_I2C_BUF_ERROR,
    RESULT_MOTOR_ERROR,
    RESULT_RECIPE_ERROR,
    RESULT_IMAGE_ERROR,
};

enum ControlState getState(); //returns currentState
enum ControlResult setState(enum ControlState newState); //forces next state to newState
enum ControlResult update();  //executes current state or transitions to another state
enum ControlResult start();
enum ControlResult abort();

//state entry transition functions
enum ControlResult enterReset();
enum ControlResult enterAwaitUpload();
enum ControlResult enterReady();
enum ControlResult enterCoarseAlign();
enum ControlResult enterFineAlignImage();
enum ControlResult enterFineAlignMotor();
enum ControlResult enterExpose();

//state functions
enum ControlResult executeReset();
enum ControlResult executeAwaitUpload();
enum ControlResult executeReady();
enum ControlResult executeCoarseAlign();
enum ControlResult executeFineAlignImage();
enum ControlResult executeFineAlignMotor();
enum ControlResult executeExpose();

//state exit transition functions
enum ControlResult exitReset();
enum ControlResult exitAwaitUpload();
enum ControlResult exitReady();
enum ControlResult exitCoarseAlign();
enum ControlResult exitFineAlignImage();
enum ControlResult exitFineAlignMotor();
enum ControlResult exitExpose();

enum ControlResult update() {
    enum ControlResult result = RESULT_GOOD;

#ifdef DEBUG_MODE_PROCESS_CONTROL
    printf("[ProcessControl] CurrentState: %d, NextState: %d\n", currentState, nextState);
    fflush(stdout);
#endif

    if(shouldAbort) {
        nextState = STATE_RESET; //exit from current state and reset
    }

    if(currentState == nextState) {
        //state execution
        switch(currentState) {
        case STATE_RESET:
            result = executeReset();
            break;
        case STATE_AWAIT_UPLOAD:
            result = executeAwaitUpload();
            break;
        case STATE_READY:
            result = executeReady();
            break;
        case STATE_COARSE_ALIGN:
            result = executeCoarseAlign();
            break;
        case STATE_FINE_ALIGN_IMAGE:
            result = executeFineAlignImage();
            break;
        case STATE_FINE_ALIGN_MOTOR:
            result = executeFineAlignMotor();
            break;
        case STATE_EXPOSE:
            result = executeExpose();
            break;
        default:
            currentState = STATE_INVALID;
            nextState = STATE_RESET;
        }
    }
    else {
        //state exit
        switch(currentState) {
        case STATE_RESET:
            result = exitReset();
            break;
        case STATE_AWAIT_UPLOAD:
            result = exitAwaitUpload();
            break;
        case STATE_READY:
            result = exitReady();
            break;
        case STATE_COARSE_ALIGN:
            result = exitCoarseAlign();
            break;
        case STATE_FINE_ALIGN_IMAGE:
            result = exitFineAlignImage();
            break;
        case STATE_FINE_ALIGN_MOTOR:
            result = exitFineAlignMotor();
            break;
        case STATE_EXPOSE:
            result = exitExpose();
            break;
        default:
            currentState = STATE_INVALID;
        }

        //check result here, error handling, etc.

        //state entry
        switch(nextState) {
        case STATE_RESET:
            result = enterReset();
            break;
        case STATE_AWAIT_UPLOAD:
            result = enterAwaitUpload();
            break;
        case STATE_READY:
            result = enterReady();
            break;
        case STATE_COARSE_ALIGN:
            result = enterCoarseAlign();
            break;
        case STATE_FINE_ALIGN_IMAGE:
            result = enterFineAlignImage();
            break;
        case STATE_FINE_ALIGN_MOTOR:
            result = enterFineAlignMotor();
            break;
        case STATE_EXPOSE:
            result = enterExpose();
            break;
        default:
            nextState = STATE_INVALID;
        }

        //check result here, error handling, etc.
        currentState = nextState;
    }


    if(shouldAbort) {
        result = RESULT_ABORTED;
        shouldAbort = false;
    }

    return result;
}

enum ControlResult setState(enum ControlState newState) {
    nextState = newState;
    return update();
}

enum ControlState getState() {
    return currentState;
}

enum ControlResult start() {
    shouldStart = true;
    return update();
}

enum ControlResult abort() {
    shouldAbort = true;
    return update();
}

enum ControlResult enterReset() {
#ifdef DEBUG_MODE_PROCESS_CONTROL
    printf("[ProcessControl] Entering STATE_RESET\n");
    fflush(stdout);
#endif
    //close all open devices

    if(projector_module::isOpen()) {
        projector_module::closeProjector();
    }

    if(camera_module::isOpen()) {
        camera_module::closeCamera();
    }

    if(stage_controller::isOpen()) {
        stage_controller::closeI2c();
        stage_controller::clearBuffer();
    }

    if(patternPoints != nullptr){
        delete[] patternPoints;
        patternPoints = nullptr;
    }

    if(kernel != nullptr) {
        delete[] kernel;
        kernel = nullptr;
    }

    return RESULT_GOOD;
}

enum ControlResult executeReset() {
#ifdef DEBUG_MODE_PROCESS_CONTROL
    printf("[ProcessControl] Executing STATE_RESET\n");
    fflush(stdout);
#endif

    enum ControlResult result = RESULT_GOOD;
    nextState = STATE_AWAIT_UPLOAD;

    if(!projector_module::openProjector()) {
        nextState = STATE_ERROR;
        result = RESULT_PROJECTOR_ERROR;
    }

    if(!camera_module::openCamera()) {
        nextState = STATE_ERROR;
        result = RESULT_CAMERA_ERROR;
    }

    if(!stage_controller::openI2c()) {
        nextState = STATE_ERROR;
        result = RESULT_I2C_COMM_ERROR;
    }

    return result;
}

enum ControlResult exitReset() {
#ifdef DEBUG_MODE_PROCESS_CONTROL
    printf("[ProcessControl] Exiting STATE_RESET\n");
    fflush(stdout);
#endif
    return RESULT_GOOD;
}

enum ControlResult enterAwaitUpload() {
#ifdef DEBUG_MODE_PROCESS_CONTROL
    printf("[ProcessControl] Entering STATE_AWAIT_UPLOAD\n");
    fflush(stdout);
#endif

    recipe.erase();
    return RESULT_GOOD;
}

enum ControlResult executeAwaitUpload() {
#ifdef DEBUG_MODE_PROCESS_CONTROL
    printf("[ProcessControl] Executing STATE_AWAIT_UPLOAD\n");
    fflush(stdout);
#endif

    if(recipe.isValid()) {
        nextState = STATE_READY;
    }
    else if(recipe.isInvalid()) {
        return RESULT_RECIPE_ERROR;
    }

    return RESULT_GOOD;
}

enum ControlResult exitAwaitUpload() {
#ifdef DEBUG_MODE_PROCESS_CONTROL
    printf("[ProcessControl] Exiting STATE_AWAIT_UPLOAD\n");
    fflush(stdout);
#endif

    dieNumber = 0;

    //load kernel
    if(kernel != nullptr) {
        delete[] kernel;
        kernel = nullptr;
    }

    QImage tmp; //error found here?
    if(!tmp.load(recipe.getMarkPath())) {
        return RESULT_RECIPE_ERROR;
    }

    tmp = tmp.convertToFormat(QImage::Format_Grayscale8);
    kernelWidth = tmp.width();
    kernelHeight = tmp.height();
    kernel = new float[kernelWidth * kernelHeight];

    for(unsigned y = 0; y < kernelHeight; y++) {
        for(unsigned x = 0; x < kernelWidth; x++) {
            kernel[kernelWidth*y + x] = ((tmp.pixel(x, y) & 0xFF) > 127 ? 1 : -1);
        }
    }

#ifdef DEBUG_MODE_PROCESS_CONTROL
    printf("[ProcessControl]   Alignment mark loaded into kernel\n");
    fflush(stdout);
#endif

    if(!tmp.load(recipe.getPatternPath())) {
        return RESULT_RECIPE_ERROR;
    }

    tmp = tmp.convertToFormat(QImage::Format_Grayscale8);
    unsigned imageWidth = tmp.width();
    unsigned imageHeight = tmp.height();
    unsigned char *pattern = new unsigned char[imageWidth * imageHeight];

    for(unsigned y = 0; y < imageHeight; y++) {
        for(unsigned x = 0; x < imageWidth; x++) {
            pattern[imageWidth*y + x] = tmp.pixel(x, y) & 0xFF;
        }
    }

    imageProcessor.setImage(pattern, imageWidth, imageHeight);
    delete[] pattern;

#ifdef DEBUG_MODE_PROCESS_CONTROL
    unsigned w, h;
    unsigned char *img = imageProcessor.getResult(w, h);
    for(unsigned y = 0; y < h; y++) {
        for(unsigned x = 0; x < w; x++) {
            if(img[imageWidth*y + x] != 0) {
                //printf("[ProcessControl]   Pattern[%4d][%4d]=%d\n", y, x, img[imageWidth*y + x]);
                //fflush(stdout);
            }
        }
    }

    printf("[ProcessControl]   Pattern loaded into image processor; performing cross-correlation\n");
    fflush(stdout);
#endif

    imageProcessor.crossCorrelate(kernel, kernelHeight, kernelHeight, false);

#ifdef DEBUG_MODE_PROCESS_CONTROL
    img = imageProcessor.getResult(w, h);
    for(unsigned y = 0; y < h; y++) {
        for(unsigned x = 0; x < w; x++) {
            if(img[imageWidth*y + x] != 0) {
                //printf("[ProcessControl]   CC[%4d][%4d]=%d\n", y, x, img[imageWidth*y + x]);
                //fflush(stdout);
            }
        }
    }

    printf("[ProcessControl]   Beginning thresholding on pattern\n");
    fflush(stdout);
#endif

    int threshWidth = -1;
    int maxThreshWidth = -1;
    int nMax = 0;

    //should find a better way to do this
    for(int n = 1; n <= 16; n++) {
        threshWidth = imageProcessor.threshold(n);
        if(threshWidth > maxThreshWidth) {
            maxThreshWidth = threshWidth;
            nMax = n;
        }
    }

    if(nMax == 0) {
#ifdef DEBUG_MODE_PROCESS_CONTROL
        printf("[ProcessControl]   No threshold found\n");
        fflush(stdout);
#endif
        return RESULT_IMAGE_ERROR;
    }

    numPoints = nMax;
    imageProcessor.threshold(numPoints);

#ifdef DEBUG_MODE_PROCESS_CONTROL
    printf("[ProcessControl]   %d alignment marks found; threshold width is %d\n", nMax, maxThreshWidth);
    fflush(stdout);
#endif

    if(patternPoints != nullptr){
        delete[] patternPoints;
        patternPoints = nullptr;
    }

    ImageProcessor::Point *tmpPt = imageProcessor.sortPoints(numPoints);
    patternPoints = new ImageProcessor::Point[numPoints];

    for(unsigned i = 0; i < numPoints; i++) {
        patternPoints[i] = tmpPt[i];
    }

#ifdef DEBUG_MODE_PROCESS_CONTROL
    printf("[ProcessControl]   Points sorted\n");
    fflush(stdout);
#endif

    return RESULT_GOOD;
}

enum ControlResult enterReady() {
#ifdef DEBUG_MODE_PROCESS_CONTROL
    printf("[ProcessControl] Entering STATE_READY\n");
    fflush(stdout);
#endif

    //if wafer view does not exist
        //create wafer view
    return RESULT_GOOD;
}

enum ControlResult executeReady() {
#ifdef DEBUG_MODE_PROCESS_CONTROL
    printf("[ProcessControl] Executing STATE_READY\n");
    fflush(stdout);
#endif

    if(shouldStart) {
        nextState = STATE_COARSE_ALIGN;
        shouldStart = false;
    }

    //update wafer view
    return RESULT_GOOD;
}

enum ControlResult exitReady() {
#ifdef DEBUG_MODE_PROCESS_CONTROL
    printf("[ProcessControl] Exiting STATE_READY\n");
    fflush(stdout);
#endif

    dieNumber = 0;
    //wait for motors to be not moving
    return RESULT_GOOD;
}

enum ControlResult enterCoarseAlign() {
#ifdef DEBUG_MODE_PROCESS_CONTROL
    printf("[ProcessControl] Entering STATE_COARSE_ALIGN\n");
    fflush(stdout);
#endif

    //get wafer coordinates in millimeters
    float xmm = recipe.getDiePositions()[dieNumber].x;
    float ymm = recipe.getDiePositions()[dieNumber].y;

    //convert to motor coordinates
    __u32 motorX = stage_controller::millimetersToMicrosteps(xmm);
    __u32 motorY = stage_controller::millimetersToMicrosteps(ymm);

    //move motors
    if(stage_controller::setPosition(motorX, motorY)) {
        return RESULT_GOOD;
    }
    else {
        return RESULT_MOTOR_ERROR;
    }
}

enum ControlResult executeCoarseAlign() {
#ifdef DEBUG_MODE_PROCESS_CONTROL
    printf("[ProcessControl] Executing STATE_COARSE_ALIGN\n");
    fflush(stdout);
#endif
    __u32 x, y;
    __u8 stat;

    //get x and y position of stage
    if(!stage_controller::getPosition(x, y, stat)) {
        return RESULT_MOTOR_ERROR;
    }

    //if finished moving, go to next state
    if(stat == stage_controller::STAGE_IN_POSITION) {
        if(recipe.isFirstLayer()) {
            nextState = STATE_EXPOSE;
        }
        else {
            nextState = STATE_FINE_ALIGN_IMAGE;
        }
    }

    //convert to wafer coordinates
    currentX = stage_controller::microstepsToMillimeters(x);
    currentY = stage_controller::microstepsToMillimeters(y);

    //TODO: update wafer display on UI (if applicable)

    return RESULT_GOOD;
}

enum ControlResult exitCoarseAlign() {
#ifdef DEBUG_MODE_PROCESS_CONTROL
    printf("[ProcessControl] Exiting STATE_COARSE_ALIGN\n");
    fflush(stdout);
#endif
    //send halt command
    if(stage_controller::halt()) {
        return RESULT_GOOD;
    }
    else {
        return RESULT_MOTOR_ERROR;
    }
}

enum ControlResult enterFineAlignImage() {
#ifdef DEBUG_MODE_PROCESS_CONTROL
    printf("[ProcessControl] Entering STATE_FINE_ALIGN_IMAGE\n");
    fflush(stdout);
#endif

    //capture image and check for error
    if(!camera_module::captureImage()) {
#ifdef DEBUG_MODE_PROCESS_CONTROL
        printf("[ProcessControl]   Could not capture image\n");
        fflush(stdout);
#endif
        return RESULT_CAMERA_ERROR;
    }

    return RESULT_GOOD;
}

enum ControlResult executeFineAlignImage() {
#ifdef DEBUG_MODE_PROCESS_CONTROL
    printf("[ProcessControl] Executing STATE_FINE_ALIGN_IMAGE\n");
    fflush(stdout);
#endif

    //wait for camera to capture image
    if(camera_module::stillImageReady()) {
#ifdef DEBUG_MODE_PROCESS_CONTROL
        printf("[ProcessControl]   Still image captured\n");
        fflush(stdout);
#endif
        QImage tmp = camera_module::stillImage->getImage();
        tmp = tmp.convertToFormat(QImage::Format_Grayscale8).scaledToWidth(256).mirrored(true, false);
        unsigned imageWidth = tmp.width();
        unsigned imageHeight = tmp.height();
        unsigned char *copy = new unsigned char[imageWidth * imageHeight];

        for(unsigned y = 0; y < imageHeight; y++) {
            for(unsigned x = 0; x < imageWidth; x++) {
                copy[imageWidth*y + x] = tmp.pixel(x, y) & 0xFF;
            }
        }

        imageProcessor.setImage(copy, imageWidth, imageHeight);
        delete[] copy;

#ifdef DEBUG_MODE_PROCESS_CONTROL
        printf("[ProcessControl]   Captured image is %dx%d\n", imageWidth, imageHeight);
        printf("[ProcessControl]   Preprocessing captured image\n");
        fflush(stdout);
#endif
        imageProcessor.preprocess();

#ifdef DEBUG_MODE_PROCESS_CONTROL
        printf("[ProcessControl]   Performing cross-correlation\n");
        fflush(stdout);
#endif
        imageProcessor.crossCorrelate(kernel, kernelHeight, kernelHeight, false);
        unsigned char *img = imageProcessor.getResult(imageWidth, imageHeight);

        for(int y = 0; y < (int) imageHeight; y++) {
            for(int x = 0; x < (int) imageWidth; x++) {
                if(img[imageWidth*y + x] < 208);
                    //img[imageWidth*y + x] = 0;
            }
        }

#ifdef DEBUG_MODE_PROCESS_CONTROL
        printf("[ProcessControl]   Performing thresholding\n");
        fflush(stdout);
#endif
        int threshRange = imageProcessor.threshold(numPoints);

        if(threshRange < 0) {
#ifdef DEBUG_MODE_PROCESS_CONTROL
            printf("[ProcessControl]   Bad image.\n");
            fflush(stdout);
#endif
            return RESULT_IMAGE_ERROR;
        }

        imageProcessor.sortPoints(numPoints);
        imageProcessor.scalePoints(patternPoints);
        disp = imageProcessor.calcDisplacement(patternPoints);

        double distance = sqrt(disp.x*disp.x + disp.y*disp.y); //absolute value of displacement
        distance /= kernelWidth; //scale by kernel size?

#ifdef DEBUG_MODE_PROCESS_CONTROL
            printf("[ProcessControl]   Displacement is (%d,%d)\n", disp.x, disp.y);
            fflush(stdout);
#endif

        if(distance < 0.1) { //adjust this value as needed
            nextState = STATE_EXPOSE;
        }
        else {
            nextState = STATE_FINE_ALIGN_MOTOR;
        }
    }
    else {
#ifdef DEBUG_MODE_PROCESS_CONTROL
        printf("[ProcessControl]   No image acquired\n");
        fflush(stdout);
#endif
    }

    return RESULT_GOOD;
}

enum ControlResult exitFineAlignImage() {
#ifdef DEBUG_MODE_PROCESS_CONTROL
    printf("[ProcessControl] Exiting STATE_FINE_ALIGN_IMAGE\n");
    fflush(stdout);
#endif

    return RESULT_GOOD;
}

//send adjustment commands to motors
enum ControlResult enterFineAlignMotor() {
#ifdef DEBUG_MODE_PROCESS_CONTROL
    printf("[ProcessControl] Entering STATE_FINE_ALIGN_MOTOR\n");
    fflush(stdout);
#endif
    unsigned x, y;
    unsigned char stat;

    //get x and y position of stage
    if(!stage_controller::getPosition(x, y, stat)) {
        return RESULT_MOTOR_ERROR;
    }

    //adjust by displacement value
    x += ALIGN_ALPHA * disp.x;
    y += ALIGN_ALPHA * disp.y;

    //move motors
    if(!stage_controller::setPosition(x, y)) {
        return RESULT_MOTOR_ERROR;
    }

    return RESULT_GOOD;
}

enum ControlResult executeFineAlignMotor() {
#ifdef DEBUG_MODE_PROCESS_CONTROL
    printf("[ProcessControl] Executing STATE_FINE_ALIGN_MOTOR\n");
    fflush(stdout);
#endif
    unsigned x, y;
    unsigned char stat;

    //get x and y position of stage
    if(!stage_controller::getPosition(x, y, stat)) {
        return RESULT_MOTOR_ERROR;
    }

    //if finished moving, go to next state
    if(stat == stage_controller::STAGE_IN_POSITION) {
        nextState = STATE_FINE_ALIGN_IMAGE;
    }

    //convert to wafer coordinates
    currentX = stage_controller::microstepsToMillimeters(x);
    currentY = stage_controller::microstepsToMillimeters(y);

    return RESULT_GOOD;
}

enum ControlResult exitFineAlignMotor() {
#ifdef DEBUG_MODE_PROCESS_CONTROL
    printf("[ProcessControl] Exiting STATE_FINE_ALIGN_MOTOR\n");
    fflush(stdout);
#endif
    //send halt command
    if(!stage_controller::halt()) {
        return RESULT_MOTOR_ERROR;
    }

    //CALIB command?

    return RESULT_GOOD;
}

enum ControlResult enterExpose() {
#ifdef DEBUG_MODE_PROCESS_CONTROL
    printf("[ProcessControl] Entering STATE_EXPOSE\n");
    fflush(stdout);
#endif

    //turn projector image on and begin timer with time from recipe (should be precise!)
    projector_module::show(); //must modify this
    return RESULT_GOOD;
}

//executeExpose is not needed, but showing exp. time and time left might help w/ debugging
enum ControlResult executeExpose() {
#ifdef DEBUG_MODE_PROCESS_CONTROL
    printf("[ProcessControl] Executing STATE_EXPOSE\n");
    fflush(stdout);
#endif

    bool durationPassed = true; //must modify this

    if(durationPassed) {
        projector_module::hide();
        dieNumber++;

        if(dieNumber < (int) recipe.getDiePositions().size()) {
            nextState = STATE_COARSE_ALIGN;
        }
        else {
            nextState = STATE_READY;
        }
    }

    return RESULT_GOOD;
}

enum ControlResult exitExpose() {
#ifdef DEBUG_MODE_PROCESS_CONTROL
    printf("[ProcessControl] Exiting STATE_EXPOSE\n");
    fflush(stdout);
#endif
    return RESULT_GOOD;
}

}

#endif // PROCESSCONTROL_HPP
