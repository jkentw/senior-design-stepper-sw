#ifndef CONTROLINTERFACE_HPP
#define CONTROLINTERFACE_HPP

#include <QObject>

#include "ProcessControl.hpp"

class ControlInterface : public QObject {
    Q_OBJECT

public:
    ControlInterface() : QObject(nullptr) {}

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

        return status;
    }

    Q_INVOKABLE
    void start() {
        process_control::start();
    }

    Q_INVOKABLE
    void abort() {
        process_control::abort();
    }

signals:
    void setStartStopEnabled(bool enabled);

};

#endif // CONTROLINTERFACE_HPP
