#ifndef NETWORK_H
#define NETWORK_H

#include <QtNetwork/QNetworkInterface>
#include <QList>

namespace yasem
{

class Network {

public:
    virtual ~Network() {}
    virtual bool isConnected() = 0;
    virtual bool isLanConnected() = 0;
    virtual bool isWifiConnected() = 0;
    virtual bool isInterfaceConnected(QNetworkInterface iface) = 0;
    virtual QList<QNetworkInterface> getInterfaces() = 0;
};

}

#endif // NETWORK_H
