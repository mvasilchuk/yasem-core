#include "pluginthread.h"


using namespace yasem;

PluginThread::PluginThread(Plugin* plugin, QObject *parent) :
    QThread(parent)
{
    this->plugin = plugin;
}

void PluginThread::run()
{
    plugin->initialize();
}
