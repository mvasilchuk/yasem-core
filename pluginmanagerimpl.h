#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "pluginmanager.h"
#include "config.h"
#include "plugindependency.h"

#include <QObject>

namespace yasem {

class PluginManagerImpl : public SDK::PluginManager
{
    Q_OBJECT
public:
    PluginManagerImpl();

// PluginManager interface
    virtual SDK::PluginErrorCodes listPlugins();
    virtual SDK::PluginErrorCodes initPlugins();
    virtual SDK::PluginErrorCodes deinitPlugins();
    //virtual PLUGIN_ERROR_CODES connectSlots();
    virtual QList<SDK::Plugin*> getPlugins(SDK::PluginRole role = SDK::ROLE_ANY, bool active_only = true);
    virtual SDK::AbstractPluginObject* getByRole(SDK::PluginRole role, bool show_warning = true);
    virtual SDK::Plugin* getByIID(const QString &iid);
    virtual void setPluginDir(const QString &pluginDir);
    virtual QString getPluginDir();

    SDK::PluginFlag parseFlags(const QString &flagsStr);

    bool pluginHasConflicts(SDK::Plugin* plugin);


    // PluginManager interface
protected:
    void registerPluginRole(const SDK::PluginRole &role, const SDK::PluginRoleData &data);

    SDK::ConfigContainer* m_plugins_config;

protected slots:
    void onPluginLoaded();
    void onPluginUnloaded();
    void onPluginInitialized();
    void onPluginDeinitialized();

    SDK::PluginErrorCodes initializePlugin(SDK::Plugin* plugin, bool go_deeper = true, bool ignore_dependencies = false);
    SDK::PluginErrorCodes deinitializePlugin(SDK::Plugin* plugin);
    QList<SDK::PluginDependency> getUnresolvedDependencies(SDK::Plugin* plugin);
    QStringList getDependencyNames(QList<SDK::PluginDependency> list);

};

}


#endif // PLUGINMANAGER_H
