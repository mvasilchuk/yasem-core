#include "pluginmanagerimpl.h"
#include "core.h"
#include "stbplugin.h"
#include "profilemanager.h"
#include "pluginthread.h"

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

PluginManagerImpl::PluginManagerImpl()
{
   this->setObjectName("PluginManager");
   setPluginDir("plugins");
   blacklistedPlugins.append("vlc-mediaplayer");
   blacklistedPlugins.append("qt-mediaplayer");
   //blacklistedPlugins.append("dunehd-plugin");
   //blacklistedPlugins.append("mag-api");
   //blacklistedPlugins.append("samsung-smart-tv-plugin");
   //blacklistedPlugins.append("libav-mediaplayer");
}

PLUGIN_ERROR_CODES PluginManagerImpl::listPlugins()
{
    DEBUG() << "Looking for plugins...";
    QSettings* settings = Core::instance()->settings();

    QStringList pluginsToLoad = settings->value("plugins", "").toString().split(",");

    DEBUG() << "pluginsToLoad:" << pluginsToLoad;


    DEBUG() << "PluginManager::listPlugins()";
    QStringList pluginIds;
    plugins.clear();

    QDir pluginsDir(qApp->applicationDirPath());
    #if defined(Q_OS_WIN)
        if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
            pluginsDir.cdUp();
    #elif defined(Q_OS_MAC)
        if (pluginsDir.dirName() == "MacOS") {
            pluginsDir.cdUp();
            pluginsDir.cdUp();
            pluginsDir.cdUp();
        }
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

        QObject* obj = pluginLoader.instance();
        Plugin *plugin = qobject_cast<Plugin*>(obj);
        if (plugin) {
           // PluginInterface *plugin = qobject_cast<PluginInterface*>(absPlugin);
            if (plugin != NULL)
            {
                QJsonObject metadata = pluginLoader.metaData().value("MetaData").toObject();
                QString id = metadata.value("id").toString();

                if(!pluginIds.contains(id))
                {
                    QString name = metadata.value("name").toString();
                    QString version = metadata.value("version").toString();
                    QString type = metadata.value("type").toString();

                    plugin->IID = pluginLoader.metaData().value("IID").toString();
                    plugin->className = pluginLoader.metaData().value("className").toString();
                    plugin->metadata = metadata;
                    plugin->id = id;
                    plugin->version = version;
                    plugin->name = name;
                    plugin->roles = parseRoles(metadata.value("roles").toArray());
                    plugin->dependencies = convertDependenciesList(metadata.value("dependencies").toArray());
                    plugin->state = PLUGIN_STATE_LOADED;
                    plugin->flags = parseFlags(metadata.value("flags").toString());

                    //qDebug() << "CLASS count: "
                    //            << ((QPluginLoader*)plugin)->metaObject()->className();

                    if(blacklistedPlugins.contains(plugin->id))
                    {
                        qDebug() << QString("Plugin %1 is ignored and won't be loaded.").arg(plugin->id);
                        continue;
                    }

                    pluginIds.append(id);
                    plugins.append(plugin);

                    QStringList dependenciesNames;
                    for(int index = 0; index < plugin->dependencies.length(); index++)
                    {
                        dependenciesNames.append(plugin->dependencies.at(index)->role);
                    }

                    qDebug() << "Plugin loaded:" << plugin->name << ". Dependencies:" << dependenciesNames;

                    // Trying to register plugin as profile plugin
                    StbProfilePlugin* profilePlugin = dynamic_cast<StbProfilePlugin*>(plugin);
                    if(profilePlugin != 0)
                    {
                        qDebug() << "Found STB plugin" << profilePlugin->name;
                        ProfileManager::instance()->registerProfileClassId(profilePlugin->getProfileClassId(), profilePlugin);
                    }
                }

            }

            settings->setValue("plugins", pluginIds.join(","));

        }
        else
        {
            qDebug() << pluginLoader.errorString();
        }

    }
    return PLUGIN_ERROR_NO_ERROR;
}



QList<Plugin*> PluginManagerImpl::getPlugins(const QString &role = "")
{
    DEBUG() << QString("getPlugins(%1)").arg(role);
    if(role == "")
        return plugins;

    QList<Plugin*> result;
    foreach(Plugin* plugin, plugins)
    {
        foreach(Plugin::PluginRole pluginRole, plugin->roles)
        {
            if(pluginRole.name == role)
                result.append(plugin);
        }
    }

    return result;
}

PLUGIN_ERROR_CODES PluginManagerImpl::initPlugin(Plugin *plugin, int dependencyLevel = 0)
{
    Q_ASSERT(plugin != NULL);


    QStringList depLevel;
    for(int index=0; index < dependencyLevel; index++)
        depLevel.append("--");

    QString spacing = depLevel.size() > 0 ? depLevel.join("-") : "";
    if(plugin->state == PLUGIN_STATE_INITIALIZED)
    {
        //INFO(QString("Plugin '%1' is already initialized").arg(plugin->name));
        return PLUGIN_ERROR_NO_ERROR;
    }

    Q_ASSERT(plugin->id != NULL);
    Q_ASSERT(plugin->id.length() > 0);
    if(plugin->id == NULL || plugin->id == "")
        return PLUGIN_ERROR_NO_PLUGIN_ID;

    DEBUG() << qPrintable(spacing) << "Plugin" << plugin->id << "initialization...";
    DEBUG() << qPrintable(spacing) << "Loading dependencies for" << plugin->id << "...";

    if(plugin->dependencies.size() > 0)
    {
        plugin->state = PLUGIN_STATE_WAITING_FOR_DEPENDENCY;
        PLUGIN_ERROR_CODES result = PLUGIN_ERROR_NO_ERROR;

        for (int i = 0; i < plugin->dependencies.size(); ++i)
        {
            PluginDependency* dependency = plugin->dependencies.at(i);

            //Check dependency cycle

            bool cyclic = false;
            foreach(Plugin::PluginRole role, plugin->roles)
            {
                if(role.name == dependency->role)
                {
                    cyclic = true;
                    break;
                }
            }

            if(cyclic)
            {
                WARN() << "Cyclic dependency in" << plugin->id << "with role" << dependency->role;
                continue;
            }

            DEBUG() << qPrintable(spacing) << "Trying to load dependency" << dependency->role << " for" << plugin->name;

            Plugin* dependencyPlugin = PluginManager::instance()->getByRole(dependency->role);
            if(dependencyPlugin == NULL)
            {
                WARN() << "Dependency" << dependency->role << "for" << plugin->id << "not found!";
                if(dependency->required)
                {
                    result = PLUGIN_ERROR_DEPENDENCY_MISSING;
                    break;
                }
                else
                {
                    LOG() << qPrintable(spacing) << "Skipping" << dependency->role << "as not required";
                    continue;
                }
            }

            dependency->plugin = dependencyPlugin;

            if(dependencyPlugin->state == PLUGIN_STATE_WAITING_FOR_DEPENDENCY)
            {
                WARN() << "Cyclic dependency in plugin" << plugin->id << "with" << dependencyPlugin->id;
                continue;
            }



            PLUGIN_ERROR_CODES code = PluginManager::instance()->initPlugin(dependencyPlugin, dependencyLevel+1);
            if(code != PLUGIN_ERROR_NO_ERROR)
            {
                WARN() << "Error" << code << "occured while initializing plugin" << plugin->name << "dependency" << dependency->role;
                plugin->state = PLUGIN_STATE_INITIALIZED;
                result = PLUGIN_ERROR_DEPENDENCY_MISSING;
            }
            else
            {
                DEBUG() << qPrintable(spacing) << "Dependency" << dependency->role << "initialized";
            }
        }


        switch(result)
        {
            case PLUGIN_ERROR_NO_ERROR:
            {
                DEBUG() << qPrintable(spacing) << "Dependencies for" << plugin->id << "have been loaded.";
                break;
            }
            case PLUGIN_ERROR_DEPENDENCY_MISSING:
            {
                WARN() << "Plugin" << plugin->id << "won't be loaded because of missing dependencies!";
                return PLUGIN_ERROR_DEPENDENCY_MISSING;
            }
            default:
            {
                WARN() << "Plugin" << plugin->id << "unknown initialization code" << result;
                return PLUGIN_ERROR_UNKNOWN_ERROR;
            }
        }
    }
    else
        DEBUG() << "No dependencies for" << plugin->id << "have been found";


    #ifdef THREAD_SAFE_PLUGINS
    int initValue = PLUGIN_ERROR_NO_ERROR;
    if(plugin->hasFlag(Plugin::GUI))
    {
        qDebug() << "HAS GUI FLAG";

    }
    else
    {
        PluginThread* thread = new PluginThread(plugin);
        thread->start();
    }
    #else
    int initValue = plugin->initialize();
    #endif // THREAD_SAFE_PLUGINS

    if(initValue != PLUGIN_ERROR_NO_ERROR)
    {
        WARN() <<"Plugin initialization failed. Code:" << initValue;
        throw new std::exception();
        plugin->state = PLUGIN_STATE_DISABLED;
        return PLUGIN_ERROR_NOT_INITIALIZED;
    }
    DEBUG() << qPrintable(spacing) << "Plugin" << plugin->id << "initialized";

    plugin->state = PLUGIN_STATE_INITIALIZED;
    return PLUGIN_ERROR_NO_ERROR;
}

PLUGIN_ERROR_CODES  PluginManagerImpl::deinitPlugin(Plugin *plugin)
{
    DEBUG() << "deinitPlugin(" << plugin->name << ")";

    PLUGIN_ERROR_CODES result = plugin->deinitialize();
    if(result == PLUGIN_ERROR_NO_ERROR)
        plugin->state = PLUGIN_STATE_UNLOADED;


    /*
        while (!plugins.isEmpty()) {
           QPluginLoader* pluginLoader = plugins.takeFirst();
           pluginLoader->unload();
           delete pluginLoader;
        }
     */


    return result;
}

Plugin* PluginManagerImpl::getByRole(const QString &role, Plugin::PluginFlag flags = Plugin::CLIENT)
{
    //DEBUG(QString("getByRole(%1)").arg(role));
    for(int index = 0; index < plugins.size(); index++)
    {
        Plugin *plugin = plugins.at(index);
        foreach(Plugin::PluginRole pluginRole, plugin->roles)
        {
            if(pluginRole.name == role && pluginRole.hasFlag(flags))
            {
                return plugin;
            }
        }
    }
    return NULL;
}

Plugin *PluginManagerImpl::getByIID(const QString &iid)
{

    for(int index = 0; index < plugins.size(); index++)
    {
        Plugin *plugin = plugins.at(index);
        //qDebug() << plugin->IID;
        if(plugin->IID == iid)
        {
            return plugin;
        }
    }
    return NULL;
}

QList<Plugin::PluginRole> PluginManagerImpl::parseRoles(const QJsonArray &array)
{
    QList<Plugin::PluginRole> list;
    for(int index = 0; index < array.size(); index++)
    {
        QJsonValue value = array.at(index);
        Plugin::PluginRole role;
        if(value.isString())
        {
            role.name = value.toString();
            role.flags = Plugin::CLIENT;
        }
        else if(value.isObject())
        {
            QJsonObject obj = value.toObject();
            QString name = obj.value("name").toString();


            Q_ASSERT_X(name != "", "PluginManagerImpl::parseRoles", "Plugin role name cannot be null");
            role.name = name;

            role.flags = parseFlags(obj.value("flags").toString());
        }
        list.append(role);
    }
    return list;
}

Plugin::PluginFlag PluginManagerImpl::parseFlags(const QString &flagsStr)
{
    QStringList flags = flagsStr.split("|");
    Plugin::PluginFlag result = Plugin::NONE;
    foreach(QString flag, flags)
    {
        if(flag == "client")
            result = (Plugin::PluginFlag)(result | Plugin::CLIENT);
        else if(flag == "system")
            result = (Plugin::PluginFlag)(result | Plugin::SYSTEM);
        else if(flag == "hidden")
            result = (Plugin::PluginFlag)(result | Plugin::HIDDEN);
        else if(flag == "gui")
            result = (Plugin::PluginFlag)(result | Plugin::GUI);
    }
    return result;
}

QList<yasem::PluginDependency*> PluginManagerImpl::convertDependenciesList(const QJsonArray &array)
{
    QList<PluginDependency*> list;
    for(int index = 0; index < array.size(); index++)
    {
        QJsonValue item = array.at(index);
        PluginDependency* dep = new PluginDependency();
        if(item.isObject())
        {
            QJsonObject obj = item.toObject();
            dep->role = obj.value("role").toString("???");
            dep->required = obj.value("required").toBool(true);
        }
        else if(item.isString())
        {
           dep->role = item.toString();
           dep->required = true;
        }
        list.append(dep);
    }
    return list;
}

/*
void PluginManagerImpl::loadProfiles()
{
    STUB();
    ProfileManager::instance()->loadProfiles();

    / *
    if(ProfileManager::instance()->getProfiles().size() == 0)
    {
        INFO("No profiles found. Creating the new one.");
        StbProfilePlugin* profilePlugin = ProfileManager::instance()->getProfilePluginByClassId("mag");
        Q_ASSERT(profilePlugin);
        Profile* profile = ProfileManager::instance()->createProfile("test profile 1", profilePlugin->getProfileClassId());
        Q_ASSERT(profile);
        ProfileManager::instance()->addProfile(profile);
        ProfileManager::instance()->setActiveProfile(profile);
    }
    else
    {
        QString profileId = Core::instance()->settings()->value("active_profile", "").toString();
        Q_ASSERT(profileId != "");
        Profile* profile = ProfileManager::instance()->findById(profileId);
        Q_ASSERT(profile);
        ProfileManager::instance()->setActiveProfile(profile);
    }* /

}*/

PLUGIN_ERROR_CODES PluginManagerImpl::initPlugins()
{
    DEBUG() << "initPlugins()";
    for(int index = 0; index < plugins.size(); index++)
    {
        Plugin* plugin = plugins.at(index);
        initPlugin(plugin);

    }
    return PLUGIN_ERROR_NO_ERROR;
}

PLUGIN_ERROR_CODES PluginManagerImpl::deinitPlugins()
{
    DEBUG() << "deinitPlugins()";
    for(int index = 0; index < plugins.size(); index++)
    {
        deinitPlugin(plugins.at(index));
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
