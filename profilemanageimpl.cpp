#include "profilemanageimpl.h"
#include "core.h"
#include "datasource.h"
#include "stbprofileplugin.h"
#include "datasource.h"
#include "datasourceplugin.h"
#include "stbplugin.h"
#include "browserplugin.h"

#include <QFile>
#include <QDir>
#include <QRegularExpression>

using namespace yasem;

ProfileManageImpl::ProfileManageImpl(QObject *parent): ProfileManager(parent)
{
    activeProfile = NULL;
    profilesDir = QFileInfo(Core::instance()->settings()->fileName()).absoluteDir();
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

            qDebug() << QString("Active profile: %1").arg(profile->getName());

            profile->getProfilePlugin()->init();

            loadProfileKeymap(profile);

            profile->start();

            profileStack.push(profile);

            emit profileChanged(profile);
            return;
        }
    }

    qWarning() << QString("Cannot change profile '%1': not found!").arg(profile->getId());
}

void ProfileManageImpl::loadDefaultKeymapFileIfNotExists(QSettings& keymap, const QString &classId)
{
    QFile file(keymap.fileName());
    if(!file.exists())
    {
        QFileInfo fileInfo(file);
        QDir dir = fileInfo.absoluteDir();

        if(!dir.exists())
        {
            DEBUG() << "Creating directory" << dir.absolutePath();
            if(dir.mkpath(dir.absolutePath()))
            {
                DEBUG() << "Directory created";
            }
            else
            {
                ERROR() << "Directory not created!";
                return;
            }
        }

        if(!file.open(QFile::WriteOnly))
        {
            ERROR() << "Cannot open file" << file.fileName() << "to write into!";
            return;
        }

        DEBUG() << "keymap file" << keymap.fileName() << "doesn't exists. Copying from resourses";

        QString defaultKeymapName = QString(":/defaults/keymaps/%1/default.ini").arg(classId);
        QFile res(defaultKeymapName);
        if(res.open(QIODevice::ReadOnly|QIODevice::Text))
        {
            QByteArray content = res.readAll();

            if(file.isWritable())
            {
                file.write(content);
                file.flush();
            }
            else
            {
                ERROR() << "Keymap file" << file.fileName() << "is  not writable!";
                return;
            }

        }
        else
        {
            ERROR() << "Cannot load default keymap from resourses. File:" << defaultKeymapName;
            return;
        }
    }
}

void ProfileManageImpl::loadProfileKeymap(Profile *profile)
{
    DEBUG() << "Loading keymap for profile" << profile->getName();
    QString classId = profile->getProfilePlugin()->getProfileClassId();

    QSettings keymap(QSettings::IniFormat, QSettings::UserScope, QString(CONFIG_DIR).append("/keymaps/%1/default").arg(classId));

    loadDefaultKeymapFileIfNotExists(keymap, classId);

    keymap.sync();
    QHash<QString, RC_KEY> keycode_hashes = Core::instance()->getKeycodeHashes();

    keymap.beginGroup("keymap");
    QStringList keys = keymap.allKeys();

    BrowserPlugin* browser = profile->getProfilePlugin()->browser();
    browser->clearKeyEvents();

    for(QString key: keys)
    {
        QString value = keymap.value(key).toString();
        QStringList data = value.split("|");

        int code = -1;
        int which = -1;
        bool alt = false;
        bool ctrl = false;
        bool shift = false;

        for(QString element: data)
        {
            QStringList val = element.split(":");

            if(val.length() != 2)
            {
                WARN() << "Value length for keymap element" << element << "is" << val.length();
                continue;
            }

            QString key_name = val.at(0);
            QString key_value = val.at(1);

            if(key_name == "code")
                code = QString(key_value).toInt();
            else if(key_name == "which")
                which = QString(key_value).toInt();
            else if(key_name == "alt")
                alt = (key_value == "true");
            else if(key_name == "ctrl")
                ctrl = (key_value == "true");
            else if(key_name == "shift")
                shift = (key_value == "true");
            else
                WARN() << "Undefined key" << key_name << "->" <<  key_value << "in keymap record" << value;

            if(which == -1)
                which = code;
        }

        RC_KEY keycode_value = keycode_hashes.value(key);

        if(keycode_value == RC_KEY_NO_KEY)
            WARN() << "Key value for" << key << "not found!";
        else
            browser->registerKeyEvent(keycode_value, code, which, alt, ctrl, shift);

    }
    keymap.endGroup();

    DEBUG() << "Keymap loaded";
}

bool ProfileManageImpl::removeProfile(Profile *profile)
{
    if(profile == activeProfile)
        return false;

    QFile file(profilesDir.path().append("/").append(profile->getId()).append(".ini"));
    DEBUG() << "Removing profile file" << file.fileName();
    bool is_removed = file.remove();
    emit profileRemoved(is_removed);
    return is_removed && profilesList.remove(profile);
}

void ProfileManageImpl::loadProfiles()
{

    QString profilePath = Core::instance()->settings()->value(CONFIG_PROFILES_DIR, "profiles").toString();

    QString full_profile_path = profilesDir.absolutePath().append("/").append(profilePath);

    QDir configDir(full_profile_path);

    if(!configDir.exists())
    {
        DEBUG() << "Profiles directory doesn't exist. Creating" << full_profile_path;
        bool is_created = configDir.mkpath(full_profile_path);
        DEBUG() << "Directory create result" << is_created;
    }

    qDebug() << "Looking for profiles in" << full_profile_path;
    if(!profilesDir.cd(full_profile_path))
    {
        ERROR() << "Cannot go to profiles dir" << full_profile_path;
        return;
    }

    DEBUG() << "Searching for profiles in" << profilesDir.path();

    profilesDir.setNameFilters(QStringList() << "*.ini");

    DatasourcePlugin* dsPlugin = dynamic_cast<DatasourcePlugin*>(PluginManager::instance()->getByRole(PluginRole::ROLE_DATASOURCE));

    foreach (QString fileName, profilesDir.entryList(QDir::Files | QDir::NoSymLinks | QDir::Readable))
    {
        DEBUG() << "Loading profile from" << fileName;
        QSettings s(profilesDir.path().append("/").append(fileName), QSettings::IniFormat);

        s.beginGroup("profile");
        QString classId = s.value("classid").toString();
        StbProfilePlugin* profilePlugin = ProfileManager::instance()->getProfilePluginByClassId(classId);

        if(profilePlugin == NULL)
        {
            WARN() << "Plugin for profile classid " << classId << " not found! Profile will be skipped!";
            continue;
        }

        Profile* profile = profilePlugin->createProfile(s.value("uuid").toString());
        Q_ASSERT(profile);
        profile->datasource(dsPlugin->getDatasourceForProfile(profile));
        profile->setName(s.value("name").toString());
        DEBUG() << "Profile" << profile->getName() << "loaded";

        s.endGroup();

        profilesList.insert(profile);
    }
}

Profile* ProfileManageImpl::createProfile(const QString &classId, const QString &submodel, const QString &baseName = "")
{
    DatasourcePlugin* dsPlugin = dynamic_cast<DatasourcePlugin*>(PluginManager::instance()->getByRole(PluginRole::ROLE_DATASOURCE));

    Q_ASSERT(classId != "");
    DEBUG() << baseName << classId;
    StbPlugin* stbPlugin = getProfilePluginByClassId(classId);
    Q_ASSERT(stbPlugin != NULL);
    Profile* profile = stbPlugin->createProfile();

    Q_ASSERT(profile);
    profile->setName(createUniqueName(classId, baseName));
    profile->datasource(dsPlugin->getDatasourceForProfile(profile));
    profile->setSubmodel(stbPlugin->findSubmodel(submodel));
    profile->datasource()->set("profile", "uuid", profile->getId());
    profile->datasource()->set("profile", "name", profile->getName());
    profile->datasource()->set("profile", "classid", classId);
    profile->initDefaults();

    return profile;
}

void ProfileManageImpl::registerProfileClassId(const QString &classId, StbPlugin* profilePlugin)
{
    if(!profileClasses.contains(classId))
    {
        qDebug() << "Registered" << classId << "as profile class ID";
        profileClasses.insert(classId, profilePlugin);
    }
    else
        WARN() << "Profile class ID" << classId <<  "has bee already registered!";
}

QMap<QString, StbPlugin *> ProfileManageImpl::getRegisteredClasses()
{
    return profileClasses;
}

StbPlugin *ProfileManageImpl::getProfilePluginByClassId(const QString &classId)
{
    for(auto iterator = profileClasses.begin(); iterator != profileClasses.end(); iterator++)
    {
        if(iterator.key() == classId)
            return iterator.value();
    }

    qWarning() << QString("Profile plugin '%1' not found!").arg(classId);
    return NULL;
}

Profile *ProfileManageImpl::findById(const QString &id)
{
    foreach (Profile* profile, profilesList) {
        if(id == profile->getId())
        {
            DEBUG() << "PROFILE: " << profile;
            return profile;
        }
    }

    ERROR() << QString("Profile '%1' not found!").arg(id);
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

void ProfileManageImpl::backToMainPage()
{
    if(profileStack.size() < 1)
        return;

    profileStack.resize(1);
    Profile* profile = profileStack.at(0);
    setActiveProfile(profile);
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

