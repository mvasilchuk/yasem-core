#ifndef GUIPLUGIN_H
#define GUIPLUGIN_H

#include "enums.h"
#include "abstractpluginobject.h"

#include <QtPlugin>

class QRect;
class QMenu;

namespace yasem
{

class GuiPluginObject: public AbstractPluginObject
{
public:
    explicit GuiPluginObject(Plugin* plugin, QObject* parent = NULL):
        AbstractPluginObject(plugin, parent) {}
    virtual ~GuiPluginObject(){}

    virtual QRect getWindowRect() = 0;
    virtual void setWindowRect(const QRect &rect) = 0;

    virtual void setFullscreen(bool fullscreen) = 0;
    virtual bool getFullscreen() = 0;

    virtual void repaintGui() = 0;
    virtual QList<QMenu*> getMenuItems() = 0;
    virtual QRect widgetRect() = 0;

};

}

#endif // GUIPLUGIN_H
