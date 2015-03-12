#ifndef WEBSERVERPLUGIN
#define WEBSERVERPLUGIN

#include "enums.h"
#include "plugin.h"
#include <QtPlugin>
#include <QHash>

class QString;

namespace yasem {

class WebServerPluginPrivate;

class WebServerPlugin
{
    Q_DECLARE_PRIVATE(WebServerPlugin)
public:
    explicit WebServerPlugin();
    virtual ~WebServerPlugin();

    void setRootDirectory(const QString& dir);
    QString getRootDirectory();

protected:
    WebServerPlugin(WebServerPluginPrivate &d): d_ptr(&d) {}
    // allow subclasses to initialize with their own concrete Private
    WebServerPluginPrivate *d_ptr;
};

}

#define WebServerPlugin_iid "com.mvas.yasem.WebServerPlugin/1.0"

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(yasem::WebServerPlugin, WebServerPlugin_iid)
QT_END_NAMESPACE

#endif // WEBSERVERPLUGIN

