#include <QApplication>
#include <QQmlApplicationEngine>

#include <QtQml>
#include <QtCore>

#define DEBUG_MODE_GLOBAL

#include "imageinput.hpp"
#include "FileSelect.hpp"
//#include "Recipe.hpp"
#include "testbutton.hpp"
#include "stagecontroller.h"

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


int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    //my code here

    //end of my code

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

    /*
    Recipe recipe;
    //engine.rootContext()->setContextObject(&recipe);
    engine.rootContext()->setContextProperty("RecipeCpp", &recipe);
    //end of my code
    */

    TestButton i2cTestBtn(testI2c);
    engine.rootContext()->setContextProperty("I2cTestCpp", &i2cTestBtn);

    engine.load(url);

    return app.exec();
}
