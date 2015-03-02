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

    // Core interface
    QHash<QString, RC_KEY> getKeycodeHashes();
public slots:
    void onClose();

protected:
    Core* coreInstance;
    QSettings* appSettings;
    CoreNetwork* networkObj;
    QList<DiskInfo *> disksList;

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
};

}


#endif // COREIMPL_H
