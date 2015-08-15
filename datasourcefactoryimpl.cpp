#include "datasourcefactoryimpl.h"
#include "datasource.h"
#include "macros.h"
#include "datasourceclass.h"

using namespace yasem;

DatasourceFactoryImpl::DatasourceFactoryImpl(QObject* parent):
    DatasourceFactory(parent)
{

}

DatasourceFactoryImpl::~DatasourceFactoryImpl()
{
    STUB();
}

SDK::Datasource* DatasourceFactoryImpl::forProfile(const SDK::Profile *profile)
{
    SDK::Datasource* ds = m_plugin_datasources.value(profile);
    if(ds)
        return ds;

    ds = m_registered_creators.first()->createDatasource(profile);
    m_plugin_datasources.insert(profile, ds);
    return ds;
}

void DatasourceFactoryImpl::registerDatasourceClass(SDK::DatasourceClass* ds_class)
{
    m_registered_creators.append(ds_class);
    DEBUG() << "Registered datasource" << ds_class;
}
