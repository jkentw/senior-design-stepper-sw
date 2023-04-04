#ifndef TESTBUTTON_HPP
#define TESTBUTTON_HPP

#include <QWidget>

class TestButton : public QObject
{
    Q_OBJECT
public:
    TestButton(void (*action)(void *params)) : QObject(nullptr) {
        this->action = action;
    }

    virtual ~TestButton() {}

    Q_INVOKABLE
    void invoke() { //void *params
        action(nullptr);
    }

private:
    void (*action)(void *params);
};

#endif // TESTBUTTON_HPP
