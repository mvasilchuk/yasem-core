#ifndef WEBSERVERPLUGIN
#define WEBSERVERPLUGIN

#include "abstractpluginobject.h"
#include "enums.h"
#include "plugin.h"
#include <QtPlugin>
#include <QHash>

class QString;

namespace yasem {

class WebServerPluginPrivate;

class WebServerPlugin: public AbstractPluginObject
{
    Q_DECLARE_PRIVATE(WebServerPlugin)
public:
    explicit WebServerPlugin(Plugin* plugin);
    virtual ~WebServerPlugin();

    void setRootDirectory(const QString& dir);
    QString getRootDirectory();

protected:
    //WebServerPlugin(WebServerPluginPrivate &d): d_ptr(&d) {}
    // allow subclasses to initialize with their own concrete Private
    WebServerPluginPrivate *d_ptr;
};

}
#endif // WEBSERVERPLUGIN

