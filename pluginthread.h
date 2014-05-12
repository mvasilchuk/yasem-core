#ifndef PLUGINTHREAD_H
#define PLUGINTHREAD_H

#include "plugin.h"

#include <QThread>

namespace yasem
{

class PluginThread : public QThread
{
    Q_OBJECT
public:
    explicit PluginThread(Plugin* plugin, QObject *parent = 0);

signals:
public slots:


    // QThread interface
protected:
    Plugin* plugin;
    void run();
};

}

#endif // PLUGINTHREAD_H
