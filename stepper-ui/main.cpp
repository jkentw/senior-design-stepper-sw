#include <QApplication>
#include <QQmlApplicationEngine>

#include <QtQml>
#include <QtCore>

#include "imageinput.hpp"
#include "FileSelect.hpp"
//#include "Recipe.hpp"
#include "cameramodule.hpp"

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

    //camera image provider
    CameraModule camera;
    if(CameraModule::initialize()) {
        engine.addImageProvider(QString("camera"), &camera);
        engine.rootContext()->setContextProperty("CameraModuleCpp", &camera);
    }

    FileSelect fileSelect;
    engine.rootContext()->setContextProperty("FileSelectCpp", &fileSelect);

    //Recipe recipe;
    //engine.rootContext()->setContextObject(&recipe);
    //engine.rootContext()->setContextProperty("RecipeCpp", &recipe);

    //end of my code

    engine.load(url);

    return app.exec();
}
