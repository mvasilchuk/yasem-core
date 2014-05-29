#ifndef CORENETWORK_H
#define CORENETWORK_H

#include <QtNetwork/QNetworkInterface>
#include <QList>

namespace yasem
{

class CoreNetwork {

public:
    virtual ~CoreNetwork() {}
    virtual bool isConnected() = 0;
    virtual bool isLanConnected() = 0;
    virtual bool isWifiConnected() = 0;
    virtual bool isInterfaceConnected(QNetworkInterface iface) = 0;
    virtual QList<QNetworkInterface> getInterfaces() = 0;
};

}

#endif // CORENETWORK_H
