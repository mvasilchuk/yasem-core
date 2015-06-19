#ifndef IPLUGIN_H
#define IPLUGIN_H

#include "enums.h"
#include "macros.h"
#include "plugindependency.h"

#include <QObject>
#include <QList>
#include <QJsonObject>
#include <QPluginLoader>

class QStringList;
class QCoreApplication;
class QMetaType;

namespace yasem {
class PluginPrivate;
class AbstractPluginObject;

class PluginConflict {
public:
    PluginConflict(const QString &id, const QString &name, PluginConflictType type):
        m_id(id),
        m_name(name),
        m_type(type)
    {

    }

     QString id()               const { return m_id; }
     QString name()             const { return m_name; }
     PluginConflictType type()  const { return m_type; }

protected:
    QString m_id;
    QString m_name;
    PluginConflictType m_type;
};

class Plugin: public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Plugin)
public:

    explicit Plugin(QObject* parent = NULL);
    virtual ~Plugin();

    Q_INVOKABLE virtual bool hasFlag(PluginFlag flag);

    Q_INVOKABLE virtual void addFlag(PluginFlag flag);

    /**
     * @brief initialize.
     *
     * Method will be called from PluginManager for all plugins after they are loaded and ready to be initialized.
     * @return
     */
    Q_INVOKABLE virtual PluginErrorCodes initialize();
    /**
     * @brief deinitialize.
     *
     * Method is called from PluginManager before app is closed.
     * @return
     */
    Q_INVOKABLE virtual PluginErrorCodes deinitialize();

    /**
     * @brief register_dependencies
     * Plugin should call add_dependency() method for each dependency it requires
     */
    Q_INVOKABLE virtual void register_dependencies() = 0;

    /**
     * @brief register_roles
     */
    Q_INVOKABLE virtual void register_roles() = 0;

    /**
     * @brief has_role
     * @param role
     * @return Plugin has a role
     */
    Q_INVOKABLE virtual bool has_role(PluginRole role);

    Q_INVOKABLE virtual QHash<PluginRole, AbstractPluginObject*> roles();

    Q_INVOKABLE virtual QList<PluginDependency> dependencies();

    Q_INVOKABLE virtual QString getIID();
    Q_INVOKABLE virtual void setIID(const QString &iid);

    Q_INVOKABLE virtual QString getClassName();
    Q_INVOKABLE virtual void setClassName(const QString &className);

    Q_INVOKABLE virtual QString getId();
    Q_INVOKABLE virtual void setId(const QString &id);

    Q_INVOKABLE virtual QString getVersion();
    Q_INVOKABLE virtual void setVersion(const QString &version);

    Q_INVOKABLE virtual QString getRevision() const;

    Q_INVOKABLE virtual QString getName();
    Q_INVOKABLE virtual void setName(const QString &name);

    Q_INVOKABLE virtual QJsonObject getMetadata();
    Q_INVOKABLE virtual void setMetadata(const QJsonObject &metadata);

    Q_INVOKABLE virtual PluginFlag getFlags();
    Q_INVOKABLE virtual void setFlags(const PluginFlag &flags);

    Q_INVOKABLE virtual PluginState getState();
    Q_INVOKABLE virtual void setState(const PluginState &state);

    Q_INVOKABLE virtual QString getStateDescription();

    Q_INVOKABLE virtual bool isActive();
    Q_INVOKABLE virtual void setActive(bool active);

    Q_INVOKABLE virtual QList<Plugin*> getRuntimeConflicts();
    Q_INVOKABLE virtual void addRuntimeConflict(Plugin* plugin);

    Q_INVOKABLE virtual QList<PluginConflict> getStaticConflicts();

    Q_INVOKABLE virtual bool isMultithreadingEnabled();

signals:
    void loaded();
    void unloaded();

    void initialized();
    void deinitialized();
    void got_conflict();
    void error_happened();
    void disabled();
    void waiting_for_dependency();
    void disabled_by_dependency();
    void plugin_thread_started();

protected:
    virtual void add_dependency(const PluginDependency &dependency);
    virtual void add_static_conflict(const PluginConflict &conflict_info);
    virtual void register_role(PluginRole role, AbstractPluginObject* obj);
    virtual void setMultithreading(bool enable);

protected:
    Plugin(PluginPrivate &d): d_ptr(&d) {}
    // allow subclasses to initialize with their own concrete Private
    PluginPrivate *d_ptr;    
};
}

#define Plugin_iid "com.mvas.yasem.Plugin/1.0"

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(yasem::Plugin, Plugin_iid)
QT_END_NAMESPACE

#endif // IPLUGIN_H
