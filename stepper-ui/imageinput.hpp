#ifndef IMAGEINPUT_H
#define IMAGEINPUT_H

#include <QWidget>
#include <QFileDialog>
#include <QQuickImageProvider>
#include <QByteArray>

#include <iostream>

class ImageInput : public QWidget, public QQuickImageProvider
{
    Q_OBJECT
    Q_PROPERTY(NOTIFY updatePattern)
    Q_PROPERTY(NOTIFY updateMark)

public:
    explicit ImageInput() : QWidget(nullptr), QQuickImageProvider(QQuickImageProvider::Pixmap) {}

    Q_INVOKABLE
    void patternUpload() {
        QString patternPath = QFileDialog::getOpenFileName(this, tr("Choose Pattern"),
                                                           QDir::homePath(),
                                                           tr("Image Files (*.png *.jpg *.bmp)"));

        if(!patternPath.isEmpty()) {
            //std::cout << patternPath.toStdString() << std::endl;

            if(patternImg.load(patternPath)) {
                patternImg = patternImg.convertToFormat(QImage::Format_Mono, Qt::ThresholdDither);
                //std::cout << "Updating pattern..." << std::endl;
                emit updatePattern(patternImg);
                checkForImages();
            }

        }
        else {
            //std::cout << "(Pattern Select Cancelled.)" << std::endl;
        }
    }

    Q_INVOKABLE
    void markUpload() {
        QString markPath = QFileDialog::getOpenFileName(this, tr("Choose Alignment Mark"),
                                                           QDir::homePath(),
                                                           tr("Image Files (*.png *.jpg *.bmp)"));

        if(!markPath.isEmpty()) {
            //std::cout << markPath.toStdString() << std::endl;

            if(markImg.load(markPath)) {
                markImg = markImg.convertToFormat(QImage::Format_Mono, Qt::ThresholdDither);
                //std::cout << "Updating alignment mark..." << std::endl;
                emit updateMark(markImg);
                checkForImages();
            }

        }
        else {
            //std::cout << "(Mark Select Cancelled.)" << std::endl;
        }
    }

    //input images are treated as monochrome
    //output is relative to center of alignment mark image
    Q_INVOKABLE
    void crossCorrelation() {
        //std::cout << "cross corr now" << std::endl;

        //pattern width and height
        int iw = patternImg.width();
        int ih = patternImg.height();

        //kernel width and height
        int kw = markImg.width();
        int kh = markImg.height();

        int kernel[kh][kw];

        for(int v = 0; v < kh; v++) {
            for(int u = 0; u < kw; u++) {
                kernel[v][u] = (markImg.pixel(u, v) & 1) * 2 - 1; //-1 to +1
            }
        }

        //std::cout << "light, dark = " << kernel[kh/2][kw/2] << ", " << kernel[0][0] << std::endl;
        //std::cout << "light, dark = " << markImg.pixel(kh/2, kw/2) << ", " << markImg.pixel(0, 0) << std::endl;

        QImage padded(patternImg.copy(-kw/2, -kh/2, iw+kw/2*2, ih+kh/2*2));
        short *cc = new short[iw * ih];

        int colorIndex = 1;

        for(int y = 0; y < ih+kh/2*2; y++) {
            for(int x = 0; x < kw/2; x++) {
                padded.setPixel(x, y, colorIndex);
            }

            for(int x = iw; x < iw+kw/2*2; x++) {
                padded.setPixel(x, y, colorIndex);
            }
        }

        for(int x = kw/2; x < iw; x++) {
            for(int y = 0; y < kh/2; y++) {
                padded.setPixel(x, y, colorIndex);
            }

            for(int y = ih; y < ih+kh/2*2; y++) {
                padded.setPixel(x, y, colorIndex);
            }
        }

        int ccIdx;
        int maxVal = -1000000;
        int minVal = 1000000;

        std::cout << padded.width() << " " << padded.height() << std::endl;

        for(int y = kh/2; y < ih+kh/2; y++) {
            for(int x = kw/2; x < iw+kw/2; x++) {
                ccIdx = iw * (y-kh/2) + x-kw/2;
                cc[ccIdx] = 0;

                for(int v = -kh/2; v <= kh/2; v++) {
                    for(int u = -kw/2; u <= kw/2; u++) {
                        cc[ccIdx] = cc[ccIdx] + kernel[v+kh/2][u+kw/2] * (padded.pixel(x+u,y+v) & 1);
                    }
                }

                if(cc[ccIdx] > maxVal)
                    maxVal = cc[ccIdx];
                if(cc[ccIdx] < minVal)
                    minVal = cc[ccIdx];
            }
        }

        //std::cout << "min: " << minVal << "; max: " << maxVal << std::endl;

        int maxX = 0;
        int maxY = 0;
        ushort maxNormVal = 0;

        //normalization to 8-bit grayscale
        for(int y = 0; y < ih; y++) {
            for(int x = 0; x < iw; x++) {
                cc[iw * y + x] = (short)((cc[iw * y + x] - minVal)
                        / (float) (maxVal - minVal) * 32767);
                if(cc[iw * y + x] > maxNormVal) {
                    maxNormVal = cc[iw * y + x];
                    maxX = x;
                    maxY = y;
                }
            }
        }

        QImage ccImg((const uchar *) cc, iw, ih, 2*iw, QImage::Format_Grayscale16);
        ccImg = ccImg.convertToFormat(QImage::Format_RGB32);

        //highlight maxima
        for(int y = 0; y < ih; y++) {
            for(int x = 0; x < iw; x++) {
                if(cc[iw * y + x] >= maxNormVal) { //arbitrary threshold
                    ccImg.setPixel(x, y, 0xFFFF007F);
                    std::cout << "Found mark at (" << x << ", " << y << ")" << std::endl;
                }
            }
        }

        patternImg = ccImg;

        //delete[] cc; //memory leak here - must fix

        emit updatePattern(patternImg);
    }

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override {
        if(id == "pattern") {
            //std::cout << "patternImg" << std::endl;
            size->setWidth(patternImg.width());
            size->setHeight(patternImg.height());
            return QPixmap::fromImage(patternImg);
        }
        else if(id == "mark") {
            //std::cout << "markImg" << std::endl;
            size->setWidth(markImg.width());
            size->setHeight(markImg.height());
            return QPixmap::fromImage(markImg);
        }
        else {
            //std::cout << "nothing" << std::endl;
            return QPixmap();
        }
    }

signals:
    void updatePattern(QImage img);
    void updateMark(QImage img);
    void setCrossCorrBtnEnabled(bool enabled);

private:
    QImage patternImg;
    QImage markImg;

    bool checkForImages() {
        bool good = !patternImg.isNull() && !markImg.isNull();
        emit setCrossCorrBtnEnabled(good);
        return good;
    }

};

#endif // IMAGEINPUT_H
