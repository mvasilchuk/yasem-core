#ifndef ABSTRACTPLUGINOBJECT
#define ABSTRACTPLUGINOBJECT

#include <QObject>

namespace yasem {

class Plugin;

enum PluginObjectResult {
    PLUGIN_OBJECT_RESULT_OK,
    PLUGIN_OBJECT_RESULT_ERROR,
};

class AbstractPluginObject: public QObject
{
    Q_OBJECT
public:
    explicit AbstractPluginObject(Plugin* plugin, QObject* parent = NULL):
        m_plugin(plugin),
        m_parent(parent){}
    virtual ~AbstractPluginObject(){};
    virtual PluginObjectResult init() = 0;
    virtual PluginObjectResult deinit() = 0;
    virtual Plugin* plugin() {
        return m_plugin;
    }

protected:
    Plugin* m_plugin;
    QObject* m_parent;
};

}

#endif // ABSTRACTPLUGINOBJECT

