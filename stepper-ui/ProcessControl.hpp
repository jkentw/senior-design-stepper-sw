#ifndef PROCESSCONTROL_HPP
#define PROCESSCONTROL_HPP

#include "config.hpp"

#include "Recipe.hpp"
#include "cameramodule.hpp"
#include "projectormodule.hpp"
#include "stagecontroller.h"

namespace process_control {

//main variables
Recipe recipe;
int dieNumber = 0;

//last read motor position in wafer coordinates (millimeters)
float currentX = 0;
float currentY = 0;

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
    RESULT_I2C_BUF_ERROR
};

enum ControlState getState(); //returns currentState
enum ControlResult setState(enum ControlState newState); //forces next state to newState
enum ControlResult update();  //executes current state or transitions to another state
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
//enum ControlResult executeReset();
enum ControlResult executeAwaitUpload();
enum ControlResult executeReady();
enum ControlResult executeCoarseAlign();
enum ControlResult executeFineAlignImage();
enum ControlResult executeFineAlignMotor();
//static enum ControlResult executeExpose();

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

    if(shouldAbort) {
        nextState = STATE_RESET; //exit from current state and reset
    }

    if(currentState == nextState) {
        //state execution
        switch(currentState) {
        case STATE_RESET:
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
    }

    currentState = nextState;

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

enum ControlResult abort() {
    shouldAbort = true;
    return update();
}

enum ControlResult enterReset() {
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

    nextState = STATE_AWAIT_UPLOAD;

    return RESULT_GOOD;
}

//no need for executeReset()

enum ControlResult exitReset() {
    if(!projector_module::openProjector()) {
        nextState = STATE_ERROR;
        return RESULT_PROJECTOR_ERROR;
    }

    if(!camera_module::openCamera()) {
        nextState = STATE_ERROR;
        return RESULT_CAMERA_ERROR;
    }

    if(!stage_controller::openI2c()) {
        nextState = STATE_ERROR;
        return RESULT_I2C_COMM_ERROR;
    }

    return RESULT_GOOD;
}

enum ControlResult enterAwaitUpload() {
    recipe.erase();
    return RESULT_GOOD;
}

enum ControlResult executeAwaitUpload() {
    if(recipe.isValid()) {
        nextState = STATE_READY;
    }

    return RESULT_GOOD;
}

enum ControlResult exitAwaitUpload() {
    dieNumber = 0;
    //locate alignment marks on pattern
    //optimize internal representation, if applicable
    return RESULT_GOOD;
}

enum ControlResult enterReady() {
    //if wafer view does not exist
        //create wafer view
    return RESULT_GOOD;
}

enum ControlResult executeReady() {
    //update wafer view
    return RESULT_GOOD;
}

enum ControlResult exitReady() {
    dieNumber = 0;
    //wait for motors to be not moving
    return RESULT_GOOD;
}

enum ControlResult enterCoarseAlign() {
    //get wafer coordinates in millimeters
    float xmm = recipe.getDiePositions()[dieNumber].x;
    float ymm = recipe.getDiePositions()[dieNumber].y;

    //convert to motor coordinates
    __u32 motorX = (__u32) (xmm * 1000000);
    __u32 motorY = (__u32) (ymm * 1000000);

    //set up x and y positioning commands
    bool status = stage_controller::addFrame(stage_controller::CMD_SETX, motorX);
    status &= stage_controller::addFrame(stage_controller::CMD_SETY, motorY);
    if(!status) return RESULT_I2C_BUF_ERROR;

    //send setX command
    status &= stage_controller::sendNextFrame();
    if(!status) return RESULT_I2C_COMM_ERROR;

    //read response to setX
    if(stage_controller::GOOD != stage_controller::readResponse(NULL, NULL)) return RESULT_I2C_FRAME_ERROR;

    //send setY command
    status &= stage_controller::sendNextFrame();
    if(!status) return RESULT_I2C_COMM_ERROR;

    //read response to setY
    if(stage_controller::GOOD != stage_controller::readResponse(NULL, NULL)) return RESULT_I2C_FRAME_ERROR;

    //TODO: update wafer display on UI (if applicable)

    return RESULT_GOOD;
}

enum ControlResult executeCoarseAlign() {
    //get x and y position of stage
    bool status = stage_controller::addFrame(stage_controller::CMD_GETX, 0);
    status &= stage_controller::addFrame(stage_controller::CMD_GETY, 0);
    if(!status) return RESULT_I2C_BUF_ERROR;

    //send setX command
    status &= stage_controller::sendNextFrame();
    if(!status) return RESULT_I2C_COMM_ERROR;

    //read response to setX
    __u32 x, y;
    __u8 stat;
    if(stage_controller::GOOD != stage_controller::readResponse(&x, &stat)) return RESULT_I2C_FRAME_ERROR;

    //send setY command
    status &= stage_controller::sendNextFrame();
    if(!status) return RESULT_I2C_COMM_ERROR;

    //read response to setY
    if(stage_controller::GOOD != stage_controller::readResponse(&y, &stat)) return RESULT_I2C_FRAME_ERROR;

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
    currentX = x / 1000000.0;
    currentY = y / 1000000.0;

    //TODO: update wafer display on UI (if applicable)

    return RESULT_GOOD;
}

enum ControlResult exitCoarseAlign() {
    //initialize halt command
    bool status = stage_controller::addFrame(stage_controller::CMD_HALT, 0);
    if(!status) return RESULT_I2C_BUF_ERROR;

    //send halt command
    status &= stage_controller::sendNextFrame();
    if(!status) return RESULT_I2C_COMM_ERROR;

    //read response to halt command
    if(stage_controller::GOOD != stage_controller::readResponse(NULL, NULL)) return RESULT_I2C_FRAME_ERROR;

    return RESULT_GOOD;
}

enum ControlResult enterFineAlignImage() {
    //capture image and check for error
    if(!camera_module::captureImage()) {
        return RESULT_CAMERA_ERROR;
    }

    return RESULT_GOOD;
}

enum ControlResult executeFineAlignImage() {
    //wait for camera to capture image
    if(camera_module::stillImageReady()) {
        //TODO:
        //locate alignment marks
        //calculate displacement
        //set global displacement vector?

        double distance = 0; //absolute value of displacement

        if(distance < 0.01) { //adjust this value as needed
            nextState = STATE_EXPOSE;
        }
        else {
            nextState = STATE_FINE_ALIGN_MOTOR;
        }
    }

    return RESULT_GOOD;
}

enum ControlResult exitFineAlignImage() {
    //???
    return RESULT_GOOD;
}

enum ControlResult enterFineAlignMotor() {
    //send adjustment commands to motors
    return RESULT_GOOD;
}

enum ControlResult executeFineAlignMotor() {
    //get x and y position of stage
    bool status = stage_controller::addFrame(stage_controller::CMD_GETX, 0);
    status &= stage_controller::addFrame(stage_controller::CMD_GETY, 0);
    if(!status) return RESULT_I2C_BUF_ERROR;

    //send setX command
    status &= stage_controller::sendNextFrame();
    if(!status) return RESULT_I2C_COMM_ERROR;

    //read response to setX
    __u32 x, y;
    __u8 stat;
    if(stage_controller::GOOD != stage_controller::readResponse(&x, &stat)) return RESULT_I2C_FRAME_ERROR;

    //send setY command
    status &= stage_controller::sendNextFrame();
    if(!status) return RESULT_I2C_COMM_ERROR;

    //read response to setY
    if(stage_controller::GOOD != stage_controller::readResponse(&y, &stat)) return RESULT_I2C_FRAME_ERROR;

    //if finished moving, go to next state
    if(stat == stage_controller::STAGE_IN_POSITION) {
        nextState = STATE_FINE_ALIGN_IMAGE;
    }

    //convert to wafer coordinates
    currentX = x / 1000000.0;
    currentY = y / 1000000.0;

    return RESULT_GOOD;
}

enum ControlResult exitFineAlignMotor() {
    //initialize halt command
    bool status = stage_controller::addFrame(stage_controller::CMD_HALT, 0);
    if(!status) return RESULT_I2C_BUF_ERROR;

    //send halt command
    status &= stage_controller::sendNextFrame();
    if(!status) return RESULT_I2C_COMM_ERROR;

    //read response to halt command
    if(stage_controller::GOOD != stage_controller::readResponse(NULL, NULL)) return RESULT_I2C_FRAME_ERROR;

    //CALIB command?

    return RESULT_GOOD;
}

enum ControlResult enterExpose() {
    //turn projector image on and begin timer with time from recipe (should be precise!)
    return RESULT_GOOD;
}

//executeExpose is not needed, but showing exp. time and time left might help w/ debugging

enum ControlResult exitExpose() {
    projector_module::hide();
    dieNumber++;

    if(dieNumber < (int) recipe.getDiePositions().size()) {
        nextState = STATE_COARSE_ALIGN;
    }
    else {
        nextState = STATE_READY;
    }

    return RESULT_GOOD;
}

}

#endif // PROCESSCONTROL_HPP
