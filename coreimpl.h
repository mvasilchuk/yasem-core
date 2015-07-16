#ifndef COREIMPL_H
#define COREIMPL_H

#include "core.h"
#include <QObject>

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
signals:

protected:
    QSettings* m_app_settings;
    SDK::CoreNetwork* m_network;
    SDK::Config* m_yasem_settings;
    SDK::Statistics* m_statistics;
    QList<SDK::StorageInfo *> m_disks;

    QString m_config_dir;
    VirtualMachine m_detected_vm;

    void initBuiltInSettingsGroup();
    void initSettings();

    // Core interface
    void fillKeymapHashTable();
public slots:
    void onClose();
    void mountPointChanged();
    void buildBlockDeviceTree();

    QThread* mainThread();

    // Core interface
public:
    QSettings *settings();
    QSettings* settings(const QString &filename);
    SDK::Config *yasem_settings();

    QList<SDK::StorageInfo *> storages();
    SDK::CoreNetwork* network();
    SDK::Statistics* statistics();
    VirtualMachine getVM();

    // Core interface
    Q_INVOKABLE QString version();
    Q_INVOKABLE QString revision();
    Q_INVOKABLE QString compiler();
    Q_INVOKABLE QString getConfigDir() const;


    // Core interface
public:
    virtual void init();
};

}


#endif // COREIMPL_H
