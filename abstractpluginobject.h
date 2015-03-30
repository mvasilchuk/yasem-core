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
    explicit AbstractPluginObject(Plugin* plugin):
        QObject((QObject*)plugin),
        m_plugin(plugin){}
    virtual ~AbstractPluginObject(){};
    virtual PluginObjectResult init() = 0;
    virtual PluginObjectResult deinit() = 0;
    Plugin* plugin() {
        return m_plugin;
    }

protected:
    Plugin* m_plugin;
};

}

#endif // ABSTRACTPLUGINOBJECT

