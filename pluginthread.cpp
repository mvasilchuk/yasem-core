#include "pluginthread.h"
#include "plugin.h"

using namespace yasem;

PluginThread::PluginThread(SDK::Plugin* plugin, QObject *parent) :
    QThread(parent)
{
    this->plugin = plugin;
    plugin->setParent(0);

    setObjectName(QString("Thread of %1").arg(plugin->getName()));
}

void PluginThread::run()
{
    plugin->setState(SDK::PLUGIN_STATE_THREAD_STARTED);
    SDK::PluginErrorCodes result = plugin->initialize();

    if(result == SDK::PLUGIN_ERROR_NO_ERROR)
        plugin->setState(SDK::PLUGIN_STATE_INITIALIZED);
    else
        plugin->setState(SDK::PLUGIN_STATE_ERROR_STATE);
    exec();
}
