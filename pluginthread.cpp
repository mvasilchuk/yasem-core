#include "pluginthread.h"


using namespace yasem;

PluginThread::PluginThread(Plugin* plugin, QObject *parent) :
    QThread(parent)
{
    this->plugin = plugin;
    plugin->setParent(0);
}

void PluginThread::run()
{
    exec();
}
