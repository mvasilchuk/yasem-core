#ifndef NETWORKIMPL_H
#define NETWORKIMPL_H

#include "core-network.h"

#include <QObject>

namespace yasem
{

class NetworkImpl : public QObject, public SDK::CoreNetwork
{
    Q_OBJECT
public:
    explicit NetworkImpl(QObject *parent = 0);
    virtual ~NetworkImpl();
    bool isConnected();
    bool isLanConnected();
    bool isWifiConnected();
    bool isInterfaceConnected(QNetworkInterface iface);
    QList<QNetworkInterface> getInterfaces();

    SDK::Samba* samba();
};

}

#endif // NETWORKIMPL_H
