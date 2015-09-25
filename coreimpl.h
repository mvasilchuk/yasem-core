#ifndef COREIMPL_H
#define COREIMPL_H

#include "core.h"
#include <QObject>
#include <QPointer>
#include <QScopedPointer>

namespace yasem {

namespace SDK {
class Statistics;
}

class CoreImpl : public SDK::Core
{
    Q_OBJECT
public:
    explicit CoreImpl(QObject *parent = 0);
    virtual ~CoreImpl();

    virtual void init() Q_DECL_OVERRIDE;
    QHash<QString, QString> arguments() const Q_DECL_OVERRIDE;
    QSettings *settings() Q_DECL_OVERRIDE;
    QSettings* settings(const QString &filename) Q_DECL_OVERRIDE;
    virtual SDK::Config* yasem_settings() const Q_DECL_OVERRIDE;

    QList<SDK::StorageInfo *> storages() Q_DECL_OVERRIDE;
    virtual SDK::CoreNetwork* network() const Q_DECL_OVERRIDE;
    virtual SDK::Statistics* statistics() const Q_DECL_OVERRIDE;
    VirtualMachine getVM() Q_DECL_OVERRIDE;
    bool featureAvailable(const Feature feature) const Q_DECL_OVERRIDE;

    // Core interface
    Q_INVOKABLE QString version() const Q_DECL_OVERRIDE;
    Q_INVOKABLE QString revision() const Q_DECL_OVERRIDE;
    Q_INVOKABLE QString compiler() const Q_DECL_OVERRIDE;
    Q_INVOKABLE QString getConfigDir() const Q_DECL_OVERRIDE;

public slots:
    void onClose() Q_DECL_OVERRIDE;
    void mountPointChanged() Q_DECL_OVERRIDE;

    QThread* mainThread() Q_DECL_OVERRIDE;

protected:
    void parseCommandLineArgs() Q_DECL_OVERRIDE;
    void initBuiltInSettingsGroup();
    void initSettings();
    void fillKeymapHashTable();
    void buildBlockDeviceTree();
    void checkCmdLineArgs();
    void printHelp();

    QSettings* m_app_settings;
    SDK::CoreNetwork* m_network;
    SDK::Config* m_yasem_settings;
    SDK::Statistics* m_statistics;
    QList<SDK::StorageInfo *> m_disks;
    Features m_features;

    QString m_config_dir;
    VirtualMachine m_detected_vm;
    QHash<QString, QString> m_cmd_line_args;

    // Core interface
public:

};

}


#endif // COREIMPL_H
