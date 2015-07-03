#ifndef PROFILEMANAGEIMPL_H
#define PROFILEMANAGEIMPL_H

#include "profilemanager.h"

#include <QObject>
#include <QHash>
#include <QFile>
#include <QDir>
#include <QSettings>

namespace yasem
{

class ProfileManageImpl : public SDK::ProfileManager
{
    Q_OBJECT
public:
    explicit ProfileManageImpl(QObject *parent = 0);

signals:

public slots:

    // ProfileManager interface
public:
    QSet<SDK::Profile *> getProfiles();
    SDK::Profile *getActiveProfile();
    void addProfile(SDK::Profile *profile);
    void setActiveProfile(SDK::Profile *profile);
    bool removeProfile(SDK::Profile *profile);
    void loadProfiles();
    SDK::Profile* createProfile(const QString &classId, const QString &submodel, const QString &baseName, bool overwrite);
    void registerProfileClassId(const QString &classId, SDK::StbPluginObject* profilePlugin);
    QMap<QString, SDK::StbPluginObject*> getRegisteredClasses();
    SDK::StbPluginObject *getProfilePluginByClassId(const QString &idclassId);
    SDK::Profile *findById(const QString &id);
    SDK::Profile *findByName(const QString &id);
    SDK::Profile* backToPreviousProfile();

    void loadProfileKeymap(SDK::Profile *profile);
protected:
    QDir profilesDir;
    QString createUniqueName(const QString &classId, const QString &baseName, bool overwrite);

    void loadDefaultKeymapFileIfNotExists(QSettings& keymap, const QString &classId);

    // ProfileManager interface
public:
    void backToMainPage();
    bool canGoBack();
};

}

#endif // PROFILEMANAGEIMPL_H
