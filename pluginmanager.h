#ifndef IPLUGINMANAGER_H
#define IPLUGINMANAGER_H

#include "enums.h"
#include "plugin.h"

#include <QObject>
#include <QList>
#include <QStringList>
#include <QJsonArray>
#include <QCoreApplication>
#include <QVariant>

namespace yasem {

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

    virtual PLUGIN_ERROR_CODES listPlugins() = 0;
    virtual PLUGIN_ERROR_CODES initPlugins() = 0;
    virtual PLUGIN_ERROR_CODES deinitPlugins() = 0;
    //virtual PLUGIN_ERROR_CODES connectSlots();
    virtual QList<Plugin*> getPlugins(PluginRole role) = 0;
    virtual PLUGIN_ERROR_CODES initPlugin(Plugin* plugin, int dependencyLevel = 0) = 0;
    virtual PLUGIN_ERROR_CODES deinitPlugin(Plugin* plugin) = 0;
    virtual Plugin* getByRole(PluginRole role) = 0;
    virtual Plugin* getByIID(const QString &iid) = 0;
    virtual void setPluginDir(const QString &pluginDir) = 0;
    virtual QString getPluginDir() = 0;
    //virtual void loadProfiles() = 0;
protected:
    PluginManager() {}
    QList<Plugin*> plugins;
    virtual PluginFlag parseFlags(const QString &flagsStr) = 0;
    QString pluginDir;

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
