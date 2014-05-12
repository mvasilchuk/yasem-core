#ifndef STBPROFILEPLUGIN_H
#define STBPROFILEPLUGIN_H

#include "plugin.h"
#include "macros.h"

#include <QString>

namespace yasem
{
class Profile;

class StbProfilePlugin: public virtual Plugin
{
public:
    virtual QString getProfileClassId() = 0;
    virtual Profile* createProfile(const QString& id = "") = 0;

    QString getImage() { return this->image; }
    void setImage(const QString &path) { this->image = path; }

    virtual void init() = 0;
protected:
    QString image;

    //Profile* profileObj;
};

}

#endif // STBPROFILEPLUGIN_H
