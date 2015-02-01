#ifndef WEBSERVERPLUGIN
#define WEBSERVERPLUGIN

#include "enums.h"
#include "plugin.h"

#include <QtPlugin>
#include <QHash>

class QString;

namespace yasem {

class WebServerPluginPrivate: public PluginPrivate {
public:
    QString m_rootDirectory;
};

class WebServerPlugin: public virtual Plugin
{
    Q_DECLARE_PRIVATE(WebServerPlugin)
public:
    WebServerPlugin() {};

    void setRootDirectory(const QString& dir)
    {
        Q_D(WebServerPlugin);
        d->m_rootDirectory = dir;
    }

    QString getRootDirectory () {
        Q_D(WebServerPlugin);
        return d->m_rootDirectory;
    }

protected:
   WebServerPlugin(WebServerPluginPrivate &d)
      : Plugin(d) {
   }
};

}

#define WebServerPlugin_iid "com.mvas.yasem.WebServerPlugin/1.0"

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(yasem::WebServerPlugin, WebServerPlugin_iid)
QT_END_NAMESPACE

#endif // WEBSERVERPLUGIN

