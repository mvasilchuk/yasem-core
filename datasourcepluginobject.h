#ifndef DATASOURCE_H
#define DATASOURCE_H

#include "abstractpluginobject.h"
#include "datasourceplugin.h"

#include <QString>
#include <QVariant>

namespace yasem
{

class DatasourcePluginObject: public AbstractPluginObject, public DatasourcePlugin
{
    Q_OBJECT
public:
    DatasourcePluginObject(Plugin* plugin, QObject* parent):
        AbstractPluginObject(plugin, parent){}

    virtual ~DatasourcePluginObject() {};

    virtual bool set(const QString &tag, const QString &name, const int value) = 0;
    virtual int get(const QString &tag, const QString &name, const int defaultValue)  = 0;

    virtual bool set(const QString &tag, const QString &name, const QString &value)  = 0;
    virtual QString get(const QString &tag, const QString &name, const QString &defaultValue = "")  = 0;
};

}

#endif // DATASOURCE_H
