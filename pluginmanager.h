#ifndef IPLUGINMANAGER_H
#define IPLUGINMANAGER_H

#include "enums.h"

#include <QObject>
#include <QList>
#include <QStringList>
#include <QJsonArray>
#include <QCoreApplication>
#include <QVariant>
#include <QHash>

namespace yasem {

class Plugin;
class AbstractPluginObject;


struct PluginRoleData {

    PluginRoleData() {}

    PluginRoleData(const QString &name, const QString &description, QList<PluginRole> conflicts_with = QList<PluginRole>())
        :m_name(name),
         m_description(description),
         m_conflicts_with(conflicts_with)
    {

    }

    PluginRoleData(const PluginRoleData& other)
        :m_name(other.m_name),
         m_description(other.m_description),
         m_conflicts_with(other.m_conflicts_with)
    {
    }

    QString m_name;
    QString m_description;
    QList<PluginRole> m_conflicts_with;

};

class PluginManager : public QObject
{
    Q_OBJECT
public:
    static PluginManager* setInstance(PluginManager* inst = 0)
    {
        static PluginManager* instance = inst;// Guaranteed to be destroyed.

        if(instance == NULL)
            instance = static_cast<PluginManager*>(qApp->property("PluginManager").value<QObject*>());

        Q_CHECK_PTR(instance);
        return instance;
    }

    static PluginManager* instance()
    {
       return setInstance();
    }

    virtual PluginErrorCodes listPlugins() = 0;
    virtual PluginErrorCodes initPlugins() = 0;
    virtual PluginErrorCodes deinitPlugins() = 0;
    //virtual PLUGIN_ERROR_CODES connectSlots();
    virtual QList<Plugin*> getPlugins(PluginRole role, bool active_only) = 0;
    virtual PluginErrorCodes initPlugin(Plugin* plugin) = 0;
    virtual PluginErrorCodes deinitPlugin(Plugin* plugin) = 0;
    virtual AbstractPluginObject* getByRole(PluginRole role) = 0;
    virtual QList<AbstractPluginObject*> getAllByRole(PluginRole role, bool active_only = true) = 0;
    virtual Plugin* getByIID(const QString &iid) = 0;
    virtual void setPluginDir(const QString &pluginDir) = 0;
    virtual QString getPluginDir() = 0;

    //virtual void loadProfiles() = 0;
protected:
    PluginManager() {}
    QList<Plugin*> plugins;
    virtual PluginFlag parseFlags(const QString &flagsStr) = 0;
    QString pluginDir;

    virtual void registerPluginRole(const PluginRole& role, const PluginRoleData& data) = 0;

    QHash<PluginRole, PluginRoleData> m_plugin_roles;

private:

    // Dont forget to declare these two. You want to make sure they
    // are unaccessable otherwise you may accidently get copies of
    // your singleton appearing.
    PluginManager(PluginManager const&);              // Don't Implement
    void operator=(PluginManager const&); // Don't implement

signals:
    void pluginLoaded(Plugin* plugin);
    void pluginUnloaded(Plugin* plugin);
    void pluginInitialized(Plugin* plugin);
    void pluginDeinitialized(Plugin* plugin);

};

}


#endif // IPLUGINMANAGER_H
