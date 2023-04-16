#ifndef PROCESSCONTROL_HPP
#define PROCESSCONTROL_HPP

#include "config.hpp"

#include "Recipe.hpp"
#include "cameramodule.hpp"
#include "projectormodule.hpp"
#include "stagecontroller.h"
#include "imageprocessor.hpp"

#include <QImage>
#include <cmath>

namespace process_control {

//main variables
extern Recipe recipe;
extern int dieNumber;

extern ImageProcessor imageProcessor;
extern ImageProcessor::Point *patternPoints;
extern unsigned numPoints;
extern ImageProcessor::Point disp;

extern float *kernel;
extern unsigned kernelWidth;
extern unsigned kernelHeight;

//last read motor position in wafer coordinates (millimeters)
extern float currentX;
extern float currentY;
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
};

enum ControlResult {
    RESULT_GOOD,
    RESULT_ABORTED,
    RESULT_ERROR,
    RESULT_PROJECTOR_ERROR,
    RESULT_CAMERA_ERROR,
    RESULT_I2C_COMM_ERROR,
    RESULT_I2C_FRAME_ERROR,
    RESULT_I2C_BUF_ERROR,
    RESULT_RECIPE_ERROR,
    RESULT_IMAGE_ERROR,
};

extern enum ControlState getState(); //returns currentState
extern enum ControlResult setState(enum ControlState newState); //forces next state to newState
extern enum ControlResult update();  //executes current state or transitions to another state
extern enum ControlResult start();
extern enum ControlResult abort();

//state entry transition functions
extern enum ControlResult enterReset();
extern enum ControlResult enterAwaitUpload();
extern enum ControlResult enterReady();
extern enum ControlResult enterCoarseAlign();
extern enum ControlResult enterFineAlignImage();
extern enum ControlResult enterFineAlignMotor();
extern enum ControlResult enterExpose();

//state functions
//extern enum ControlResult executeReset();
extern enum ControlResult executeAwaitUpload();
extern enum ControlResult executeReady();
extern enum ControlResult executeCoarseAlign();
extern enum ControlResult executeFineAlignImage();
extern enum ControlResult executeFineAlignMotor();
//extern enum ControlResult executeExpose();

//state exit transition functions
extern enum ControlResult exitReset();
extern enum ControlResult exitAwaitUpload();
extern enum ControlResult exitReady();
extern enum ControlResult exitCoarseAlign();
extern enum ControlResult exitFineAlignImage();
extern enum ControlResult exitFineAlignMotor();
extern enum ControlResult exitExpose();

}

#endif // PROCESSCONTROL_HPP
