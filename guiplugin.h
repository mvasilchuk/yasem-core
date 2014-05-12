#ifndef GUIPLUGIN_H
#define GUIPLUGIN_H

#include "enums.h"

#include <QtPlugin>

class QRect;

namespace yasem
{

class BrowserPlugin;
class GuiPlugin
{
public:
    virtual ~GuiPlugin(){}

    virtual QRect getWindowRect() = 0;
    virtual void setWindowRect(const QRect &rect) = 0;

    virtual void setFullscreen(bool fullscreen) = 0;
    virtual bool getFullscreen() = 0;
};

}

#define GuiPlugin_iid "com.mvas.yasem.GuiPlugin/1.0"

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(yasem::GuiPlugin, GuiPlugin_iid)
QT_END_NAMESPACE

#endif // GUIPLUGIN_H
