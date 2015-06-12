#include "plugin.h"
#include "plugin_p.h"
#include "abstractpluginobject.h"
#include "crashhandler.h"

using namespace yasem;

Plugin::Plugin(QObject* parent):
    QObject(parent),
    d_ptr(new PluginPrivate(this))
{
#ifdef USE_BREAKPAD
    Breakpad::CrashHandler::instance()->init();
#endif //USE_BREAKPAD
}

Plugin::~Plugin()
{

}

bool Plugin::hasFlag(PluginFlag flag)
{
    return (getFlags() & flag) == flag;
}

void Plugin::addFlag(PluginFlag flag)
{
    this->setFlags((PluginFlag)(getFlags() | flag));
}

PluginErrorCodes Plugin::initialize()
{
    for(AbstractPluginObject* obj: roles().values())
    {
        obj->init();
        obj->setInitialized(true);
    }
    return PLUGIN_ERROR_NO_ERROR;
}

PluginErrorCodes Plugin::deinitialize()
{
    for(AbstractPluginObject* obj: roles().values())
    {
        obj->deinit();
        obj->setInitialized(false);
    }
    return PLUGIN_ERROR_NO_ERROR;
}

bool Plugin::has_role(PluginRole role)
{
    Q_D(Plugin);
    return d->m_role_list.contains(role);
}

QHash<PluginRole, AbstractPluginObject*> Plugin::roles()
{
    Q_D(Plugin);
    return d->m_role_list;
}

QList<PluginDependency> Plugin::dependencies()
{
    Q_D(Plugin);
    return d->m_dependency_list;
}

QString Plugin::getIID()
{
    Q_D(Plugin);
    return d->m_IID;
}

void Plugin::setIID(const QString &iid)
{
    Q_D(Plugin);
    d->m_IID = iid;
}

QString Plugin::getClassName()
{
    Q_D(Plugin);
    return d->m_className;
}

void Plugin::setClassName(const QString &className)
{
    Q_D(Plugin);
    d->m_className = className;
}

QString Plugin::getId()
{
    Q_D(Plugin);
    return d->m_id;
}

void Plugin::setId(const QString &id)
{
    Q_D(Plugin);
    d->m_id = id;
}

QString Plugin::getVersion()
{
    Q_D(Plugin);
    return d->m_version;
}

void Plugin::setVersion(const QString &version)
{
    Q_D(Plugin);
    d->m_version = version;
}

QString Plugin::getRevision() const
{
    return GIT_VERSION;
}

QString Plugin::getName()
{
    Q_D(Plugin);
    return d->m_name;
}

void Plugin::setName(const QString &name)
{
    Q_D(Plugin);
    d->m_name = name;
}

QJsonObject Plugin::getMetadata()
{
    Q_D(Plugin);
    return d->m_metadata;
}

void Plugin::setMetadata(const QJsonObject &metadata)
{
    Q_D(Plugin);
    d->m_metadata = metadata;
}

PluginFlag Plugin::getFlags()
{
    Q_D(Plugin);
    return d->m_flags;
}

void Plugin::setFlags(const PluginFlag &flags)
{
    Q_D(Plugin);
    d->m_flags = flags;
}

PluginState Plugin::getState()
{
    Q_D(Plugin);
    return d->m_state;
}

void Plugin::setState(const PluginState &state)
{
    Q_D(Plugin);
    d->m_state = state;

    switch(state)
    {
        case PLUGIN_STATE_INITIALIZED:              { emit initialized();  break; }
        case PLUGIN_STATE_NOT_INITIALIZED:          { emit deinitialized();  break; }
        case PLUGIN_STATE_CONFLICT:                 { emit got_conflict();  break; }
        case PLUGIN_STATE_DISABLED:                 { emit disabled();  break; }
        case PLUGIN_STATE_ERROR_STATE:              { emit error_happened();  break; }
        case PLUGIN_STATE_DISABLED_BY_DEPENDENCY:   { emit disabled_by_dependency();  break; }
        case PLUGIN_STATE_WAITING_FOR_DEPENDENCY:   { emit waiting_for_dependency();  break; }
        default: {
            WARN() << "Unknown plugin state" << state << "for" << getName();
            break;
        }
    }
}

QString Plugin::getStateDescription()
{
    QString result;
    switch(getState())
    {
        case PLUGIN_STATE_UNKNOWN:                  { result = "Unknown"; break; }
        case PLUGIN_STATE_ERROR_STATE:              { result = "Error"; break; }
        case PLUGIN_STATE_INITIALIZED:              { result = "Working"; break; }
        case PLUGIN_STATE_NOT_INITIALIZED:          { result = "Not initialized"; break; }
        case PLUGIN_STATE_WAITING_FOR_DEPENDENCY:   { result = "Wait4dependency"; break; }
        case PLUGIN_STATE_DISABLED:                 { result = "Disabled"; break; }
        case PLUGIN_STATE_CONFLICT:                 { result = "Conflict"; break; }
        case PLUGIN_STATE_DISABLED_BY_DEPENDENCY:   { result = "Disabled by dep."; break; }
        default:                                    { result = QString::number(getState()); break; }
    }
    return result;
}

bool Plugin::isActive()
{
    Q_D(Plugin);
    return d->m_active;
}

void Plugin::setActive(bool active)
{
    Q_D(Plugin);
    d->m_active = active;
}

QList<Plugin *> Plugin::getRuntimeConflicts()
{
    Q_D(Plugin);
    return d->m_runtime_conflicts;
}

void Plugin::addRuntimeConflict(Plugin *plugin)
{
    Q_D(Plugin);
    d->m_runtime_conflicts.append(plugin);
}

QList<PluginConflict> Plugin::getStaticConflicts()
{
    Q_D(Plugin);
    return d->m_static_conflicts;
}

void Plugin::add_dependency(const PluginDependency &dependency)
{
    Q_D(Plugin);
    d->m_dependency_list.append(dependency);
}

void Plugin::add_static_conflict(const PluginConflict &conflict_info)
{
    Q_D(Plugin);
    d->m_static_conflicts.append(conflict_info);
}

void Plugin::register_role(PluginRole role, AbstractPluginObject* obj)
{
    Q_D(Plugin);
    d->m_role_list.insert(role, obj);
}

