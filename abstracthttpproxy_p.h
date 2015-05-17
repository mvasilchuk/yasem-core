#ifndef ABSTRACTHTTPPROXYPRIVATE_H
#define ABSTRACTHTTPPROXYPRIVATE_H

#include "abstracthttpproxy.h"

namespace yasem {

class AbstractHttpProxyPrivate
{
public:
    explicit AbstractHttpProxyPrivate(AbstractHttpProxy* q) :
        q_ptr(q),
        m_host_name(""),
        m_port(0),
        m_max_rps(0)
    {
    }

    virtual ~AbstractHttpProxyPrivate() {}
protected:
    AbstractHttpProxy *q_ptr;
    QString m_host_name;
    quint16 m_port;
    quint16 m_max_rps;

    friend class AbstractHttpProxy;
};

}

#endif // ABSTRACTHTTPPROXYPRIVATE_H
