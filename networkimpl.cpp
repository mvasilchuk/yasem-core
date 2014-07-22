#include "networkimpl.h"
#include "sambaimpl.h"
#include "macros.h"

#include <QNetworkInterface>

using namespace yasem;

NetworkImpl::NetworkImpl(QObject *parent) :
    QObject(parent)
{
    samba_impl = NULL;
}

NetworkImpl::~NetworkImpl()
{
    STUB();
    if(samba_impl == NULL)
        delete samba_impl;
}

bool NetworkImpl::isConnected()
{
    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
    bool result = false;

    for (int index = 0; index < ifaces.count(); index++) {
        if (result == false)
            result = isInterfaceConnected(ifaces.at(index));
    }

    return result;
}

bool NetworkImpl::isLanConnected()
{
    QList<QNetworkInterface> ifaces = getInterfaces();

    for (int index = 0; index < ifaces.count(); index++) {
        QNetworkInterface iface = ifaces.at(index);
        if(iface.name().startsWith("eth"))
        {
            if(isInterfaceConnected(iface)) return true;
        }
    }
    return false;
}

bool NetworkImpl::isWifiConnected()
{
    QList<QNetworkInterface> ifaces = getInterfaces();

    for (int index = 0; index < ifaces.count(); index++) {
        QNetworkInterface iface = ifaces.at(index);
        if(iface.name().startsWith("wlan"))
        {
            if(isInterfaceConnected(iface)) return true;
        }
    }
    return false;
}

bool NetworkImpl::isInterfaceConnected(QNetworkInterface iface)
{
    if ( iface.flags().testFlag(QNetworkInterface::IsUp)
         && !iface.flags().testFlag(QNetworkInterface::IsLoopBack))
    {
        for (int j=0; j < iface.addressEntries().count(); j++)
        {
            return true;
        }
    }
    return false;
}

QList<QNetworkInterface> NetworkImpl::getInterfaces()
{
    return QNetworkInterface::allInterfaces();
}


Samba* yasem::NetworkImpl::samba()
{
    if(samba_impl == NULL)
        samba_impl = new SambaImpl(this);
    return samba_impl;
}
