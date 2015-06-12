#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "pluginmanager.h"
#include "yasemsettings.h"
#include "plugindependency.h"

#include <QObject>

namespace yasem {

class PluginDependency;
class PluginManagerImpl : public PluginManager
{
    Q_OBJECT
public:
    PluginManagerImpl();

// PluginManager interface
    virtual PluginErrorCodes listPlugins();
    virtual PluginErrorCodes initPlugins();
    virtual PluginErrorCodes deinitPlugins();
    //virtual PLUGIN_ERROR_CODES connectSlots();
    virtual QList<Plugin*> getPlugins(PluginRole role = ROLE_ANY, bool active_only = true);
    virtual AbstractPluginObject* getByRole(PluginRole role, bool show_warning = true);
    virtual QList<AbstractPluginObject*> getAllByRole(PluginRole role, bool active_only = true);
    virtual Plugin*getByIID(const QString &iid);
    virtual void setPluginDir(const QString &pluginDir);
    virtual QString getPluginDir();

    PluginFlag parseFlags(const QString &flagsStr);

    bool pluginHasConflicts(Plugin* plugin);


    // PluginManager interface
protected:
    void registerPluginRole(const PluginRole &role, const PluginRoleData &data);

    ConfigContainer* m_plugins_config;

protected slots:
    void onPluginLoaded();
    void onPluginUnloaded();
    void onPluginInitialized();
    void onPluginDeinitialized();

    PluginErrorCodes initializePlugin(Plugin* plugin, bool go_deeper = true, bool ignore_dependencies = false);
    PluginErrorCodes deinitializePlugin(Plugin* plugin);
    QList<PluginDependency> getUnresolvedDependencies(Plugin* plugin);
    QStringList getDependencyNames(QList<PluginDependency> list);

};

}


#endif // PLUGINMANAGER_H
