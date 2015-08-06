#include "pluginthread.h"
#include "plugin.h"

using namespace yasem;

PluginThread::PluginThread(SDK::Plugin* plugin, QObject *parent) :
    QThread(parent)
{
    this->m_plugin = plugin;
    plugin->setParent(0);

    setObjectName(QString("Thread of %1").arg(plugin->getName()));
}

void PluginThread::run()
{
    m_plugin->setState(SDK::PLUGIN_STATE_THREAD_STARTED);
    SDK::PluginErrorCodes result = m_plugin->initialize();

    if(result == SDK::PLUGIN_ERROR_NO_ERROR)
        m_plugin->setState(SDK::PLUGIN_STATE_INITIALIZED);
    else
        m_plugin->setState(SDK::PLUGIN_STATE_ERROR_STATE);
    exec();
}
