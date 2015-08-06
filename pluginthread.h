#ifndef PLUGINTHREAD_H
#define PLUGINTHREAD_H

#include <QThread>

namespace yasem {
namespace SDK {
class Plugin;
}

class PluginThread : public QThread
{
    Q_OBJECT
public:
    explicit PluginThread(SDK::Plugin* m_plugin, QObject *parent = 0);

signals:
public slots:


    // QThread interface
protected:
    SDK::Plugin* m_plugin;
    void run();
};

}

#endif // PLUGINTHREAD_H
