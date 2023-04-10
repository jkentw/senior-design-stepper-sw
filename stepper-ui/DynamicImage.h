#ifndef DYNAMICIMAGE_H
#define DYNAMICIMAGE_H

#include "config.hpp"

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
#ifdef DEBUG_MODE_DYNAMIC_IMAGE
        printf("[DynamicImage] New image set (%p)\n", image);
        fflush(stdout);
#endif
        this->image = image;
        emit updateImage(*image);
    }

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override {
        if(id == "image") {
            QImage scaled;

            if(requestedSize.width() <= 0 || requestedSize.height() <= 0) {
                return *image;
            }
            if(image->width() * requestedSize.height() > image->height() * requestedSize.width()) {
                scaled = image->scaledToWidth(requestedSize.width());
            }
            else {
                scaled = image->scaledToHeight(requestedSize.height());
            }

            size->setWidth(image->width());
            size->setHeight(image->height());

#ifdef DEBUG_MODE_DYNAMIC_IMAGE
            printf("[DynamicImage] Image scaled from %dx%d to %dx%d\n",
                   image->width(), image->height(), scaled.width(), scaled.height());
            fflush(stdout);
#endif

            return scaled;
        }
        else {
            return QImage();
        }
    }

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override {
        if(id == "image") {
            QImage scaled;

            if(requestedSize.width() <= 0 || requestedSize.height() <= 0) {
                return QPixmap::fromImage(*image);
            }
            else if(image->width() * requestedSize.height() > image->height() * requestedSize.width()) {
                scaled = image->scaledToWidth(requestedSize.width());
            }
            else {
                scaled = image->scaledToHeight(requestedSize.height());
            }

            size->setWidth(image->width());
            size->setHeight(image->height());

#ifdef DEBUG_MODE_DYNAMIC_IMAGE
            printf("[DynamicImage] Image scaled from %dx%d to %dx%d\n",
                   image->width(), image->height(), scaled.width(), scaled.height());
            fflush(stdout);
#endif

            return QPixmap::fromImage(scaled);
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
