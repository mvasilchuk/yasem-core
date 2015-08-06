#ifndef DATASOURCEFACTORYIMPL_H
#define DATASOURCEFACTORYIMPL_H

#include "datasourcefactory.h"

#include <QList>
#include <QHash>

namespace yasem {

class DatasourceFactoryImpl: public SDK::DatasourceFactory
{
public:
    DatasourceFactoryImpl(QObject* parent);
    virtual ~DatasourceFactoryImpl();

    QList<SDK::DatasourceClass*> m_registered_creators;
    QHash<const SDK::Profile*, SDK::Datasource*> m_plugin_datasources;

    // DatasourceFactory interface
public:
    virtual SDK::Datasource *forProfile(const SDK::Profile *profile);
    virtual void registerDatasourceClass(SDK::DatasourceClass* ds_class);
};
}



#endif // DATASOURCEFACTORYIMPL_H
