#ifndef IPLUGIN_H
#define IPLUGIN_H

#include "enums.h"
#include "macros.h"
#include "plugindependency.h"

#include <typeinfo>

#include <QtPlugin>
#include <QList>
#include <QPluginLoader>
#include <QCryptographicHash>
#include <QThread>

class QObject;
class QJsonObject;
class QStringList;
class QCoreApplication;
class QMetaType;

namespace yasem {

class PluginDependency;
class Plugin
{

public:
    Plugin() {

    }

    enum PluginFlag {
        NONE = 0,
        CLIENT = 1,
        SYSTEM = 2,
        HIDDEN = 4,
        GUI = 8
    };

    class PluginRole {
    public:
        QString name;
        PluginFlag flags;

        bool hasFlag(PluginFlag flag)
        {
            return (flags & flag) == flag;
        }

        void addFlag(PluginFlag flag)
        {
            this->flags = (PluginFlag)(this->flags | flag);
        }
    };

    virtual ~Plugin() {}

    QString IID;
    QString className;
    QString name;
    QString version;
    QString id;
    QList<PluginRole> roles;
    QList<PluginDependency*> dependencies;
    QJsonObject metadata;
    QPluginLoader loader;

    PluginFlag flags;

    bool hasFlag(PluginFlag flag)
    {
        return (flags & flag) == flag;
    }

    void addFlag(PluginFlag flag)
    {
        this->flags = (PluginFlag)(this->flags | flag);
    }

    bool active;



    PLUGIN_STATE state;

    /**
     * @brief initialize.
     *
     * Method will be called from PluginManager for all plugins after they are loaded and ready to be initialized.
     * @return
     */
    virtual PLUGIN_ERROR_CODES initialize() = 0;
    /**
     * @brief deinitialize.
     *
     * Method is called from PluginManager before app is closed.
     * @return
     */
    virtual PLUGIN_ERROR_CODES deinitialize() = 0;

};

}

#define Plugin_iid "com.mvas.yasem.Plugin/1.0"

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(yasem::Plugin, Plugin_iid)
QT_END_NAMESPACE

#endif // IPLUGIN_H
