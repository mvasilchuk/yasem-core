#include "abstracthttpproxy.h"
#include "abstracthttpproxy_p.h"

using namespace yasem;

AbstractHttpProxy::AbstractHttpProxy(Plugin* plugin):
    AbstractPluginObject(plugin),
    d_ptr(new AbstractHttpProxyPrivate(this))
{

}

AbstractHttpProxy::~AbstractHttpProxy()
{

}

void AbstractHttpProxy::setHostHame(const QString &host_name)
{
    Q_D(AbstractHttpProxy);
    d->m_host_name = host_name;
}

QString AbstractHttpProxy::hostName()
{
    Q_D(AbstractHttpProxy);
    return d->m_host_name;
}

void AbstractHttpProxy::setPort(quint16 port)
{
    Q_D(AbstractHttpProxy);
    d->m_port = port;
}

quint16 AbstractHttpProxy::port()
{
    Q_D(AbstractHttpProxy);
    return d->m_port;
}

void AbstractHttpProxy::setMaxRequestPerSecond(quint16 max_rps)
{
    Q_D(AbstractHttpProxy);
    d->m_max_rps = max_rps;
}

quint16 AbstractHttpProxy::getMaxRequestPerSecond()
{
    Q_D(AbstractHttpProxy);
    return d->m_max_rps;
}

bool AbstractHttpProxy::isRpsLimited()
{
    Q_D(AbstractHttpProxy);
    return d->m_max_rps > 0;
}

