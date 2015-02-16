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
#include <QJsonObject>

class QObject;
class QStringList;
class QCoreApplication;
class QMetaType;

namespace yasem {

class Plugin;

class PluginPrivate {
public:
    // Constructor that initializes the q-ptr

    PluginPrivate(){
        active = false;
        flags = PluginFlag::PLUGIN_FLAG_NONE;
        state = PLUGIN_STATE_UNKNOWN;
        q_ptr = NULL;
    }
    PluginPrivate(Plugin *q) : q_ptr(q) {
    }
    Plugin *q_ptr; // q-ptr points to the API class
    QList<PluginRole> role_list;
    QList<PluginDependency> dependency_list;

    QString IID;
    QString className;
    QString name;
    QString version;
    QString id;
    QJsonObject metadata;
    QPluginLoader loader;
    bool active;

    PluginFlag flags;
    PLUGIN_STATE state;

};

class Plugin
{

public:

    Plugin() : d_ptr(new PluginPrivate(this)) {}

    virtual ~Plugin() {}

    bool hasFlag(PluginFlag flag)
    {
        return (getFlags() & flag) == flag;
    }

    void addFlag(PluginFlag flag)
    {
        this->setFlags((PluginFlag)(getFlags() | flag));
    }



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

    virtual void register_dependencies() = 0;

    QList<PluginDependency> dependencies()
    {
        return d_ptr->dependency_list;
    }

    virtual void add_dependency(const PluginDependency &dependency)
    {
        d_ptr->dependency_list.append(dependency);
    }


    virtual void register_roles() = 0;

    void register_role(PluginRole role)
    {
        d_ptr->role_list.append(role);
    }

    bool has_role(PluginRole role)
    {
        return d_ptr->role_list.contains(role);
    }

    QList<PluginRole> roles()
    {
        return d_ptr->role_list;
    }

    QString getIID() const { return d_ptr->IID; }
    void setIID(const QString &iid) { d_ptr->IID = iid; }

    QString getClassName() const { return d_ptr->className; }
    void setClassName(const QString &className) { d_ptr->className = className; }

    QString getId() const { return d_ptr->id; }
    void setId(const QString &id) { d_ptr->id = id; }

    QString getVersion() const { return d_ptr->version; }
    void setVersion(const QString &version) { d_ptr->version = version; }

    QString getName() const { return d_ptr->name; }
    void setName(const QString &name) { d_ptr->name = name; }

    QJsonObject getMetadata() const { return d_ptr->metadata; }
    void setMetadata(const QJsonObject &metadata) { d_ptr->metadata = metadata; }

    //QPluginLoader& getLoader() const { return d_ptr->loader; }
    //void setLoader(QPluginLoader &loader) { d_ptr->loader = loader; }


    PluginFlag getFlags() const { return d_ptr->flags; }
    void setFlags(const PluginFlag &flags) { d_ptr->flags = flags; }

    PLUGIN_STATE getState() const { return d_ptr->state; }
    void setState(const PLUGIN_STATE &state) { d_ptr->state = state; }

    bool isActive() const { return d_ptr->active; }
    void setActive(bool active) { d_ptr->active = active; }
protected:
    Plugin(PluginPrivate &d): d_ptr(&d) {}
        // allow subclasses to initialize with their own concrete Private
    PluginPrivate *d_ptr;

    Q_DECLARE_PRIVATE(Plugin)
};

}

#define Plugin_iid "com.mvas.yasem.Plugin/1.0"

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(yasem::Plugin, Plugin_iid)
QT_END_NAMESPACE

#endif // IPLUGIN_H
