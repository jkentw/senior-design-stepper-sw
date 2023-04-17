#ifndef CONTROLINTERFACE_HPP
#define CONTROLINTERFACE_HPP

#include <QObject>
#include <QTimer>

#include "ProcessControl.hpp"

class ControlInterface : public QObject {
    Q_OBJECT

public:
    QTimer *timer;

    ControlInterface() : QObject(nullptr) {
        permanentImageData = new unsigned char[3000*3000]; //this is enough space
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, QOverload<>::of(&ControlInterface::updateOnce));
        timer->start(1000);
    }

    ~ControlInterface() {
        delete[] permanentImageData;
        timer->stop();
        delete timer;
    }

    Q_INVOKABLE
    bool loadRecipe(QString path) {
       return process_control::recipe.read(path.toStdString().c_str());
    }

    Q_INVOKABLE
    int updateOnce() {
        switch(process_control::getState()) {
        case process_control::STATE_READY:
        case process_control::STATE_COARSE_ALIGN:
        case process_control::STATE_FINE_ALIGN_IMAGE:
        case process_control::STATE_FINE_ALIGN_MOTOR:
        case process_control::STATE_EXPOSE:
            emit setStartStopEnabled(true);
            break;
        case process_control::STATE_RESET:
        case process_control::STATE_AWAIT_UPLOAD:
            emit setStartStopEnabled(false);
            break;
        default:
            break;
        }

        process_control::ControlResult status = process_control::update();

        printf("  Status: %d\n", status);
        fflush(stdout);

        unsigned w, h;
        unsigned char *img = process_control::imageProcessor.getResult(w, h);
        printf("  %d, %d\n", w, h);
        fflush(stdout);


        for(int i = 0; i < (int) (w*h); i++) {
            //permanentImageData[i] = img[i];
        }

        qimg = QImage((uchar *) img, w, h, QImage::Format_Grayscale8);

        QVector<QRgb> colorTable;
        colorTable.clear();

        for(int i = 0; i <= 255; i++) {
            colorTable.append(0xFF000000 | (0x010101 * i));
        }

        qimg.setColorCount(256);
        qimg.setColorTable(colorTable);
        imgProcResult.setImage(&qimg);

        return status;
    }

    Q_INVOKABLE
    void start() {
        process_control::ControlResult status = process_control::start();
        printf("  Status: %d\n", status);
        fflush(stdout);
    }

    Q_INVOKABLE
    void abort() {
        process_control::ControlResult status = process_control::abort();
        printf("  Status: %d\n", status);
        fflush(stdout);
    }

    DynamicImage *getImageProcessorResult() {
        return &imgProcResult;
    }

private:
    DynamicImage imgProcResult;
    QImage qimg;
    unsigned char *permanentImageData;

signals:
    void setStartStopEnabled(bool enabled);

};

#endif // CONTROLINTERFACE_HPP
