#include "pluginthread.h"


using namespace yasem;

PluginThread::PluginThread(Plugin* plugin, QObject *parent) :
    QThread(parent)
{
    this->plugin = plugin;
    plugin->setParent(0);

    setObjectName(QString("Thread of %1").arg(plugin->getName()));
}

void PluginThread::run()
{
    plugin->setState(PLUGIN_STATE_THREAD_STARTED);
    PluginErrorCodes result = plugin->initialize();

    if(result == PLUGIN_ERROR_NO_ERROR)
        plugin->setState(PLUGIN_STATE_INITIALIZED);
    else
        plugin->setState(PLUGIN_STATE_ERROR_STATE);
    exec();
}
