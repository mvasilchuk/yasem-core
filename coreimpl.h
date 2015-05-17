#ifndef COREIMPL_H
#define COREIMPL_H

#include "core.h"
#include <QObject>

namespace yasem {

class CoreImpl : public Core
{
    Q_OBJECT
public:
    explicit CoreImpl(QObject *parent = 0);
    virtual ~CoreImpl();
signals:

public slots:
    // Core interface
public:
    QSettings *settings();
    QSettings* settings(const QString &filename);

    // Core interface
    QHash<QString, RC_KEY> getKeycodeHashes();
public slots:
    void onClose();

protected:
    Core* coreInstance;
    QSettings* appSettings;
    CoreNetwork* networkObj;
    QList<DiskInfo *> disksList;
    YasemSettings* m_yasem_settings;

    void initBuiltInSettingsGroup();
    void initSettings();

    // Core interface
    void fillKeymapHashTable();
public slots:
    void mountPointChanged();
    void buildBlockDeviceTree();
    QList<DiskInfo *> disks();
    CoreNetwork* network();
    QThread* mainThread();

    // Core interface
public:
    QString version();

    // Core interface
public:
    QString revision();

    // Core interface
public:
    YasemSettings *yasem_settings();
};

}


#endif // COREIMPL_H
