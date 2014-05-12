#ifndef CUSTOMKEYEVENT_H
#define CUSTOMKEYEVENT_H

#include <QString>

namespace yasem
{

class CustomKeyEvent
{
public:
    virtual ~CustomKeyEvent() {}
    enum KeyState {
        KeyDown = 0,
        KeyPress = 1,
        KeyRelease = 2,
        MouseDown = 3,
        MouseClick = 4,
        MouseUp = 5
    };

    enum EventType {
        TypeUnknown = 0,
        TypeKeyboard = 1,
        TypeMouse = 2
    };

    int keyCode;
    int which;
    int keyCode2;
    int which2;
    bool alt;
    bool ctrl;
    bool shift;

    virtual QString toString() = 0;

protected:
    virtual void init(int keyCode, int which, int keyCode2, int which2, bool alt = false, bool ctrl = false, bool shift = false) = 0;

};
}

#endif // CUSTOMKEYEVENT_H
