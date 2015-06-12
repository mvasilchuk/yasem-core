#ifndef DATASOURCEPLUGIN_H
#define DATASOURCEPLUGIN_H

#include "enums.h"
#include "abstractpluginobject.h"

#include <QtPlugin>
#include <QHash>

class QString;

namespace yasem {
class Profile;
class DatasourcePluginObject;

class DatasourcePlugin: public AbstractPluginObject
{
    Q_OBJECT
public:
    DatasourcePlugin(Plugin* plugin):
        AbstractPluginObject(plugin)
    {

    }
    virtual ~DatasourcePlugin(){}

    virtual DatasourcePluginObject* getDatasourceForProfile(Profile* profile) = 0;


protected:
    QHash<Profile*, DatasourcePluginObject*> datasources;
};

}
#endif // DATASOURCEPLUGIN_H
