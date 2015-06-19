#include "pluginmanagerimpl.h"
#include "core.h"
#include "plugin.h"
#include "stbpluginobject.h"
#include "profilemanager.h"
#include "pluginthread.h"
#include "yasemsettings.h"
#include "guipluginobject.h"

#include <QDir>
#include <QDebug>
#include <QPluginLoader>
#include <QString>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QtCore/QCoreApplication>
#include <QSettings>
#include <QThread>

using namespace yasem;

static const QString PLUGIN_FLAG_NAME_CLIENT = "client";
static const QString PLUGIN_FLAG_NAME_SYSTEM = "system";
static const QString PLUGIN_FLAG_NAME_HIDDEN = "hidden";
static const QString PLUGIN_FLAG_NAME_GUI    = "gui";

PluginManagerImpl::PluginManagerImpl():
    m_plugins_config(dynamic_cast<ConfigContainer*>(Core::instance()->yasem_settings()->findItem(YasemSettings::SETTINGS_GROUP_PLUGINS)))
{
   this->setObjectName("PluginManager");
   #ifdef USE_OSX_BUNDLE
   setPluginDir("Plugins");
#else
    setPluginDir("plugins");
#endif //USE_OSX_BUNDLE

   registerPluginRole(ROLE_ANY,          PluginRoleData(tr("Any role"),      tr("Any role")));
   registerPluginRole(ROLE_STB_API,      PluginRoleData(tr("STB API"),       tr("STB API implementation")));
   registerPluginRole(ROLE_UNSPECIFIED,  PluginRoleData(tr("Unspecified"),   tr("Unspecified role")));
   registerPluginRole(ROLE_UNKNOWN,      PluginRoleData(tr("Unknown role"),  tr("Role is unknown. Plugin may not be compatible with core module!")));

   registerPluginRole(ROLE_BROWSER,      PluginRoleData(tr("Browser"),       tr("Web browser component"),        QList<PluginRole>() << ROLE_BROWSER));
   registerPluginRole(ROLE_MEDIA,        PluginRoleData(tr("Media player"),  tr("Media player component"),       QList<PluginRole>() << ROLE_MEDIA));
   registerPluginRole(ROLE_DATASOURCE,   PluginRoleData(tr("Datasource"),    tr("Datasource component"),         QList<PluginRole>() << ROLE_DATASOURCE));
   registerPluginRole(ROLE_GUI,          PluginRoleData(tr("GUI"),           tr("Graphical user interface"),     QList<PluginRole>() << ROLE_GUI));
   registerPluginRole(ROLE_WEB_GUI,      PluginRoleData(tr("Web GUI"),       tr("Web-based GUI for main screen"),QList<PluginRole>() << ROLE_WEB_GUI));
   registerPluginRole(ROLE_WEB_SERVER,   PluginRoleData(tr("Web server"),    tr("Web server for fake web service emulation"), QList<PluginRole>() << ROLE_WEB_SERVER));
}

PluginErrorCodes PluginManagerImpl::listPlugins()
{
    DEBUG() << "Looking for plugins...";

    DEBUG() << "PluginManager::listPlugins()";
    QStringList pluginIds;
    plugins.clear();

    QDir pluginsDir(qApp->applicationDirPath());
    #if defined(Q_OS_WIN)
        if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
            pluginsDir.cdUp();
    #elif defined(Q_OS_DARWIN)
        #ifdef USE_OSX_BUNDLE
            pluginsDir.cdUp();
        #else
            pluginsDir.cd("../../..");
        #endif

    #endif
    if(!pluginsDir.cd(getPluginDir()))
    {
        ERROR() << "Cannot go to plugins dir:" << getPluginDir();
        return PLUGIN_ERROR_DIR_NOT_FOUND;
    }

    DEBUG() << "Searching for plugins in" << pluginsDir.path();

    foreach (QString fileName, pluginsDir.entryList(QDir::Files | QDir::NoSymLinks | QDir::Readable))
    {

        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));

        Plugin *plugin = qobject_cast<Plugin*>(pluginLoader.instance());

        if (plugin != NULL)
        {
            qDebug() << "Loading plugin from file" << fileName;
            QJsonObject metadata = pluginLoader.metaData().value("MetaData").toObject();
            QString id = metadata.value("id").toString();

            if(!pluginIds.contains(id))
            {
                plugin->setActive(true);

                QString name = metadata.value("name").toString();
                QString version = metadata.value("version").toString();
                QString type = metadata.value("type").toString();

                plugin->setIID(pluginLoader.metaData().value("IID").toString());
                plugin->setClassName(pluginLoader.metaData().value("className").toString());
                plugin->setMetadata(metadata);
                plugin->setId(id);
                plugin->setVersion(version);
                plugin->setName(name);
                plugin->setState(PLUGIN_STATE_NOT_INITIALIZED);
                plugin->setFlags(parseFlags(metadata.value("flags").toString()));

                DEBUG() << "....Registering plugin roles...";
                plugin->register_roles();
                DEBUG() << "....Registering plugin dependencies...";
                plugin->register_dependencies();

                pluginIds.append(id);
                plugins.append(plugin);

                DEBUG() << "....Plugin loaded:" << plugin->getName();

                connect(plugin, &Plugin::loaded, this, &PluginManagerImpl::onPluginLoaded);
                connect(plugin, &Plugin::unloaded, this, &PluginManagerImpl::onPluginUnloaded);
                connect(plugin, &Plugin::initialized, this, &PluginManagerImpl::onPluginInitialized);
                connect(plugin, &Plugin::deinitialized, this, &PluginManagerImpl::onPluginDeinitialized);

                ConfigItem* plugin_info = new ConfigItem(plugin->getId(), plugin->getName(), true, ConfigItem::BOOL);
                m_plugins_config->addItem(plugin_info);
            }

        }
        else
        {
           WARN() << pluginLoader.errorString();
        }
    }
    Core::instance()->yasem_settings()->load(m_plugins_config);
    return PLUGIN_ERROR_NO_ERROR;
}


QList<Plugin*> PluginManagerImpl::getPlugins(PluginRole role, bool active_only)
{
    DEBUG() << QString("getPlugins(role: %1, active only: %2)").arg(role).arg(active_only);

    QList<Plugin*> result;
    foreach(Plugin* plugin, plugins)
    {
        bool add_this = false;
        foreach(PluginRole pluginRole, plugin->roles().keys())
        {
            if(pluginRole == role || role == ROLE_ANY)
            {
                add_this = !active_only || plugin->isActive();
            }
        }

        if(add_this) result.append(plugin);
    }

    return result;
}

AbstractPluginObject* PluginManagerImpl::getByRole(PluginRole role, bool show_warning)
{
    for(int index = 0; index < plugins.size(); index++)
    {
        Plugin *plugin = plugins.at(index);
        if(!plugin->isActive()) continue;
        for(PluginRole pluginRole: plugin->roles().keys())
        {
            if(pluginRole == role)
            {
                Q_ASSERT(plugin);
                AbstractPluginObject* obj = plugin->roles().value(pluginRole);
                if(obj->isInitialized())
                    return obj;
            }
        }
    }
    if(show_warning)
        ERROR() << qPrintable(QString("Plugin for role %1 not found!").arg(role));
    return NULL;
}

QList<AbstractPluginObject *> PluginManagerImpl::getAllByRole(PluginRole role, bool active_only)
{
    QList<AbstractPluginObject *> result;
    for(int index = 0; index < plugins.size(); index++)
    {
        Plugin *plugin = plugins.at(index);
        if(active_only && !plugin->isActive()) continue;

        for(PluginRole pluginRole: plugin->roles().keys())
        {
            if(pluginRole == role)
            {
                AbstractPluginObject* obj = plugin->roles().value(pluginRole);
                if(!active_only || obj->isInitialized())
                    result.append(obj);
            }
        }
    }
    return result;
}

Plugin *PluginManagerImpl::getByIID(const QString &iid)
{

    for(int index = 0; index < plugins.size(); index++)
    {
        Plugin *plugin = plugins.at(index);
        if(!plugin->isActive()) continue;
        if(plugin->getIID() == iid)
        {
            return plugin;
        }
    }
    return NULL;
}

PluginFlag PluginManagerImpl::parseFlags(const QString &flagsStr)
{
    QStringList flags = flagsStr.split("|");
    PluginFlag result = PLUGIN_FLAG_NONE;
    foreach(QString flag, flags)
    {
        if(flag == PLUGIN_FLAG_NAME_CLIENT)
            result = (PluginFlag)(result | PLUGIN_FLAG_CLIENT);
        else if(flag == PLUGIN_FLAG_NAME_SYSTEM)
            result = (PluginFlag)(result | PLUGIN_FLAG_SYSTEM);
        else if(flag == PLUGIN_FLAG_NAME_HIDDEN)
            result = (PluginFlag)(result | PLUGIN_FLAG_HIDDEN);
        else if(flag == PLUGIN_FLAG_NAME_GUI)
            result = (PluginFlag)(result | PLUGIN_FLAG_GUI);
    }
    return result;
}

bool PluginManagerImpl::pluginHasConflicts(Plugin *plugin)
{
    QHash<Plugin*, QList<PluginRoleData>> conflict_list;
    for(Plugin* pl: plugins)
    {
        QList<PluginRoleData> conflicts;

        // Skip itself and plugins that hasn't been loaded yet
        if(pl == plugin || pl->getState() != PLUGIN_STATE_INITIALIZED) continue;

        // First check static conflicts that has been registered by plugin
        // @see: registerPluginRole()

        bool has_conflict = false;
        for(PluginConflict conflict: pl->getStaticConflicts())
        {
            switch (conflict.type()) {
                case PLUGIN_CONFLICTS_BY_ID:
                    if(conflict.id() == plugin->getId())
                        has_conflict = true;
                    break;
                case PLUGIN_CONFLICTS_BY_NAME:
                    if(conflict.name() == plugin->getName())
                        has_conflict = true;
                    break;
                case PLUGIN_CONFLICTS_BY_NAME_OR_ID:
                    if(conflict.id() == plugin->getId() || conflict.name() == plugin->getName())
                        has_conflict = true;
                    break;
                default:
                    break;
            }
        }

        if(has_conflict)
        {
            plugin->addRuntimeConflict(pl);
            continue; // Don't need to continue checking because we've already got a conflict
        }

        // Next check any runtime conflicts that PluginManager registered

        for(const PluginRole& role: pl->roles().keys())
        {
            for(const PluginRole& own_role: plugin->roles().keys())
            {
                const PluginRoleData& role_data = m_plugin_roles.value(role);
                if(role_data.m_conflicts_with.contains(own_role))
                {
                    conflicts.append(role_data);
                }
            }
        }

        if(!conflicts.isEmpty())
            conflict_list.insert(pl, conflicts);
    }

    if(conflict_list.isEmpty()) return false;

    WARN() << "Plugin" << plugin->getName() << "conflicts with the following plugins:";
    for(Plugin* pl: conflict_list.keys())
    {
        plugin->addRuntimeConflict(pl);

        QList<PluginRoleData> roles = conflict_list.value(pl);
        QStringList names;
        for(PluginRoleData role: roles)
        {
            names.append(role.m_name);
        }
        WARN() << qPrintable(QString("%1 ").arg(QString(10, '*'))) << pl->getName();
        WARN() << qPrintable(QString("%1 ").arg(QString(16, ' '))) << "roles: " << names.join(", ");
    }
    return true;
}

PluginErrorCodes PluginManagerImpl::initPlugins()
{
    DEBUG() << "initPlugins()";

    if(plugins.size() == 0)
        return PLUGIN_ERROR_NOT_INITIALIZED;

    for(Plugin* plugin: plugins)
    {
        for(PluginDependency dep: plugin->dependencies())
        {
            for(Plugin* pl: plugins)
            {
                if(pl == plugin) continue;

                if(pl->has_role(dep.getRole()))
                {
                    connect(pl, &Plugin::initialized, [=]() {
                        initializePlugin(plugin, true);
                    });
                    connect(pl, &Plugin::deinitialized, [=]() {
                        deinitializePlugin(plugin);
                    });
                    connect(pl, &Plugin::error_happened, [=, &dep]() {
                        if(dep.isRequired())
                            deinitializePlugin(plugin);
                        else if(dep.doSkipIfFailed())
                            initializePlugin(plugin);
                    });
                }
            }
        }
    }

    for(Plugin* plugin: plugins)
    {
        if(plugin->isActive() && plugin->getState() == PLUGIN_STATE_NOT_INITIALIZED)
        {
            initializePlugin(plugin, false);
        }
    }

    // Draw a table
    DEBUG() << "Initialization finished";
    LOG() << qPrintable(QString(66, '-'));
    LOG() << qPrintable(QString("|%1|%2|%3|%4|")
                          .arg(QString("PLUGIN").leftJustified(30))
                          .arg(QString("VERSION").leftJustified(7))
                          .arg(QString("REVISION").leftJustified(8))
                          .arg(QString("STATUS").leftJustified(16)));
    LOG() << qPrintable(QString(66, '-')) ;

    for(Plugin* plugin: plugins)
    {
        LOG() << qPrintable(QString("|%1|%2|%3|%4|")
                              .arg(plugin->getName().leftJustified(30))
                              .arg(plugin->getVersion().toUpper().leftJustified(7))
                              .arg(plugin->getRevision().toUpper().leftJustified(8))
                              .arg(plugin->getStateDescription().toUpper().leftJustified(16))
                              );
    }
    LOG() << qPrintable(QString(66, '-')) ;
    return PLUGIN_ERROR_NO_ERROR;
}

PluginErrorCodes PluginManagerImpl::deinitPlugins()
{
    DEBUG() << "deinitPlugins()";
    for(Plugin* plugin: plugins)
    {
        if(plugin->isActive() && plugin->getState() == PLUGIN_STATE_INITIALIZED)
            deinitializePlugin(plugin);
    }
    return PLUGIN_ERROR_NO_ERROR;
}

void PluginManagerImpl::setPluginDir(const QString &pluginDir)
{
    this->pluginDir = pluginDir;
}

QString PluginManagerImpl::getPluginDir()
{
    return this->pluginDir;
}


void yasem::PluginManagerImpl::registerPluginRole(const PluginRole &role, const PluginRoleData &data)
{
    m_plugin_roles.insert(role, data);
}


void PluginManagerImpl::onPluginLoaded()
{
    Plugin* plugin = qobject_cast<Plugin*>(sender());
    STUB() << plugin->getName();
}

void PluginManagerImpl::onPluginUnloaded()
{
    Plugin* plugin = qobject_cast<Plugin*>(sender());
    STUB() << plugin->getName();
}

void PluginManagerImpl::onPluginInitialized()
{
    Plugin* plugin = qobject_cast<Plugin*>(sender());
    STUB() << plugin->getName();
    for(Plugin* plugin: plugins)
    {
        initializePlugin(plugin);
    }
}

void PluginManagerImpl::onPluginDeinitialized()
{
    Plugin* plugin = qobject_cast<Plugin*>(sender());
    STUB() << plugin->getName();
}

PluginErrorCodes PluginManagerImpl::initializePlugin(Plugin *plugin, bool go_deeper, bool ignore_dependencies)
{
    switch(plugin->getState())
    {
        case PLUGIN_STATE_INITIALIZED:  return PLUGIN_ERROR_NO_ERROR;
        case PLUGIN_STATE_CONFLICT:     return PLUGIN_ERROR_CONFLICT;
        case PLUGIN_STATE_DISABLED:     return PLUGIN_ERROR_PLUGIN_DISABLED;
        case PLUGIN_STATE_ERROR_STATE:  return PLUGIN_ERROR_UNKNOWN_ERROR;
        default: { }
    }


    ConfigItem* plugin_info = m_plugins_config->findItemByKey(plugin->getId());
    if(!plugin_info->value().toBool())
    {
        DEBUG() << "Plugin" << plugin->getName() << "is disabled in config. Skipping.";
        plugin->setState(PLUGIN_STATE_DISABLED);
        return PLUGIN_ERROR_PLUGIN_DISABLED;
    }

    if(pluginHasConflicts(plugin))
    {
        plugin->setState(PLUGIN_STATE_CONFLICT);
        return PLUGIN_ERROR_CONFLICT;
    }

    QList<PluginDependency> unresolved_dependencies = getUnresolvedDependencies(plugin);
    if(unresolved_dependencies.isEmpty() || ignore_dependencies)
    {
        LOG() << "Initializing plugin" << plugin->getName();
        PluginErrorCodes result = plugin->initialize();
        if(result == PLUGIN_ERROR_NO_ERROR)
            plugin->setState(PLUGIN_STATE_INITIALIZED);
        else
            plugin->setState(PLUGIN_STATE_ERROR_STATE);
        return result;
    }
    else if(go_deeper)
    {
        for(Plugin* pl: plugins)
        {
            if(pl == plugin) continue;
            for(PluginDependency dep: unresolved_dependencies)
            {
                if(pl->has_role(dep.getRole()))
                {
                    for(PluginDependency dep2: pl->dependencies())
                    {
                        if(plugin->has_role(dep2.getRole()))
                        {
                            QString msg("Circular dependencies %1 <-> %2");
                            WARN() << qPrintable(msg.arg(plugin->getName()) .arg(pl->getName()));
                            //return initializePlugin(plugin, false, true);
                        }
                    }
                }
            }
        }
    }
    //else
    //{
        plugin->setState(PLUGIN_STATE_WAITING_FOR_DEPENDENCY);
        //LOG() << "....Waiting for dependency(s) for " << plugin->getName() << getDependencyNames(unresolved_dependencies);
        return PLUGIN_ERROR_DEPENDENCY_MISSING;
    //}
}

PluginErrorCodes PluginManagerImpl::deinitializePlugin(Plugin *plugin)
{
    if(plugin->getState() == PLUGIN_STATE_NOT_INITIALIZED) return PLUGIN_ERROR_NO_ERROR;

    LOG() << "Deinitialization of" << plugin->getName();
    PluginErrorCodes result = plugin->deinitialize();
    if(result == PLUGIN_ERROR_NO_ERROR)
        plugin->setState(PLUGIN_STATE_NOT_INITIALIZED);
    else
        plugin->setState(PLUGIN_STATE_ERROR_STATE);
    return result;
}

QList<PluginDependency> PluginManagerImpl::getUnresolvedDependencies(Plugin *plugin)
{
    QList<PluginDependency> result;
    for(PluginDependency dep: plugin->dependencies())
    {
        if(getByRole(dep.getRole(), false)) continue;
        if(!dep.isRequired()) continue;
        result.append(dep);
    }
    return result;
}

QStringList PluginManagerImpl::getDependencyNames(QList<PluginDependency> list)
{
    QStringList result;
    for(PluginDependency item: list)
        result.append(item.roleName());
    return result;
}

