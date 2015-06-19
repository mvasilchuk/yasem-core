#ifndef PLUGIN_P
#define PLUGIN_P

#include "enums.h"
#include "plugin.h"

namespace yasem {

class Plugin;
class AbstractPluginObject;

class PluginPrivate {
    Q_DECLARE_PUBLIC(Plugin)
public:
    // Constructor that initializes the q-ptr

    explicit PluginPrivate(Plugin* q) :
        q_ptr(q),
        m_IID(""),
        m_className(""),
        m_name(""),
        m_id(""),
        m_loader(NULL),
        m_active(false),
        m_flags(PluginFlag::PLUGIN_FLAG_NONE),
        m_state(PLUGIN_STATE_UNKNOWN),
        m_multithreading_enabled(true)
    {
    }

    virtual ~PluginPrivate() {}

    Plugin *q_ptr; // q-ptr points to the API class
    QHash<PluginRole, AbstractPluginObject*> m_role_list;
    QList<PluginDependency> m_dependency_list;
    QList<PluginConflict> m_static_conflicts;

    QList<Plugin*> m_runtime_conflicts;

    QString m_IID;
    QString m_className;
    QString m_name;
    QString m_version;
    QString m_id;
    QJsonObject m_metadata;
    QPluginLoader m_loader;

    bool m_active;
    PluginFlag m_flags;
    PluginState m_state;
    bool m_multithreading_enabled;

protected:
    PluginPrivate(Plugin &p):
        q_ptr(&p) {

    }
};

}

#endif // PLUGIN_P

