#include "pluginmanagerimpl.h"
#include "core.h"
#include "plugin.h"
#include "stbpluginobject.h"
#include "profilemanager.h"
#include "pluginthread.h"
#include "yasemsettings.h"
#include "guipluginobject.h"
#include "configuration_items.h"

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
    m_plugins_config(SDK::__get_config_item<SDK::ConfigContainer*>(SETTINGS_GROUP_PLUGINS))
{
   this->setObjectName("PluginManager");
#ifdef USE_OSX_BUNDLE
   setPluginDir("Plugins");
#else
    setPluginDir("plugins");
#endif //USE_OSX_BUNDLE

   registerPluginRole(SDK::ROLE_ANY,          SDK::PluginRoleData(tr("Any role"),
                                                                  tr("Any role")));
   registerPluginRole(SDK::ROLE_STB_API,      SDK::PluginRoleData(tr("STB API"),
                                                                  tr("STB API implementation")));
   registerPluginRole(SDK::ROLE_UNSPECIFIED,  SDK::PluginRoleData(tr("Unspecified"),
                                                                  tr("Unspecified role")));
   registerPluginRole(SDK::ROLE_UNKNOWN,      SDK::PluginRoleData(tr("Unknown role"),
                                                                  tr("Role is unknown. Plugin may not be compatible with core module!")));

   registerPluginRole(SDK::ROLE_BROWSER,      SDK::PluginRoleData(tr("Browser"),
                                                                  tr("Web browser component"),
                                                                  QList<SDK::PluginRole>() << SDK::ROLE_BROWSER));
   registerPluginRole(SDK::ROLE_MEDIA,        SDK::PluginRoleData(tr("Media player"),
                                                                  tr("Media player component"),
                                                                  QList<SDK::PluginRole>() << SDK::ROLE_MEDIA));
   registerPluginRole(SDK::ROLE_DATASOURCE,   SDK::PluginRoleData(tr("Datasource"),
                                                                  tr("Datasource component"),
                                                                  QList<SDK::PluginRole>() << SDK::ROLE_DATASOURCE));
   registerPluginRole(SDK::ROLE_GUI,          SDK::PluginRoleData(tr("GUI"),
                                                                  tr("Graphical user interface"),
                                                                  QList<SDK::PluginRole>() << SDK::ROLE_GUI));
   registerPluginRole(SDK::ROLE_WEB_GUI,      SDK::PluginRoleData(tr("Web GUI"),
                                                                  tr("Web-based GUI for main screen"),
                                                                  QList<SDK::PluginRole>() << SDK::ROLE_WEB_GUI));
   registerPluginRole(SDK::ROLE_WEB_SERVER,  SDK:: PluginRoleData(tr("Web server"),
                                                                  tr("Web server for fake web service emulation"),
                                                                  QList<SDK::PluginRole>() << SDK::ROLE_WEB_SERVER));
}

SDK::PluginErrorCodes PluginManagerImpl::listPlugins()
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
        return SDK::PLUGIN_ERROR_DIR_NOT_FOUND;
    }

    DEBUG() << "Searching for plugins in" << pluginsDir.path();

    foreach (QString fileName, pluginsDir.entryList(QDir::Files | QDir::NoSymLinks | QDir::Readable))
    {

        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));

        SDK::Plugin *plugin = qobject_cast<SDK::Plugin*>(pluginLoader.instance());

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
                plugin->setName(name);
                plugin->setState(SDK::PLUGIN_STATE_NOT_INITIALIZED);
                plugin->setFlags(parseFlags(metadata.value("flags").toString()));

                DEBUG() << "....Registering plugin roles...";
                plugin->register_roles();
                DEBUG() << "....Registering plugin dependencies...";
                plugin->register_dependencies();

                pluginIds.append(id);
                plugins.append(plugin);

                DEBUG() << "....Plugin loaded:" << plugin->getName();

                connect(plugin, &SDK::Plugin::loaded, this, &PluginManagerImpl::onPluginLoaded);
                connect(plugin, &SDK::Plugin::unloaded, this, &PluginManagerImpl::onPluginUnloaded);
                connect(plugin, &SDK::Plugin::initialized, this, &PluginManagerImpl::onPluginInitialized);
                connect(plugin, &SDK::Plugin::deinitialized, this, &PluginManagerImpl::onPluginDeinitialized);

                SDK::ConfigItem* plugin_info = new SDK::ConfigItem(plugin->getId(), plugin->getName(), true, SDK::ConfigItem::BOOL);
                m_plugins_config->addItem(plugin_info);
            }

        }
        else
        {
           WARN() << pluginLoader.errorString();
        }
    }
    SDK::Core::instance()->yasem_settings()->load(m_plugins_config);
    return SDK::PLUGIN_ERROR_NO_ERROR;
}


QList<SDK::Plugin*> PluginManagerImpl::getPlugins(SDK::PluginRole role, bool active_only)
{
    DEBUG() << QString("getPlugins(role: %1, active only: %2)").arg(role).arg(active_only);

    QList<SDK::Plugin*> result;
    foreach(SDK::Plugin* plugin, plugins)
    {
        bool add_this = false;
        foreach(SDK::PluginRole pluginRole, plugin->roles().keys())
        {
            if(pluginRole == role || role == SDK::ROLE_ANY)
            {
                add_this = !active_only || plugin->isActive();
            }
        }

        if(add_this) result.append(plugin);
    }

    return result;
}

SDK::AbstractPluginObject* PluginManagerImpl::getByRole(SDK::PluginRole role, bool show_warning)
{
    for(int index = 0; index < plugins.size(); index++)
    {
        SDK::Plugin *plugin = plugins.at(index);
        if(!plugin->isActive()) continue;
        for(SDK::PluginRole pluginRole: plugin->roles().keys())
        {
            if(pluginRole == role)
            {
                Q_ASSERT(plugin);
                SDK::AbstractPluginObject* obj = plugin->roles().value(pluginRole);
                if(obj->isInitialized())
                    return obj;
            }
        }
    }
    if(show_warning)
        ERROR() << qPrintable(QString("Plugin for role %1 not found!").arg(role));
    return NULL;
}

QList<SDK::AbstractPluginObject *> PluginManagerImpl::getAllByRole(SDK::PluginRole role, bool active_only)
{
    QList<SDK::AbstractPluginObject *> result;
    for(int index = 0; index < plugins.size(); index++)
    {
        SDK::Plugin *plugin = plugins.at(index);
        if(active_only && !plugin->isActive()) continue;

        for(SDK::PluginRole pluginRole: plugin->roles().keys())
        {
            if(pluginRole == role)
            {
                SDK::AbstractPluginObject* obj = plugin->roles().value(pluginRole);
                if(!active_only || obj->isInitialized())
                    result.append(obj);
            }
        }
    }
    return result;
}

SDK::Plugin *PluginManagerImpl::getByIID(const QString &iid)
{

    for(int index = 0; index < plugins.size(); index++)
    {
        SDK::Plugin *plugin = plugins.at(index);
        if(!plugin->isActive()) continue;
        if(plugin->getIID() == iid)
        {
            return plugin;
        }
    }
    return NULL;
}

SDK::PluginFlag PluginManagerImpl::parseFlags(const QString &flagsStr)
{
    QStringList flags = flagsStr.split("|");
    SDK::PluginFlag result = SDK::PLUGIN_FLAG_NONE;
    foreach(QString flag, flags)
    {
        if(flag == PLUGIN_FLAG_NAME_CLIENT)
            result = (SDK::PluginFlag)(result | SDK::PLUGIN_FLAG_CLIENT);
        else if(flag == PLUGIN_FLAG_NAME_SYSTEM)
            result = (SDK::PluginFlag)(result | SDK::PLUGIN_FLAG_SYSTEM);
        else if(flag == PLUGIN_FLAG_NAME_HIDDEN)
            result = (SDK::PluginFlag)(result | SDK::PLUGIN_FLAG_HIDDEN);
        else if(flag == PLUGIN_FLAG_NAME_GUI)
            result = (SDK::PluginFlag)(result | SDK::PLUGIN_FLAG_GUI);
    }
    return result;
}

bool PluginManagerImpl::pluginHasConflicts(SDK::Plugin *plugin)
{
    QHash<SDK::Plugin*, QList<SDK::PluginRoleData>> conflict_list;
    for(SDK::Plugin* pl: plugins)
    {
        QList<SDK::PluginRoleData> conflicts;

        // Skip itself and plugins that hasn't been loaded yet
        if(pl == plugin || pl->getState() != SDK::PLUGIN_STATE_INITIALIZED) continue;

        // First check static conflicts that has been registered by plugin
        // @see: registerPluginRole()

        bool has_conflict = false;
        for(SDK::PluginConflict conflict: pl->getStaticConflicts())
        {
            switch (conflict.type()) {
                case SDK::PLUGIN_CONFLICTS_BY_ID:
                    if(conflict.id() == plugin->getId())
                        has_conflict = true;
                    break;
                case SDK::PLUGIN_CONFLICTS_BY_NAME:
                    if(conflict.name() == plugin->getName())
                        has_conflict = true;
                    break;
                case SDK::PLUGIN_CONFLICTS_BY_NAME_OR_ID:
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

        for(const SDK::PluginRole& role: pl->roles().keys())
        {
            for(const SDK::PluginRole& own_role: plugin->roles().keys())
            {
                const SDK::PluginRoleData& role_data = m_plugin_roles.value(role);
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
    for(SDK::Plugin* pl: conflict_list.keys())
    {
        plugin->addRuntimeConflict(pl);

        QList<SDK::PluginRoleData> roles = conflict_list.value(pl);
        QStringList names;
        for(SDK::PluginRoleData role: roles)
        {
            names.append(role.m_name);
        }
        WARN() << qPrintable(QString("%1 ").arg(QString(10, '*'))) << pl->getName();
        WARN() << qPrintable(QString("%1 ").arg(QString(16, ' '))) << "roles: " << names.join(", ");
    }
    return true;
}

SDK::PluginErrorCodes PluginManagerImpl::initPlugins()
{
    DEBUG() << "initPlugins()";

    if(plugins.size() == 0)
        return SDK::PLUGIN_ERROR_NOT_INITIALIZED;

    for(SDK::Plugin* plugin: plugins)
    {
        for(SDK::PluginDependency dep: plugin->dependencies())
        {
            for(SDK::Plugin* pl: plugins)
            {
                if(pl == plugin) continue;

                if(pl->has_role(dep.getRole()))
                {
                    connect(pl, &SDK::Plugin::initialized, [=]() {
                        initializePlugin(plugin, true);
                    });
                    connect(pl, &SDK::Plugin::deinitialized, [=]() {
                        deinitializePlugin(plugin);
                    });
                    connect(pl, &SDK::Plugin::error_happened, [=, &dep]() {
                        if(dep.isRequired())
                            deinitializePlugin(plugin);
                        else if(dep.doSkipIfFailed())
                            initializePlugin(plugin);
                    });
                }
            }
        }
    }

    for(SDK::Plugin* plugin: plugins)
    {
        if(plugin->isActive() && plugin->getState() == SDK::PLUGIN_STATE_NOT_INITIALIZED)
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

    for(SDK::Plugin* plugin: plugins)
    {
        LOG() << qPrintable(QString("|%1|%2|%3|%4|")
                              .arg(plugin->getName().leftJustified(30))
                              .arg(plugin->getVersion().toUpper().leftJustified(7))
                              .arg(plugin->getRevision().toUpper().leftJustified(8))
                              .arg(plugin->getStateDescription().toUpper().leftJustified(16))
                              );
    }
    LOG() << qPrintable(QString(66, '-')) ;
    return SDK::PLUGIN_ERROR_NO_ERROR;
}

SDK::PluginErrorCodes PluginManagerImpl::deinitPlugins()
{
    DEBUG() << "deinitPlugins()";
    for(SDK::Plugin* plugin: plugins)
    {
        if(plugin->isActive() && plugin->getState() == SDK::PLUGIN_STATE_INITIALIZED)
            deinitializePlugin(plugin);
    }
    return SDK::PLUGIN_ERROR_NO_ERROR;
}

void PluginManagerImpl::setPluginDir(const QString &pluginDir)
{
    this->pluginDir = pluginDir;
}

QString PluginManagerImpl::getPluginDir()
{
    return this->pluginDir;
}


void PluginManagerImpl::registerPluginRole(const SDK::PluginRole &role, const SDK::PluginRoleData &data)
{
    m_plugin_roles.insert(role, data);
}


void PluginManagerImpl::onPluginLoaded()
{
    SDK::Plugin* plugin = qobject_cast<SDK::Plugin*>(sender());
    STUB() << plugin->getName();
}

void PluginManagerImpl::onPluginUnloaded()
{
    SDK::Plugin* plugin = qobject_cast<SDK::Plugin*>(sender());
    STUB() << plugin->getName();
}

void PluginManagerImpl::onPluginInitialized()
{
    SDK::Plugin* plugin = qobject_cast<SDK::Plugin*>(sender());
    STUB() << plugin->getName();
    for(SDK::Plugin* plugin: plugins)
    {
        initializePlugin(plugin);
    }
}

void PluginManagerImpl::onPluginDeinitialized()
{
    SDK::Plugin* plugin = qobject_cast<SDK::Plugin*>(sender());
    STUB() << plugin->getName();
}

//#define MULTITHREADED_PLUGINS

SDK::PluginErrorCodes PluginManagerImpl::initializePlugin(SDK::Plugin *plugin, bool go_deeper, bool ignore_dependencies)
{
    switch(plugin->getState())
    {
        case SDK::PLUGIN_STATE_INITIALIZED:      return SDK::PLUGIN_ERROR_NO_ERROR;
        case SDK::PLUGIN_STATE_CONFLICT:         return SDK::PLUGIN_ERROR_CONFLICT;
        case SDK::PLUGIN_STATE_DISABLED:         return SDK::PLUGIN_ERROR_PLUGIN_DISABLED;
        case SDK::PLUGIN_STATE_ERROR_STATE:      return SDK::PLUGIN_ERROR_UNKNOWN_ERROR;
        case SDK::PLUGIN_STATE_THREAD_CREATING:  return SDK::PLUGIN_ERROR_NO_ERROR;
        case SDK::PLUGIN_STATE_THREAD_STARTED:   return SDK::PLUGIN_ERROR_NO_ERROR;
        default: {
            plugin->setState(SDK::PLUGIN_STATE_THREAD_CREATING);
        }
    }

    SDK::ConfigItem* plugin_info = m_plugins_config->findItemByKey(plugin->getId());
    if(!plugin_info->value().toBool())
    {
        DEBUG() << "Plugin" << plugin->getName() << "is disabled in config. Skipping.";
        plugin->setState(SDK::PLUGIN_STATE_DISABLED);
        return SDK::PLUGIN_ERROR_PLUGIN_DISABLED;
    }

    if(pluginHasConflicts(plugin))
    {
        plugin->setState(SDK::PLUGIN_STATE_CONFLICT);
        return SDK::PLUGIN_ERROR_CONFLICT;
    }

    QList<SDK::PluginDependency> unresolved_dependencies = getUnresolvedDependencies(plugin);
    if(unresolved_dependencies.isEmpty() || ignore_dependencies)
    {
        LOG() << "Initializing plugin" << plugin->getName();

#ifdef MULTITHREADED_PLUGINS
        if(plugin->isMultithreadingEnabled() && plugin->thread() == QThread::currentThread())
        {
            PluginThread* thread = new PluginThread(plugin, QThread::currentThread());
            plugin->moveToThread(thread);
            thread->start();
            DEBUG() << plugin->thread()->objectName();
        }
        else
        {
            PluginErrorCodes result = plugin->initialize();

            if(result == PLUGIN_ERROR_NO_ERROR)
                plugin->setState(PLUGIN_STATE_INITIALIZED);
            else
                plugin->setState(PLUGIN_STATE_ERROR_STATE);
        }
        return PLUGIN_ERROR_NO_ERROR;
#else
        SDK::PluginErrorCodes result = plugin->initialize();

        if(result == SDK::PLUGIN_ERROR_NO_ERROR)
            plugin->setState(SDK::PLUGIN_STATE_INITIALIZED);
        else
            plugin->setState(SDK::PLUGIN_STATE_ERROR_STATE);
        return result;
#endif //MULTITHREADED_PLUGINS
    }
    else if(go_deeper)
    {
        for(SDK::Plugin* pl: plugins)
        {
            if(pl == plugin) continue;
            for(SDK::PluginDependency dep: unresolved_dependencies)
            {
                if(pl->has_role(dep.getRole()))
                {
                    for(SDK::PluginDependency dep2: pl->dependencies())
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
        plugin->setState(SDK::PLUGIN_STATE_WAITING_FOR_DEPENDENCY);
        //LOG() << "....Waiting for dependency(s) for " << plugin->getName() << getDependencyNames(unresolved_dependencies);
        return SDK::PLUGIN_ERROR_DEPENDENCY_MISSING;
    //}
}

SDK::PluginErrorCodes PluginManagerImpl::deinitializePlugin(SDK::Plugin *plugin)
{
    if(plugin->getState() == SDK::PLUGIN_STATE_NOT_INITIALIZED) return SDK::PLUGIN_ERROR_NO_ERROR;

    LOG() << "Deinitialization of" << plugin->getName();
    SDK::PluginErrorCodes result = plugin->deinitialize();
    if(result == SDK::PLUGIN_ERROR_NO_ERROR)
        plugin->setState(SDK::PLUGIN_STATE_NOT_INITIALIZED);
    else
        plugin->setState(SDK::PLUGIN_STATE_ERROR_STATE);
    return result;
}

QList<SDK::PluginDependency> PluginManagerImpl::getUnresolvedDependencies(SDK::Plugin *plugin)
{
    QList<SDK::PluginDependency> result;
    for(SDK::PluginDependency dep: plugin->dependencies())
    {
        if(getByRole(dep.getRole(), false)) continue;
        if(!dep.isRequired()) continue;
        result.append(dep);
    }
    return result;
}

QStringList PluginManagerImpl::getDependencyNames(QList<SDK::PluginDependency> list)
{
    QStringList result;
    for(SDK::PluginDependency item: list)
        result.append(item.roleName());
    return result;
}

