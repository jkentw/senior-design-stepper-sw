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

void testI2c(void *params) {
    printf("beginning test\n");
    fflush(stdout);

    stagecontroller::openI2c();

    __u32 width;
    __u32 height;
    __u32 x;
    __u32 y;

    stagecontroller::addFrame(stagecontroller::CMD_CALIB, 0);
    stagecontroller::addFrame(stagecontroller::CMD_GETWIDTH, 0);
    stagecontroller::addFrame(stagecontroller::CMD_GETHEIGHT, 0);
    stagecontroller::addFrame(stagecontroller::CMD_GETX, 0);
    stagecontroller::addFrame(stagecontroller::CMD_GETY, 0);
    stagecontroller::addFrame(stagecontroller::CMD_SETX, 1000);
    stagecontroller::addFrame(stagecontroller::CMD_SETY, 1000);
    stagecontroller::addFrame(stagecontroller::CMD_HALT, 0);

    stagecontroller::sendNextFrame();
    stagecontroller::sendNextFrame();
    stagecontroller::readResponse(&width);
    stagecontroller::sendNextFrame();
    stagecontroller::readResponse(&height);
    stagecontroller::sendNextFrame();
    stagecontroller::readResponse(&x);
    stagecontroller::sendNextFrame();
    stagecontroller::readResponse(&y);
    stagecontroller::sendNextFrame();
    stagecontroller::sendNextFrame();
    stagecontroller::sendNextFrame();

    printf("width:  %d\n", width);
    printf("height: %d\n", height);
    printf("x:      %d\n", x);
    printf("y:      %d\n", y);
    fflush(stdout);

    stagecontroller::closeI2c();
}

static bool showing = false;

void testProjector(void *params) {
    if(showing) {
        projectormodule::hide();
    }
    else {
        projectormodule::show();
    }

    showing = !showing;
}

void testCamera(void *params) {
    if(camera_module::openCamera()) { //redundant calls simply return true if camera is open
        camera_module::captureImage();
    }
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

    FileSelect fileSelect;
    engine.rootContext()->setContextProperty("FileSelectCpp", &fileSelect);

    //Recipe test code
    Recipe recipe;
    engine.rootContext()->setContextObject(&recipe);
    engine.rootContext()->setContextProperty("RecipeCpp", &recipe);

    //I2C test code
    TestButton i2cTestBtn(testI2c);
    engine.rootContext()->setContextProperty("I2cTestCpp", &i2cTestBtn);

    //projector test code
    TestButton projTestBtn(testProjector);
    engine.rootContext()->setContextProperty("ProjectorTestCpp", &projTestBtn);

    QImage pattern("../../tests/inputs/pattern4.png");
    projectormodule::openProjector();
    projectormodule::setPattern(&pattern);

    DynamicImage *projectorImage = projectormodule::projectedImage;
    engine.addImageProvider(QString("projector"), projectorImage);
    engine.rootContext()->setContextProperty("ProjectorCpp", projectorImage);

    //camera test code
    TestButton cameraTestBtn(testCamera);
    engine.rootContext()->setContextProperty("CameraTestCpp", &cameraTestBtn);

    DynamicImage *liveImage;
    DynamicImage *stillImage;
    if(camera_module::openCamera()) {
        liveImage = camera_module::liveImage;
        engine.addImageProvider(QString("camera_live"), liveImage);
        engine.rootContext()->setContextProperty("CameraLiveCpp", liveImage);

        stillImage = camera_module::stillImage;
        engine.addImageProvider(QString("camera_still"), stillImage);
        engine.rootContext()->setContextProperty("CameraStillCpp", stillImage);
    }

    //end of my code

    engine.load(url);
    return app.exec();
}
