#ifndef DATASOURCEPLUGIN_H
#define DATASOURCEPLUGIN_H

#include "enums.h"

#include <QtPlugin>
#include <QHash>

class QString;

namespace yasem {
class Profile;
class DatasourcePluginObject;

class DatasourcePlugin
{
public:
    virtual DatasourcePluginObject* getDatasourceForProfile(Profile* profile) = 0;

protected:
    QHash<Profile*, DatasourcePluginObject*> datasources;
};

}

#define DatasourcePlugin_iid "com.mvas.yasem.DatasourcePlugin/1.0"

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(yasem::DatasourcePlugin, DatasourcePlugin_iid)
QT_END_NAMESPACE

#endif // DATASOURCEPLUGIN_H
