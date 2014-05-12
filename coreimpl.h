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

signals:

public slots:
    // Core interface
public:
    QSettings *settings();

    // Core interface
public slots:
    void onClose();

protected:
    Core* coreInstance;
    QSettings* appSettings;
    Network* networkObj;
    QList<DiskInfo *> disksList;

    // Core interface
public slots:
    void mountPointChanged();
    QList<DiskInfo *> disks();
    Network* network();
    QThread* mainThread();
};

}


#endif // COREIMPL_H
