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

static const QString PLUGIN_FLAG_NAME_CLIENT = "client";
static const QString PLUGIN_FLAG_NAME_SYSTEM = "system";
static const QString PLUGIN_FLAG_NAME_HIDDEN = "hidden";
static const QString PLUGIN_FLAG_NAME_GUI    = "gui";

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

        Plugin *plugin = qobject_cast<Plugin*>(pluginLoader.instance());

        if (plugin != NULL)
        {
            QJsonObject metadata = pluginLoader.metaData().value("MetaData").toObject();
            QString id = metadata.value("id").toString();

            if(!pluginIds.contains(id))
            {
                if(blacklistedPlugins.contains(id))
                {
                    DEBUG() << "Plugin" << id << "is ignored and won't be loaded.";
                    continue;
                }

                QString name = metadata.value("name").toString();
                QString version = metadata.value("version").toString();
                QString type = metadata.value("type").toString();

                plugin->setIID(pluginLoader.metaData().value("IID").toString());
                plugin->setClassName(pluginLoader.metaData().value("className").toString());
                plugin->setMetadata(metadata);
                plugin->setId(id);
                plugin->setVersion(version);
                plugin->setName(name);
                plugin->setState(PLUGIN_STATE_LOADED);
                plugin->setFlags(parseFlags(metadata.value("flags").toString()));


                DEBUG() << "Registering plugin roles...";
                plugin->register_roles();
                DEBUG() << "Registering plugin dependencies...";
                plugin->register_dependencies();

                pluginIds.append(id);
                plugins.append(plugin);


                DEBUG() << "Plugin loaded:" << plugin->getName();

                // Trying to register plugin as profile plugin
                StbPlugin* stbPlugin = dynamic_cast<StbPlugin*>(plugin);
                if(stbPlugin != 0)
                {
                    ProfileManager::instance()->registerProfileClassId(stbPlugin->getProfileClassId(), stbPlugin);
                    DEBUG() << "Found STB plugin:" << stbPlugin->getName();
                    DEBUG() << "Found STB submodels:" << stbPlugin->listSubmodels();
                }
            }

        }
        else
        {
           WARN() << pluginLoader.errorString();
        }

    settings->setValue("plugins", pluginIds.join(","));

    }
    return PLUGIN_ERROR_NO_ERROR;
}



QList<Plugin*> PluginManagerImpl::getPlugins(PluginRole role)
{
    DEBUG() << QString("getPlugins(%1)").arg(role);
    if(role == ROLE_UNKNOWN)
        return plugins;

    QList<Plugin*> result;
    foreach(Plugin* plugin, plugins)
    {
        foreach(PluginRole pluginRole, plugin->roles())
        {
            if(pluginRole == role)
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
    if(plugin->getState() == PLUGIN_STATE_INITIALIZED)
    {
        //INFO(QString("Plugin '%1' is already initialized").arg(plugin->name));
        return PLUGIN_ERROR_NO_ERROR;
    }

    Q_ASSERT(plugin->getId() != NULL);
    Q_ASSERT(plugin->getId().length() > 0);
    if(plugin->getId() == NULL || plugin->getId() == "")
        return PLUGIN_ERROR_NO_PLUGIN_ID;

    DEBUG() << qPrintable(spacing) << "Plugin" << plugin->getId() << "initialization...";
    DEBUG() << qPrintable(spacing) << "Loading dependencies for" << plugin->getId() << "...";

    if(plugin->dependencies().size() > 0)
    {
        plugin->setState(PLUGIN_STATE_WAITING_FOR_DEPENDENCY);
        PLUGIN_ERROR_CODES result = PLUGIN_ERROR_NO_ERROR;

        for (const PluginDependency &dependency: plugin->dependencies())
        {
            //Check dependency cycle

            bool cyclic = false;
            for(PluginRole role: plugin->roles())
            {
                if(role == dependency.getRole())
                {
                    cyclic = true;
                    break;
                }
            }

            if(cyclic)
            {
                WARN() << "Cyclic dependency in" << plugin->getId() << "with role" << dependency.roleName();
                continue;
            }

            DEBUG() << qPrintable(spacing) << "Trying to load dependency" << dependency.roleName() << " for" << plugin->getName();

            Plugin* dependencyPlugin = PluginManager::instance()->getByRole(dependency.getRole());
            if(dependencyPlugin == NULL)
            {
                WARN() << "Dependency" << dependency.roleName() << "for" << plugin->getId() << "not found!";
                if(dependency.isRequired())
                {
                    result = PLUGIN_ERROR_DEPENDENCY_MISSING;
                    break;
                }
                else
                {
                    LOG() << qPrintable(spacing) << "Skipping" << dependency.roleName() << "as not required";
                    continue;
                }
            }

            if(dependencyPlugin->getState() == PLUGIN_STATE_WAITING_FOR_DEPENDENCY)
            {
                WARN() << "Cyclic dependency in plugin" << plugin->getId() << "with" << dependencyPlugin->getId();
                continue;
            }



            PLUGIN_ERROR_CODES code = PluginManager::instance()->initPlugin(dependencyPlugin, dependencyLevel+1);
            if(code != PLUGIN_ERROR_NO_ERROR)
            {
                WARN() << "Error" << code << "occured while initializing plugin" << plugin->getName() << "dependency" << dependency.roleName();
                plugin->setState(PLUGIN_STATE_INITIALIZED);
                result = PLUGIN_ERROR_DEPENDENCY_MISSING;
            }
            else
            {
                DEBUG() << qPrintable(spacing) << "Dependency" << dependency.roleName() << "initialized";
            }
        }


        switch(result)
        {
            case PLUGIN_ERROR_NO_ERROR:
            {
                DEBUG() << qPrintable(spacing) << "Dependencies for" << plugin->getId() << "have been loaded.";
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
        plugin->setState(PLUGIN_STATE_DISABLED);
        return PLUGIN_ERROR_NOT_INITIALIZED;
    }
    DEBUG() << qPrintable(spacing) << "Plugin" << plugin->getId() << "initialized";

    plugin->setState(PLUGIN_STATE_INITIALIZED);
    return PLUGIN_ERROR_NO_ERROR;
}

PLUGIN_ERROR_CODES  PluginManagerImpl::deinitPlugin(Plugin *plugin)
{
    DEBUG() << "deinitPlugin(" << plugin->getName() << ")";

    PLUGIN_ERROR_CODES result = plugin->deinitialize();
    if(result == PLUGIN_ERROR_NO_ERROR)
        plugin->setState(PLUGIN_STATE_UNLOADED);


    /*
        while (!plugins.isEmpty()) {
           QPluginLoader* pluginLoader = plugins.takeFirst();
           pluginLoader->unload();
           delete pluginLoader;
        }
     */


    return result;
}

Plugin* PluginManagerImpl::getByRole(PluginRole role)
{
    for(int index = 0; index < plugins.size(); index++)
    {
        Plugin *plugin = plugins.at(index);
        for(PluginRole pluginRole: plugin->roles())
        {
            if(pluginRole == role)
            {
                Q_ASSERT(plugin);
                qDebug() << "Found plugin" << plugin << plugin->getName() << plugin->getClassName();
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

PLUGIN_ERROR_CODES PluginManagerImpl::initPlugins()
{
    DEBUG() << "initPlugins()";

    if(plugins.size() == 0)
        return PLUGIN_ERROR_NOT_INITIALIZED;

    for(int index = 0; index < plugins.size(); index++)
    {
        Plugin* plugin = plugins.at(index);
        initPlugin(plugin);

    }
    DEBUG() << "Initialization finished";
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
