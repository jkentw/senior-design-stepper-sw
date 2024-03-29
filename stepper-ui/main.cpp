#include "config.hpp"

#include <QApplication>
#include <QQmlApplicationEngine>

#include <QtQml>
#include <QtCore>
#include <QScreen>
#include <QWindow>

#include "imageinput.hpp"
#include "FileSelect.hpp"
#include "Recipe.hpp"
#include "testbutton.hpp"
#include "stagecontroller.h"
#include "projectormodule.hpp"
#include "cameramodule.hpp"
#include "ControlInterface.hpp"

void testI2c(QVariant params) {
    printf("Beginning test\n");
    fflush(stdout);

    stage_controller::openI2c();

    __u32 width;
    __u32 height;
    __u32 x;
    __u32 y;

    stage_controller::addFrame(stage_controller::CMD_CALIB, 0);
    stage_controller::addFrame(stage_controller::CMD_GETWIDTH, 0);
    stage_controller::addFrame(stage_controller::CMD_GETHEIGHT, 0);
    stage_controller::addFrame(stage_controller::CMD_GETX, 0);
    stage_controller::addFrame(stage_controller::CMD_GETY, 0);
    stage_controller::addFrame(stage_controller::CMD_SETX, 1000);
    stage_controller::addFrame(stage_controller::CMD_SETY, 1000);
    stage_controller::addFrame(stage_controller::CMD_HALT, 0);

    stage_controller::sendNextFrame();
    stage_controller::sendNextFrame();
    stage_controller::readResponse(&width, NULL);
    stage_controller::sendNextFrame();
    stage_controller::readResponse(&height, NULL);
    stage_controller::sendNextFrame();
    stage_controller::readResponse(&x, NULL);
    stage_controller::sendNextFrame();
    stage_controller::readResponse(&y, NULL);
    stage_controller::sendNextFrame();
    stage_controller::sendNextFrame();
    stage_controller::sendNextFrame();

    printf("width:  %d\n", width);
    printf("height: %d\n", height);
    printf("x:      %d\n", x);
    printf("y:      %d\n", y);
    fflush(stdout);

    stage_controller::closeI2c();
}

static bool showing = false;

void testProjector(QVariant params) {
    if(showing) {
        projector_module::hide();
    }
    else {
        projector_module::show();
    }

    showing = !showing;
}

void testCamera(QVariant params) {
    if(camera_module::isOpen()) {
    //    camera_module::closeCamera();
    }

    if(camera_module::openCamera()) {
        camera_module::captureImage();
    }
}

void testRecipe(QVariant params) {
    Recipe recipe;
    recipe.read(params.toString().toStdString().c_str());
}

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    //my code here
    ImageInput imgIn;
    engine.addImageProvider(QString("preview"), &imgIn);
    engine.rootContext()->setContextProperty("ImageInputCpp", &imgIn);


    //Recipe test code
    FileSelect fileSelect;
    engine.rootContext()->setContextProperty("FileSelectCpp", &fileSelect);

    TestButton recipeTestBtn(testRecipe);
    engine.rootContext()->setContextObject(&recipeTestBtn);
    engine.rootContext()->setContextProperty("RecipeTestCpp", &recipeTestBtn);

    //I2C test code
    TestButton i2cTestBtn(testI2c);
    engine.rootContext()->setContextProperty("I2cTestCpp", &i2cTestBtn);

    //projector test code
    TestButton projTestBtn(testProjector);
    engine.rootContext()->setContextProperty("ProjectorTestCpp", &projTestBtn);

    QImage pattern("../../tests/inputs/pattern4.png");
    projector_module::openProjector();
    projector_module::setPattern(&pattern);

    DynamicImage *projectorImage = projector_module::projectedImage;
    engine.addImageProvider(QString("projector"), projectorImage);
    engine.rootContext()->setContextProperty("ProjectorCpp", projectorImage);

    //camera test code
    TestButton cameraTestBtn(testCamera);
    engine.rootContext()->setContextProperty("CameraTestCpp", &cameraTestBtn);

    DynamicImage *liveImage;
    DynamicImage *stillImage;
    if(camera_module::openCamera() || 1) { //bypass check temporarily
        liveImage = camera_module::liveImage;
        engine.addImageProvider(QString("camera_live"), liveImage);
        engine.rootContext()->setContextProperty("CameraLiveCpp", liveImage);

        stillImage = camera_module::stillImage;
        engine.addImageProvider(QString("camera_still"), stillImage);
        engine.rootContext()->setContextProperty("CameraStillCpp", stillImage);
    }

    //process control interface
    ControlInterface control;
    engine.rootContext()->setContextProperty("ControlCpp", &control);

    DynamicImage *imageProcessorResult;
    imageProcessorResult = control.getImageProcessorResult();
    engine.addImageProvider(QString("image_processor"), imageProcessorResult);
    engine.rootContext()->setContextProperty("ProcessorImageCpp", imageProcessorResult);

    //end of my code

    engine.load(url);
    return app.exec();
}
