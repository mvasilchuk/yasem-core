#ifndef WEBSERVERPLUGIN_P
#define WEBSERVERPLUGIN_P

#include "webserverplugin.h"
#include "plugin_p.h"

namespace yasem {

class WebServerPluginPrivate
{
public:
    explicit WebServerPluginPrivate(WebServerPlugin* q) :
        q_ptr(q),
        m_rootDirectory("")
    {
    }

    virtual ~WebServerPluginPrivate() {}
protected:
    WebServerPlugin *q_ptr;
public:
    QString m_rootDirectory;
};

}

#endif // WEBSERVERPLUGIN_P

