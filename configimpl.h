#ifndef YASEMSETTINGSIMPL_H
#define YASEMSETTINGSIMPL_H

#include "yasemsettings.h"

#include <QObject>

namespace yasem {

class ConfigImpl : public SDK::Config
{
    Q_OBJECT
public:
    explicit ConfigImpl(QObject *parent = 0);

signals:

public slots:

    // YasemSettings interface
public:
    bool addConfigGroup(SDK::ConfigTreeGroup *group);
    QHash<const QString&, SDK::ConfigTreeGroup *> getConfigGroups();
    SDK::ConfigTreeGroup *getDefaultGroup(const QString &id);

protected:
    bool addBuiltInConfigGroup(SDK::ConfigTreeGroup *group);

    // YasemSettings interface
public slots:
    void save(SDK::ConfigContainer *container = 0);
    void reset(SDK::ConfigContainer *container = 0);
    void load(SDK::ConfigContainer *container = 0);
    void setItemDirty(SDK::ConfigItem* item, bool value = true);
    SDK::ConfigItem* findItem(const QStringList& path);
    SDK::ConfigItem* findItem(const QString& path);
    // YasemSettings interface
public slots:

};

}

#endif // YASEMSETTINGSIMPL_H
