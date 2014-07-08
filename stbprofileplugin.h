#ifndef STBPROFILEPLUGIN_H
#define STBPROFILEPLUGIN_H

#include "plugin.h"
#include "macros.h"

#include <QString>
#include <QSize>

namespace yasem
{
class Profile;

class StbProfilePlugin: public virtual Plugin
{
public:
    virtual QString getProfileClassId() = 0;
    virtual Profile* createProfile(const QString& id = "") = 0;
    virtual void init() = 0;
    virtual QString getIcon(const QSize &size = QSize()) = 0;
};

}

#endif // STBPROFILEPLUGIN_H
