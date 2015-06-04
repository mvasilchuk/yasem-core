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
   #ifdef Q_OS_DARWIN
   setPluginDir("Plugins");
#else
    setPluginDir("plugins");
#endif //Q_OS_DARWIN
   blacklistedPlugins.append("vlc-mediaplayer");
   //blacklistedPlugins.append("qt-mediaplayer");
   //blacklistedPlugins.append("dunehd-plugin");
   //blacklistedPlugins.append("mag-api");
   //blacklistedPlugins.append("samsung-smart-tv-plugin");
   //blacklistedPlugins.append("libav-mediaplayer");

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
            pluginsDir.cdUp();
    #endif
    if(!pluginsDir.cd(getPluginDir()))
    {
        ERROR() << "Cannot go to plugins dir:" << getPluginDir();
        return PLUGIN_ERROR_DIR_NOT_FOUND;
    }

    qDebug() << "Blacklisted plugins:" << blacklistedPlugins;

    DEBUG() << "Searching for plugins in" << pluginsDir.path();

    foreach (QString fileName, pluginsDir.entryList(QDir::Files | QDir::NoSymLinks | QDir::Readable))
    {
        qDebug() << "Found file:" << fileName;
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));

        Plugin *plugin = qobject_cast<Plugin*>(pluginLoader.instance());

        if (plugin != NULL)
        {
            QJsonObject metadata = pluginLoader.metaData().value("MetaData").toObject();
            QString id = metadata.value("id").toString();

            if(!pluginIds.contains(id))
            {
                if(blacklistedPlugins.contains(id))
                {
                    DEBUG() << "Plugin" << id << "is ignored and won't be activated.";
                    plugin->setActive(false);
                }
                else
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

                DEBUG() << "Registering plugin roles...";
                plugin->register_roles();
                DEBUG() << "Registering plugin dependencies...";
                plugin->register_dependencies();

                pluginIds.append(id);
                plugins.append(plugin);

                DEBUG() << "Plugin loaded:" << plugin->getName();

                ConfigItem* plugin_info = new ConfigItem(plugin->getId(), plugin->getName(), true, ConfigItem::BOOL);
                m_plugins_config->addItem(plugin_info);
            }

        }
        else
        {
           WARN() << pluginLoader.errorString();
        }

        Core::instance()->yasem_settings()->load(m_plugins_config);

    }
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

PluginErrorCodes PluginManagerImpl::initPlugin(Plugin *plugin)
{
    DEBUG() << qPrintable("Initialization of") << plugin->getName();
    Q_ASSERT(plugin != NULL);
    ConfigItem* plugin_info = m_plugins_config->findItemByKey(plugin->getId());
    if(!plugin_info->value().toBool())
    {
        DEBUG() << "Plugin" << plugin->getName() << "is disabled in config. Skipping.";
        plugin->setState(PLUGIN_STATE_DISABLED);
        return PLUGIN_ERROR_PLUGIN_DISABLED;
    }

    if(plugin->getState() == PLUGIN_STATE_INITIALIZED)
    {
        //INFO(QString("Plugin '%1' is already initialized").arg(plugin->name));
        return PLUGIN_ERROR_NO_ERROR;
    }

    Q_ASSERT(plugin->getId() != NULL);
    Q_ASSERT(plugin->getId().length() > 0);
    if(plugin->getId() == NULL || plugin->getId() == "")
        return PLUGIN_ERROR_NO_PLUGIN_ID;

    //DEBUG() << "Plugin" << plugin->getId() << "initialization...";

    if(pluginHasConflicts(plugin))
    {
        plugin->setState(PLUGIN_STATE_CONFLICT);
        return PLUGIN_ERROR_CONFLICT;
    }


    //DEBUG() << "Loading dependencies for" << plugin->getId() << "...";

    if(plugin->dependencies().size() > 0)
    {
        plugin->setState(PLUGIN_STATE_WAITING_FOR_DEPENDENCY);
        PluginErrorCodes result = PLUGIN_ERROR_NO_ERROR;

        for (const PluginDependency &dependency: plugin->dependencies())
        {
            if(isCircularDependency(plugin, dependency))
            {
                WARN() << "Circular dependency in" << plugin->getId() << "with role" << dependency.roleName();
                continue;
            }

            PluginErrorCodes dependencyResult = loadDependency(plugin, dependency);

            if(dependencyResult != PLUGIN_ERROR_NO_ERROR)
            {
                WARN() << "Error" << dependencyResult << "occured while initializing plugin" << plugin->getName() << "dependency" << dependency.roleName();
                plugin->setState(PLUGIN_STATE_DISABLED_BY_DEPENDENCY);
                result = PLUGIN_ERROR_DEPENDENCY_MISSING;
            }
        }


        switch(result)
        {
            case PLUGIN_ERROR_NO_ERROR:
            {
                //DEBUG() << "Dependencies for" << plugin->getId() << "have been loaded.";
                break;
            }
            case PLUGIN_ERROR_DEPENDENCY_MISSING:
            {
                WARN() << "Plugin" << plugin->getId() << "won't be loaded because of missing dependencies!";
                return PLUGIN_ERROR_DEPENDENCY_MISSING;
            }
            default:
            {
                WARN() << "Plugin" << plugin->getId() << "unknown initialization code" << result;
                return PLUGIN_ERROR_UNKNOWN_ERROR;
            }
        }            
    }
    else
        DEBUG() << "No dependencies for" << plugin->getId() << "have been found";

    #ifdef THREAD_SAFE_PLUGINS
    PluginErrorCodes initValue = PLUGIN_ERROR_NOT_INITIALIZED;
    if(plugin->has_role(ROLE_GUI) || plugin->has_role(ROLE_MEDIA))
    {
        qDebug() << "HAS GUI FLAG";
        initValue = plugin->initialize();
//
    }
    else
    {
        qRegisterMetaType<PluginErrorCodes>("PluginErrorCodes");
        PluginThread* thread = new PluginThread(plugin);
        plugin->moveToThread(thread);
        thread->start();
        DEBUG() << "invoke ok" << QMetaObject::invokeMethod(plugin, "initialize", Qt::BlockingQueuedConnection, Q_RETURN_ARG(PluginErrorCodes, initValue));
    }
    #else
    int initValue = plugin->initialize();
    #endif // THREAD_SAFE_PLUGINS

    if(initValue != PLUGIN_ERROR_NO_ERROR)
    {
        WARN() <<"Plugin initialization failed. Code:" << initValue;
        plugin->setState(PLUGIN_STATE_ERROR_STATE);
        return PLUGIN_ERROR_NOT_INITIALIZED;
    }

    DEBUG() << "Plugin" << plugin->getId() << "initialized";

    plugin->setState(PLUGIN_STATE_INITIALIZED);

    return PLUGIN_ERROR_NO_ERROR;
}

PluginErrorCodes  PluginManagerImpl::deinitPlugin(Plugin *plugin)
{
    DEBUG() << "deinitPlugin(" << plugin->getName() << ")";

    PluginErrorCodes result = plugin->deinitialize();
    if(result == PLUGIN_ERROR_NO_ERROR)
        plugin->setState(PLUGIN_STATE_ERROR_STATE);


    /*
        while (!plugins.isEmpty()) {
           QPluginLoader* pluginLoader = plugins.takeFirst();
           pluginLoader->unload();
           delete pluginLoader;
        }
     */


    return result;
}

AbstractPluginObject* PluginManagerImpl::getByRole(PluginRole role)
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
        if(plugin->isActive() && plugin->getState() == PLUGIN_STATE_NOT_INITIALIZED)
            initPlugin(plugin);
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
            deinitPlugin(plugin);
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

bool PluginManagerImpl::isCircularDependency(Plugin *plugin, const PluginDependency &dependency) const
{
    bool circular = false;
    for(PluginRole role: plugin->roles().keys())
    {
        if(role == dependency.getRole())
        {
            circular = true;
            break;
        }
    }

    return circular;
}

PluginErrorCodes PluginManagerImpl::loadDependency(Plugin *plugin, const PluginDependency &dependency)
{
    PluginErrorCodes result = PLUGIN_ERROR_DEPENDENCY_MISSING;
    //DEBUG() << qPrintable(QString("Trying to load dependency %1 for %2").arg(dependency.roleName()).arg(plugin->getName()));

    DEBUG() << qPrintable(QString("    DEPENDENCY: %1").arg(dependency.roleName()));

    QList<AbstractPluginObject*> dependencyList = getAllByRole(dependency.getRole(), false);

    if(dependencyList.isEmpty())
    {
        QString resString = QString("Required plugin with role %1 for %2 not found!").arg(dependency.roleName()).arg(plugin->getId());

        if(dependency.isRequired())
        {
            if(dependency.doSkipIfFailed())
            {
                WARN() << qPrintable(resString) << qPrintable("Failed to load. Skipped");
                return PLUGIN_ERROR_NO_ERROR;
            }
            WARN() << qPrintable(resString) << resString;
            return PLUGIN_ERROR_DEPENDENCY_MISSING;
        }

        LOG() << qPrintable(resString) << qPrintable("Not required. Skipping...");
        return PLUGIN_ERROR_NO_ERROR;
    }

    for(AbstractPluginObject* dependencyObject: dependencyList)
    {
        Plugin* depPlugin = dependencyObject->plugin();
        DEBUG() << qPrintable(QString("        %1")
                              .arg(depPlugin->getId())
                              .leftJustified(32, '.'))
                << qPrintable(depPlugin->getStateDescription().toUpper());

        PluginErrorCodes code;
        switch(dependencyObject->plugin()->getState())
        {
            case PLUGIN_STATE_WAITING_FOR_DEPENDENCY: {
                WARN() << "Circular dependency in plugin" << plugin->getId() << "with" << dependencyObject->plugin()->getId();
                code = PLUGIN_ERROR_NO_ERROR;
                break;
            }
            case PLUGIN_STATE_CONFLICT: {
                WARN() << "Plugin" << plugin->getId() << "conflicts with" << dependencyObject->plugin()->getId();
                code = PLUGIN_ERROR_CONFLICT;
                break;
            }
            case PLUGIN_STATE_DISABLED:
            case PLUGIN_STATE_DISABLED_BY_DEPENDENCY: {
                WARN() << "Plugin" << plugin->getId() << "disabled";
                code = PLUGIN_ERROR_PLUGIN_DISABLED;

                break;
            }
            default: {
                code = initPlugin(dependencyObject->plugin());
            }
        }

        if(code != PLUGIN_ERROR_NO_ERROR && dependency.doSkipIfFailed())
        {
            code = PLUGIN_ERROR_NO_ERROR;
        }


        if(code == PLUGIN_ERROR_NO_ERROR) ///< At least one of dependency plugins should be loaded
            result = PLUGIN_ERROR_NO_ERROR;
    }
    return result;
}
