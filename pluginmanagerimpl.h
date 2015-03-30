#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "pluginmanager.h"

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
    virtual PluginErrorCodes initPlugin(Plugin* plugin);
    virtual PluginErrorCodes deinitPlugin(Plugin* plugin);
    virtual AbstractPluginObject* getByRole(PluginRole role);
    virtual QList<AbstractPluginObject*> getAllByRole(PluginRole role, bool active_only = true);
    virtual Plugin*getByIID(const QString &iid);
    virtual void setPluginDir(const QString &pluginDir);
    virtual QString getPluginDir();

    PluginFlag parseFlags(const QString &flagsStr);

    QStringList blacklistedPlugins;

    bool pluginHasConflicts(Plugin* plugin);


    // PluginManager interface
protected:
    void registerPluginRole(const PluginRole &role, const PluginRoleData &data);
    bool isCircularDependency(Plugin* plugin, const PluginDependency &dependency) const;
    PluginErrorCodes loadDependency(Plugin* plugin, const PluginDependency &dependency);

};

}


#endif // PLUGINMANAGER_H
