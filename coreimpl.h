#ifndef COREIMPL_H
#define COREIMPL_H

#include "core.h"
#include <QObject>

namespace yasem {

class Statistics;

class CoreImpl : public Core
{
    Q_OBJECT
public:
    explicit CoreImpl(QObject *parent = 0);
    virtual ~CoreImpl();
signals:

protected:
    QSettings* m_app_settings;
    CoreNetwork* m_network;
    YasemSettings* m_yasem_settings;
    Statistics* m_statistics;
    QList<StorageInfo *> m_disks;

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
    YasemSettings *yasem_settings();

    QList<StorageInfo *> storages();
    CoreNetwork* network();
    Statistics* statistics();
    VirtualMachine getVM();

    // Core interface
    QHash<QString, RC_KEY> getKeycodeHashes();
    Q_INVOKABLE QString version();
    Q_INVOKABLE QString revision();
    Q_INVOKABLE QString compiler();
    Q_INVOKABLE QString getConfigDir() const;

};

}


#endif // COREIMPL_H
