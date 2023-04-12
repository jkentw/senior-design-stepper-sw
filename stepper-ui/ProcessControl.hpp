#ifndef PROCESSCONTROL_HPP
#define PROCESSCONTROL_HPP

namespace process_control {

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
    RESULT_ABORTED
};

bool shouldAbort = false;

static enum ControlResult update();    //executes current state or transitions to another state
static enum ControlResult setState(enum ControlState newState); //forces next state to newState

//state entry transition functions
static enum ControlResult enterReset();
static enum ControlResult enterAwaitUpload();
static enum ControlResult enterReady();
static enum ControlResult enterCoarseAlign();
static enum ControlResult enterFineAlignImage();
static enum ControlResult enterFineAlignMotor();
static enum ControlResult enterExpose();

//state functions
//static enum ControlResult executeReset();
static enum ControlResult executeAwaitUpload();
static enum ControlResult executeReady();
static enum ControlResult executeCoarseAlign();
static enum ControlResult executeFineAlignImage();
static enum ControlResult executeFineAlignMotor();
//static enum ControlResult executeExpose();

//state exit transition functions
static enum ControlResult exitReset();
static enum ControlResult exitAwaitUpload();
static enum ControlResult exitReady();
static enum ControlResult exitCoarseAlign();
static enum ControlResult exitFineAlignImage();
static enum ControlResult exitFineAlignMotor();
static enum ControlResult exitExpose();

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
    }

    currentState = nextState;

    if(shouldAbort) {
        result = RESULT_ABORTED;
        shouldAbort = false;
    }

    return result;
}

static enum ControlResult setState(enum ControlState newState) {
    nextState = newState;
    return update();
}

static enum ControlResult enterReset() {
    //if projector_module on
        //turn off
    //if camera_module on
        //turn off
    //if I2C on
        //turn off

    //if no recipe
        //next state is await upload
    //else
        //next state is ready
    return RESULT_GOOD;
}

//probably no need for executeReset()

static enum ControlResult exitReset() {
    //turn on projector (blank image)
    //turn on camera
    //turn on I2C

    //if error, set to state error

    return RESULT_GOOD;
}

static enum ControlResult enterAwaitUpload() {
    //invalidate existing recipe
    return RESULT_GOOD;
}

static enum ControlResult executeAwaitUpload() {
    //if new/different recipe uploaded
        //parse recipe
    //if recipe uploaded and is valid
        //next state is ready state
    //else
        //report error if invalid
    return RESULT_GOOD;
}

static enum ControlResult exitAwaitUpload() {
    //locate alignment marks on pattern
    //optimize internal representation, if applicable
    return RESULT_GOOD;
}

static enum ControlResult enterReady() {
    //if wafer view does not exist
        //create wafer view
    return RESULT_GOOD;
}

static enum ControlResult executeReady() {
    //update wafer view
    return RESULT_GOOD;
}

static enum ControlResult exitReady() {
    //wait for motors to be not moving
    return RESULT_GOOD;
}

static enum ControlResult enterCoarseAlign() {
    //get coordinates of next die from recipe structure
    //update wafer display on UI (if applicable)
    //stage_controller setX and setY
    return RESULT_GOOD;
}

static enum ControlResult executeCoarseAlign() {
    //motor controller getX and getY
    //update interface
    //if status is ready
        //if recipe specifies "alignment layer", set next state to expose
        //else, set next state to fine align camera
    return RESULT_GOOD;
}

static enum ControlResult exitCoarseAlign() {
    //send halt instruction to motor
    return RESULT_GOOD;
}

static enum ControlResult enterFineAlignImage() {
    //capture camera image
    return RESULT_GOOD;
}

static enum ControlResult executeFineAlignImage() {
    //if camera image retrieved
        //next state is STATE_FINE_ALIGN_MOTOR
    return RESULT_GOOD;
}

static enum ControlResult exitFineAlignImage() {
    //locate alignment marks
    //calculate displacement
    //if displacement < epsilon:
        //set next state to expose
    //else
        //send commands to motors
    return RESULT_GOOD;
}

static enum ControlResult enterFineAlignMotor() {
    //send adjustment commands to motors
    return RESULT_GOOD;
}

static enum ControlResult executeFineAlignMotor() {
    //get x and y of motor
    //update wafer view
    //if motor is done
        //next state is STATE_FINE_ALIGN_IMAGE
    return RESULT_GOOD;
}

static enum ControlResult exitFineAlignMotor() {
    //send halt instruction to motor
    return RESULT_GOOD;
}

static enum ControlResult enterExpose() {
    //turn projector image on and begin timer with time from recipe (should be precise!)
    return RESULT_GOOD;
}

//executeExpose is not needed, but showing exp. time and time left might help w/ debugging

static enum ControlResult exitExpose() {
    //turn projector off
    //if there are any dies left
        //load next die
        //next state is coarse align
    //else
        //next state is state_ready
    return RESULT_GOOD;
}

}

#endif // PROCESSCONTROL_HPP
