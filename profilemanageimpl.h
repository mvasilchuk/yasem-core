#ifndef PROFILEMANAGEIMPL_H
#define PROFILEMANAGEIMPL_H

#include "profilemanager.h"

#include <QObject>
#include <QHash>

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
    Profile* createProfile(const QString &classId, const QString &baseName);
    void registerProfileClassId(const QString &classId, StbProfilePlugin* profilePlugin);
    QMap<QString, StbProfilePlugin*> getRegisteredClasses();
    StbProfilePlugin *getProfilePluginByClassId(const QString &idclassId);
    Profile *findById(const QString &id);
    Profile* backToPreviousProifile();

protected:
    QString createUniqueName(const QString &classId, const QString &baseName);

};

}

#endif // PROFILEMANAGEIMPL_H
