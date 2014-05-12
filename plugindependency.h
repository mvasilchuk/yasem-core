#ifndef PLUGINDEPENDENCY_H
#define PLUGINDEPENDENCY_H

#include <QObject>

namespace yasem
{

class Plugin;
class PluginDependency: public QObject
{
    Q_OBJECT
public:
    PluginDependency() {
        required = true;
    }
    virtual ~PluginDependency() {}

    QString role;
    bool required;
    Plugin* plugin;
};

}

#endif // PLUGINDEPENDENCY_H
