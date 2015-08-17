#include "profilemanageimpl.h"
#include "core.h"
#include "datasourcefactory.h"
#include "stbpluginobject.h"
#include "browser.h"
#include "mediaplayer.h"
#include "stbprofile.h"
#include "statistics.h"
#include "webpage.h"
#include "networkstatistics.h"
#include "datasource.h"

#include <QFile>
#include <QDir>
#include <QRegularExpression>

using namespace yasem;

ProfileManageImpl::ProfileManageImpl(QObject *parent):
    SDK::ProfileManager(parent)
{
    profilesDir = QFileInfo(SDK::Core::instance()->settings()->fileName()).absoluteDir();
}

ProfileManageImpl::~ProfileManageImpl()
{
    STUB();
    qDeleteAll(m_profiles_list);
    qDeleteAll(m_profile_classes);

}

QSet<SDK::Profile*> ProfileManageImpl::getProfiles()
{
    return m_profiles_list;
}

SDK::Profile* ProfileManageImpl::getActiveProfile()
{
    return m_active_profile;
}

void ProfileManageImpl::addProfile(SDK::Profile* profile)
{
    Q_ASSERT(profile);
    if(!m_profiles_list.contains(profile))
    {
        m_profiles_list.insert(profile);
        emit profileAdded(profile);
    }
}

void ProfileManageImpl::setActiveProfile(SDK::Profile* profile)
{
    STUB() << profile;
    if(m_active_profile)
    {
        m_active_profile->stop();
        m_active_profile->cleanApi();
    }

    Q_ASSERT(profile);
    foreach (const SDK::Profile* item, m_profiles_list) {
        if(item == profile)
        {
            m_active_profile = profile;
            SDK::Core::instance()->settings()->setValue("active_profile", profile->getId());

            qDebug() << QString("Active profile: %1").arg(profile->getName());

            SDK::Browser* browser = SDK::Browser::instance();
            if(browser)
            {
                browser->setTopWidget(SDK::Browser::TOP_WIDGET_BROWSER);
                SDK::WebPage* page = browser->getFirstPage();
                page->reset();
                SDK::StbPluginObject* stb_object = profile->getProfilePlugin();
                stb_object->initObject(page);
            }
            else
                qDebug() << "[V] No browser found!";

            loadProfileKeymap(profile);

            SDK::Core::instance()->statistics()->network()->reset();
            profile->start();

            SDK::MediaPlayer* player = SDK::MediaPlayer::instance();
            if(player && player->isInitialized())
                player->mediaStop();
            else
                qDebug() << "[V] No player found!";

            m_profiles_stack.push(profile);

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

void ProfileManageImpl::loadProfileKeymap(SDK::Profile* profile)
{
    DEBUG() << "Loading keymap for profile" << profile->getName();
    QString classId = profile->getProfilePlugin()->getProfileClassId();

    QSettings keymap(SDK::Core::instance()->getConfigDir().append("keymaps/%1/default.ini").arg(classId), QSettings::IniFormat, this);

    loadDefaultKeymapFileIfNotExists(keymap, classId);

    keymap.sync();

    SDK::GUI* gui = SDK::GUI::instance();

    keymap.beginGroup("keymap");
    QStringList keys = keymap.allKeys();

    SDK::Browser* browser = SDK::Browser::instance();
    if(browser)
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

        SDK::GUI::RcKey keycode_value = gui->getRcKeyByName(key);

        if(keycode_value == SDK::GUI::RC_KEY_NO_KEY)
            WARN() << "Key value for" << key << "not found!";
        else
            if(browser) browser->registerKeyEvent(keycode_value, code, which, alt, ctrl, shift);

    }
    keymap.endGroup();

    DEBUG() << "Keymap loaded";
}

bool ProfileManageImpl::removeProfile(SDK::Profile* profile)
{
    if(profile == m_active_profile)
        return false;

    QFile file(profilesDir.path().append("/").append(profile->getId()).append(".ini"));
    DEBUG() << "Removing profile file" << file.fileName();
    bool is_removed = file.remove();
    emit profileRemoved(is_removed);
    return is_removed && m_profiles_list.remove(profile);
}

void ProfileManageImpl::loadProfiles()
{

    QString profilePath = SDK::Core::instance()->settings()->value(CONFIG_PROFILES_DIR, "profiles").toString();

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

    foreach (QString fileName, profilesDir.entryList(QDir::Files | QDir::NoSymLinks | QDir::Readable))
    {
        DEBUG() << "Loading profile from" << fileName;
        QSettings s(profilesDir.path().append("/").append(fileName), QSettings::IniFormat);

        s.beginGroup("profile");
        QString classId = s.value("classid").toString();
        SDK::StbPluginObject* stbPlugin = ProfileManager::instance()->getProfilePluginByClassId(classId);

        if(stbPlugin == NULL)
        {
            WARN() << "Plugin for profile classid " << classId << " not found! Profile will be skipped!";
            continue;
        }

        SDK::Profile* profile = stbPlugin->createProfile(s.value("uuid").toString());
        Q_ASSERT(profile);
        profile->setName(s.value("name").toString());
        profile->setSubmodel(stbPlugin->getSubmodels().at(s.value("submodel").toInt()));
        DEBUG() << "Profile" << profile->getName() << "loaded";

        s.endGroup();

        m_profiles_list.insert(profile);
    }
}

SDK::Profile* ProfileManageImpl::createProfile(const QString &classId, const QString &submodel, const QString &baseName = "", bool overwrite = false)
{
    DEBUG() << "Creaing profile" << classId << baseName << submodel << baseName;

    SDK::StbPluginObject* stbPlugin = getProfilePluginByClassId(classId);
    SDK::Profile* profile(stbPlugin->createProfile());

    profile->setName(createUniqueName(classId, baseName, overwrite));
    profile->setSubmodel(stbPlugin->findSubmodel(submodel));
    profile->datasource()->set("profile", "uuid", profile->getId());
    profile->datasource()->set("profile", "name", profile->getName());
    profile->datasource()->set("profile", "classid", classId);
    profile->initDefaults();

    return profile;
}

void ProfileManageImpl::registerProfileClassId(const QString &classId, SDK::StbPluginObject* profilePlugin)
{
    if(!m_profile_classes.contains(classId))
    {
        qDebug() << "Registered" << classId << "as profile class ID";
        m_profile_classes.insert(classId, profilePlugin);
    }
    else
        WARN() << "Profile class ID" << classId <<  "has bee already registered!";
}

QMap<QString, SDK::StbPluginObject*> ProfileManageImpl::getRegisteredClasses()
{
    return m_profile_classes;
}

SDK::StbPluginObject* ProfileManageImpl::getProfilePluginByClassId(const QString &classId)
{
    for(auto iterator = m_profile_classes.begin(); iterator != m_profile_classes.end(); iterator++)
    {
        if(iterator.key() == classId)
            return iterator.value();
    }

    qWarning() << QString("Profile plugin '%1' not found!").arg(classId);
    return NULL;
}

SDK::Profile* ProfileManageImpl::findById(const QString &id)
{
    foreach (SDK::Profile* profile, m_profiles_list) {
        if(id == profile->getId())
        {
            DEBUG() << "PROFILE: " << profile;
            return profile;
        }
    }

    ERROR() << QString("Profile '%1' not found!").arg(id);
    return NULL;
}

SDK::Profile* ProfileManageImpl::findByName(const QString &id)
{
    foreach (SDK::Profile* profile, m_profiles_list) {
        if(id == profile->getName())
        {
            DEBUG() << "PROFILE: " << profile;
            return profile;
        }
    }

    ERROR() << QString("Profile '%1' not found!").arg(id);
    return NULL;
}

SDK::Profile* ProfileManageImpl::backToPreviousProfile()
{
    qDebug() << "profiles:" << m_profiles_stack.size();
    if(m_profiles_stack.size() < 1)
        return NULL;

    SDK::Profile* currentProfile = m_profiles_stack.pop();
    if(m_profiles_stack.size() < 1) return currentProfile;

    SDK::Profile* profile = m_profiles_stack.pop();
    Q_ASSERT(profile);
    setActiveProfile(profile);
    return profile;
}

void ProfileManageImpl::backToMainPage()
{
    if(m_profiles_stack.size() < 1)
        return;

    m_profiles_stack.resize(1);
    SDK::Profile* profile = m_profiles_stack.at(0);
    setActiveProfile(profile);
}

bool ProfileManageImpl::canGoBack()
{
    return m_profiles_stack.size() > 1;
}


QString ProfileManageImpl::createUniqueName(const QString &classId, const QString &baseName = "", bool overwrite = false)
{
    const QString newProfileName = baseName != "" ? baseName : QObject::tr("New %1 profile").arg(classId.toCaseFolded());

    QRegularExpression rx(QString("^%1(?:\\s\\#(\\d+))?$").arg(newProfileName));
    int maxIndex = 0;

    foreach(const SDK::Profile* profile, ProfileManager::instance()->getProfiles())
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

    if(overwrite)
        return newProfileName;

    return maxIndex > 1 ? newProfileName + QString(" #") + QString::number(maxIndex) : newProfileName;
}

