#ifndef ABSTRACTHTTPPROXY_H
#define ABSTRACTHTTPPROXY_H

#include "abstractpluginobject.h"

namespace yasem {

class AbstractHttpProxyPrivate;

class AbstractHttpProxy: public AbstractPluginObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(AbstractHttpProxy)
public:
    explicit AbstractHttpProxy(Plugin* plugin);
    virtual ~AbstractHttpProxy();

    virtual void setHostHame(const QString &host_name);
    virtual QString hostName();

    virtual void setPort(quint16 port);
    virtual quint16 port();

    virtual void setMaxRequestPerSecond(quint16 max_rps);
    virtual quint16 getMaxRequestPerSecond();

    virtual bool isRpsLimited();

    virtual bool startServer() = 0;
    virtual bool stopServer() = 0;
    virtual bool isRunning() = 0;

protected:
    //WebServerPlugin(WebServerPluginPrivate &d): d_ptr(&d) {}
    // allow subclasses to initialize with their own concrete Private
    AbstractHttpProxyPrivate *d_ptr;
};

}

#endif // ABSTRACTHTTPPROXY_H
