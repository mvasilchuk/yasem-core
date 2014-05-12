#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "pluginmanager.h"

#include <QObject>

namespace yasem {


class PluginManagerImpl : public PluginManager
{
    Q_OBJECT
public:
    PluginManagerImpl();

// PluginManager interface
    virtual PLUGIN_ERROR_CODES listPlugins();
    virtual PLUGIN_ERROR_CODES initPlugins();
    virtual PLUGIN_ERROR_CODES deinitPlugins();
    //virtual PLUGIN_ERROR_CODES connectSlots();
    virtual QList<Plugin*> getPlugins(const QString &role);
    virtual PLUGIN_ERROR_CODES initPlugin(Plugin* plugin, int dependencyLevel);
    virtual PLUGIN_ERROR_CODES deinitPlugin(Plugin* plugin);
    virtual Plugin* getByRole(const QString &role, Plugin::PluginFlag flags);
    virtual Plugin *getByIID(const QString &iid);
    virtual void setPluginDir(const QString &pluginDir);
    virtual QString getPluginDir();

    virtual QList<Plugin::PluginRole> parseRoles(const QJsonArray &array);
    Plugin::PluginFlag parseFlags(const QString &flagsStr);
    virtual QList<PluginDependency*> convertDependenciesList(const QJsonArray &array);

    QStringList blacklistedPlugins;

    // PluginManager interface
public:
    //void loadProfiles();
};

}


#endif // PLUGINMANAGER_H
