#ifndef TESTBUTTON_HPP
#define TESTBUTTON_HPP

#include <QWidget>
#include <QVariant>

class TestButton : public QObject
{
    Q_OBJECT
public:
    TestButton(void (*action)(QVariant params)) : QObject(nullptr) {
        this->action = action;
    }

    virtual ~TestButton() {}

    Q_INVOKABLE
    void invoke(QVariant params) { //void *params
        action(params);
    }

private:
    void (*action)(QVariant params);
};

#endif // TESTBUTTON_HPP
