#include <QApplication>
#include <QQmlApplicationEngine>

#include <QtQml>
#include <QtCore>
#include <QScreen>
#include <QWindow>

#include "imageinput.hpp"
#include "FileSelect.hpp"
//#include "Recipe.hpp"

#include "projectormodule.hpp"
#include "testbutton.hpp"

QWindow *mainWin;
QScreen *secondaryScreen;

void testTheProjector(void *params) {
    testProjector();

    QWindow *projWin = mainWin->findChild<QWindow *>(QString::fromUtf8("projectorWindow"));
    printf("Projector window handle retrieved(?)\n");
    fflush(stdout);
    //projWin->setScreen(secondaryScreen);
    //projWin->setPosition(secondaryScreen->geometry().width(), 0);
    printf("Attempted to set second screen\n");
    fflush(stdout);
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
    */

    TestButton projTestBtn(testTheProjector);
    engine.rootContext()->setContextProperty("ProjectorTestCpp", &projTestBtn);

    secondaryScreen = app.screens()[0];
    printf("secondary screen detected\n");
    fflush(stdout);
    mainWin = engine.findChild<QWindow *>(QString::fromUtf8("mainWindow"));
    printf("Main window handle retrieved(?)\n");
    fflush(stdout);

    //end of my code

    engine.load(url);

    return app.exec();
}
