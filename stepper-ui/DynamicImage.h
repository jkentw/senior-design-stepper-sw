#ifndef DYNAMICIMAGE_H
#define DYNAMICIMAGE_H

#ifdef DEBUG_MODE_GLOBAL
#define DEBUG_MODE_DYNAMIC_IMAGE
#endif

#ifdef DEBUG_MODE_DYNAMIC_IMAGE
#include <cstdio>
#endif

#include <QWidget>
#include <QQuickImageProvider>

class DynamicImage : public QWidget, public QQuickImageProvider
{
    Q_OBJECT
    Q_PROPERTY(NOTIFY updateImage)

public:
    explicit DynamicImage() : QWidget(nullptr), QQuickImageProvider(QQuickImageProvider::Pixmap) {}

    void setImage(QImage *image) {
        this->image = image;
        emit updateImage(*image);
    }

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override {
        if(id == "image") {
            size->setWidth(image->width());
            size->setHeight(image->height());
            return *image;
        }
        else {
            return QImage();
        }
    }

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override {
        if(id == "image") {
            size->setWidth(image->width());
            size->setHeight(image->height());
            return QPixmap::fromImage(*image);
        }
        else {
            return QPixmap();
        }
    }

private:
    QImage *image;

signals:
    void updateImage(QImage img);

};

#endif // DYNAMICIMAGE_H
