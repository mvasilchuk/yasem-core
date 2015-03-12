
#include "webserverplugin_p.h"
#include "plugin_p.h"

using namespace yasem;


WebServerPlugin::WebServerPlugin():
    d_ptr(new WebServerPluginPrivate(this)) {

}

WebServerPlugin::~WebServerPlugin()
{

}

void WebServerPlugin::setRootDirectory(const QString &dir)
{
    Q_D(WebServerPlugin);
    d->m_rootDirectory = dir;
}

QString WebServerPlugin::getRootDirectory()
{
    Q_D(WebServerPlugin);
    return d->m_rootDirectory;
}

