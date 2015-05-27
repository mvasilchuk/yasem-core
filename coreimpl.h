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
    QList<DiskInfo *> m_disks;

    void initBuiltInSettingsGroup();
    void initSettings();

    // Core interface
    void fillKeymapHashTable();
public slots:
    void onClose();
    void mountPointChanged();
    void buildBlockDeviceTree();
    QList<DiskInfo *> disks();
    CoreNetwork* network();
    QThread* mainThread();

    // Core interface
public:
    QSettings *settings();
    QSettings* settings(const QString &filename);
    YasemSettings *yasem_settings();
    virtual Statistics *statistics();

    // Core interface
    QHash<QString, RC_KEY> getKeycodeHashes();
    QString version();
    QString revision();
};

}


#endif // COREIMPL_H
