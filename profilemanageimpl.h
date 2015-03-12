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

class ProfileManageImpl : public ProfileManager
{
    Q_OBJECT
public:
    explicit ProfileManageImpl(QObject *parent = 0);

signals:

public slots:

    // ProfileManager interface
public:
    QSet<Profile *> getProfiles();
    Profile *getActiveProfile();
    void addProfile(Profile *profile);
    void setActiveProfile(Profile *profile);
    bool removeProfile(Profile *profile);
    void loadProfiles();
    Profile* createProfile(const QString &classId, const QString &submodel, const QString &baseName, bool overwrite);
    void registerProfileClassId(const QString &classId, StbPluginObject* profilePlugin);
    QMap<QString, StbPluginObject*> getRegisteredClasses();
    StbPluginObject *getProfilePluginByClassId(const QString &idclassId);
    Profile *findById(const QString &id);
    Profile *findByName(const QString &id);
    Profile* backToPreviousProfile();

    void loadProfileKeymap(Profile *profile);
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
