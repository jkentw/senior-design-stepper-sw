#ifndef CAMERAMODULE_HPP
#define CAMERAMODULE_HPP

#include <QPixmap>
#include <QQuickImageProvider>
#include <QWidget>
#include "amcam.h"

class CameraModule : public QWidget, public QQuickImageProvider
{
    Q_OBJECT
    Q_PROPERTY(NOTIFY imageChanged)

public:
    explicit CameraModule();
    static bool initialize();
    static void terminate();

    Q_INVOKABLE
    static bool captureImage();

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    static void __stdcall callback(unsigned nEvent, void* pCallbackCtx);

    static HAmcam cameraHandle;
    static int width;
    static int height;

    static void* imageData;
    static QPixmap *pixmap;

    static CameraModule *globalInstance;

signals:
    void imageChanged();
};

#endif // CAMERAMODULE_HPP
