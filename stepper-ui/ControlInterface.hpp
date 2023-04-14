#ifndef CONTROLINTERFACE_HPP
#define CONTROLINTERFACE_HPP

#include <QObject>

#include "ProcessControl.hpp"

class ControlInterface : public QObject {
    Q_OBJECT

public:
    ControlInterface() : QObject(nullptr) {}

    bool loadRecipe(QString path) {
       return process_control::recipe.read(path.toStdString().c_str());
    }
};

#endif // CONTROLINTERFACE_HPP
