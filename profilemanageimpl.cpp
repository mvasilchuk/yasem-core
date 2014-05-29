#include "profilemanageimpl.h"
#include "core.h"
#include "datasource.h"
#include "stbprofileplugin.h"
#include "datasource.h"
#include "datasourceplugin.h"
#include "stbplugin.h"

#include <QFile>
#include <QDir>
#include <QRegularExpression>

using namespace yasem;

ProfileManageImpl::ProfileManageImpl(QObject *parent): ProfileManager(parent)
{
    activeProfile = NULL;
}

QSet<Profile *> ProfileManageImpl::getProfiles()
{
    return profilesList;
}

Profile *ProfileManageImpl::getActiveProfile()
{
    return activeProfile;
}

void ProfileManageImpl::addProfile(Profile *profile)
{
    Q_ASSERT(profile);
    if(!profilesList.contains(profile))
    {
        profilesList.insert(profile);
        emit profileAdded(profile);
    }
}

void ProfileManageImpl::setActiveProfile(Profile *profile)
{
    Q_ASSERT(profile);
    foreach (Profile* item, profilesList) {
        if(item == profile)
        {
            activeProfile = profile;
            Core::instance()->settings()->setValue("active_profile", profile->getId());

            INFO(QString("Active profile: %1").arg(profile->getId()));

            profile->getProfilePlugin()->init();
            profile->start();

            profileStack.push(profile);

            emit profileChanged(profile);
            return;
        }
    }

    WARN(QString("Cannot change profile '%1': not found!").arg(profile->getId()));
}

bool ProfileManageImpl::removeProfile(Profile *profile)
{
    if(profile == activeProfile)
        return false;

    return profilesList.remove(profile);
    emit profileRemoved();
}

void ProfileManageImpl::loadProfiles()
{
    QDir profilesDir = QFileInfo(Core::instance()->settings()->fileName()).absoluteDir();
    QString profilePath = Core::instance()->settings()->value(CONFIG_PROFILES_DIR, "profiles").toString();

    qDebug() << "Looking for profiles in" << profilePath;
    if(!profilesDir.cd(profilePath))
    {
        ERROR(QString("Cannot go to profiles dir: %1   ").arg(profilesDir.dirName()).append(Core::instance()->settings()->fileName()));
        return;
    }

    DEBUG(QString("Searching for profiles in %1").arg(profilesDir.path()));

    profilesDir.setNameFilters(QStringList() << "*.ini");

    DatasourcePlugin* dsPlugin = dynamic_cast<DatasourcePlugin*>(PluginManager::instance()->getByRole("datasource"));

    foreach (QString fileName, profilesDir.entryList(QDir::Files | QDir::NoSymLinks | QDir::Readable))
    {
        DEBUG(QString("Loading profile from %1").arg(fileName));
        QSettings s(profilesDir.path().append("/").append(fileName), QSettings::IniFormat);

        s.beginGroup("profile");
        QString classId = s.value("classid").toString();
        StbProfilePlugin* profilePlugin = ProfileManager::instance()->getProfilePluginByClassId(classId);

        if(profilePlugin == NULL)
        {
            WARN(QString("Plugin for profile classid '%1' not found!").arg(classId));
            continue;
        }

        Profile* profile = profilePlugin->createProfile(s.value("uuid").toString());
        Q_ASSERT(profile);
        profile->datasource(dsPlugin->getDatasourceForProfile(profile));
        //qDebug() << "PROFILE DS: " << profile << profile->datasource();

        profile->setName(s.value("name").toString());

        s.endGroup();

        profilesList.insert(profile);
    }
}

Profile* ProfileManageImpl::createProfile(const QString &classId, const QString &baseName = "")
{
    DatasourcePlugin* dsPlugin = dynamic_cast<DatasourcePlugin*>(PluginManager::instance()->getByRole("datasource"));

    Q_ASSERT(classId != "");
    STUB_WITH_LIST(QStringList() << baseName << classId);
    StbProfilePlugin* profilePlugin = getProfilePluginByClassId(classId);
    Q_ASSERT(profilePlugin != NULL);
    Profile* profile = profilePlugin->createProfile();

    Q_ASSERT(profile);
    profile->setName(createUniqueName(classId, baseName));
    profile->datasource(dsPlugin->getDatasourceForProfile(profile));

    profile->datasource()->set("profile", "uuid", profile->getId());
    profile->datasource()->set("profile", "name", profile->getName());
    profile->datasource()->set("profile", "classid", classId);
    profile->initDefaults();

    return profile;
}

void ProfileManageImpl::registerProfileClassId(const QString &classId, StbProfilePlugin* profilePlugin)
{
    if(!profileClasses.contains(classId))
    {
        INFO(QString("Registered '%1' as profile class ID").arg(classId));
        profileClasses.insert(classId, profilePlugin);
    }
    else
        WARN(QString("Profile class ID '%1' has bee already registered!").arg(classId));
}

QMap<QString, StbProfilePlugin *> ProfileManageImpl::getRegisteredClasses()
{
    return profileClasses;
}

StbProfilePlugin *ProfileManageImpl::getProfilePluginByClassId(const QString &classId)
{
    for(auto iterator = profileClasses.begin(); iterator != profileClasses.end(); iterator++)
    {
        if(iterator.key() == classId)
            return iterator.value();
    }

    WARN(QString("Profile plugin '%1' not found!").arg(classId));
    return NULL;
}

Profile *ProfileManageImpl::findById(const QString &id)
{
    foreach (Profile* profile, profilesList) {
        if(id == profile->getId())
        {
            qDebug() << "PROFILE: " << profile;
            return profile;
        }
    }

    ERROR(QString("Profile '%1' not found!").arg(id));
    return NULL;
}

Profile *ProfileManageImpl::backToPreviousProifile()
{
    qDebug() << "profiles:" << profileStack.size();
    if(profileStack.size() < 1)
        return NULL;

    Profile* currentProfile = profileStack.pop();
    if(profileStack.size() < 1) return currentProfile;

    Profile* profile = profileStack.pop();
    Q_ASSERT(profile);
    setActiveProfile(profile);
    return profile;
}

QString ProfileManageImpl::createUniqueName(const QString &classId, const QString &baseName = "")
{
    const QString newProfileName = baseName != "" ? baseName : QObject::tr("New %1 profile").arg(classId.toCaseFolded());

    QRegularExpression rx(QString("^%1(?:\\s\\#(\\d+))?$").arg(newProfileName));
    int maxIndex = 0;

    foreach(Profile* profile, ProfileManager::instance()->getProfiles())
    {

        auto match = rx.match(profile->getName());
        if(match.hasMatch())
        {
            if(match.capturedLength() > 1)
            {
                int currIndex = match.captured(1).toInt();
                currIndex  = currIndex > 0 ? currIndex : 1;
                if(currIndex > maxIndex)
                    maxIndex = currIndex;
            }
        }

    }

    maxIndex++;

    return maxIndex > 1 ? newProfileName + QString(" #") + QString::number(maxIndex) : newProfileName;
}

