#ifndef YASEMSETTINGSIMPL_H
#define YASEMSETTINGSIMPL_H

#include "yasemsettings.h"

#include <QObject>

namespace yasem {

class YasemSettingsImpl : public YasemSettings
{
    Q_OBJECT
public:
    explicit YasemSettingsImpl(QObject *parent = 0);

signals:

public slots:

    // YasemSettings interface
public:
    bool addConfigGroup(ConfigTreeGroup *group);
    QHash<const QString&, ConfigTreeGroup *> getConfigGroups();
    ConfigTreeGroup *getDefaultGroup(const QString &id);

protected:
    bool addBuiltInConfigGroup(ConfigTreeGroup *group);

    // YasemSettings interface
public slots:
    void save(ConfigContainer *container = 0);
    void reset(ConfigContainer *container = 0);
    void load(ConfigContainer *container = 0);
    void setItemDirty(ConfigItem* item, bool value = true);
    ConfigItem* findItem(const QStringList& path);
    ConfigItem* findItem(const QString& path);
    // YasemSettings interface
public slots:

};

}

#endif // YASEMSETTINGSIMPL_H
