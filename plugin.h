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

    virtual bool hasFlag(PluginFlag flag);

    virtual void addFlag(PluginFlag flag);

    /**
     * @brief initialize.
     *
     * Method will be called from PluginManager for all plugins after they are loaded and ready to be initialized.
     * @return
     */
    virtual PluginErrorCodes initialize();
    /**
     * @brief deinitialize.
     *
     * Method is called from PluginManager before app is closed.
     * @return
     */
    virtual PluginErrorCodes deinitialize();

    /**
     * @brief register_dependencies
     * Plugin should call add_dependency() method for each dependency it requires
     */
    virtual void register_dependencies() = 0;

    /**
     * @brief register_roles
     */
    virtual void register_roles() = 0;

    /**
     * @brief has_role
     * @param role
     * @return Plugin has a role
     */
    virtual bool has_role(PluginRole role);

    virtual QHash<PluginRole, AbstractPluginObject*> roles();

    virtual QList<PluginDependency> dependencies();

    virtual QString getIID();
    virtual void setIID(const QString &iid);

    virtual QString getClassName();
    virtual void setClassName(const QString &className);

    virtual QString getId();
    virtual void setId(const QString &id);

    virtual QString getVersion();
    virtual void setVersion(const QString &version);

    virtual QString getName();
    virtual void setName(const QString &name);

    virtual QJsonObject getMetadata();
    virtual void setMetadata(const QJsonObject &metadata);

    virtual PluginFlag getFlags();
    virtual void setFlags(const PluginFlag &flags);

    virtual PluginState getState();
    virtual void setState(const PluginState &state);

    virtual QString getStateDescription();

    virtual bool isActive();
    virtual void setActive(bool active);

    virtual QList<Plugin*> getRuntimeConflicts();
    virtual void addRuntimeConflict(Plugin* plugin);

    virtual QList<PluginConflict> getStaticConflicts();

signals:
    void started();
    void error_happened(int code);
    void finished();

protected:
    virtual void add_dependency(const PluginDependency &dependency);
    virtual void add_static_conflict(const PluginConflict &conflict_info);
    virtual void register_role(PluginRole role, AbstractPluginObject* obj);

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
