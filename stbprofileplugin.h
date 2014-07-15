#ifndef STBPROFILEPLUGIN_H
#define STBPROFILEPLUGIN_H

#include "plugin.h"
#include "macros.h"

#include <QString>
#include <QSize>

namespace yasem
{
class Profile;

class StbProfilePluginPrivate: public PluginPrivate {
public:
};

class StbProfilePlugin: public Plugin
{
    Q_DECLARE_PRIVATE(StbProfilePlugin)
public:
    StbProfilePlugin(): Plugin(*new StbProfilePluginPrivate)
    {}

    virtual QString getProfileClassId() = 0;
    virtual Profile* createProfile(const QString& id = "") = 0;
    virtual void init() = 0;
    virtual QString getIcon(const QSize &size = QSize()) = 0;

protected:
   StbProfilePlugin(StbProfilePluginPrivate &d)
      : Plugin(d) {
   }
};

}

#endif // STBPROFILEPLUGIN_H
