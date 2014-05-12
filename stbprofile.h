#ifndef PROFILE_H
#define PROFILE_H

#include "datasourceplugin.h"
#include "pluginmanager.h"
#include "stbplugin.h"
#include "profileconfig.h"

#include <QUrl>
#include <QUuid>
#include <QSize>
#include <QJsonDocument>

static const QString CONFIG_SUBMODEL = "submodel";
static const QString CONFIG_PORTAL_SIZE = "portal_size";
static const QString CONFIG_PORTAL_URL = "portal";

namespace yasem
{
class Profile
{

public:
    enum ProfileFlag {
        NORMAL = 0,
        HIDDEN = 1,
    };

    explicit Profile(StbPlugin* profilePlugin, const QString &id)
    {
        this->profilePlugin = profilePlugin;
        this->id = id;
        this->flags = NORMAL;

        profileConfig.add(ProfileConfig::Option(DB_TAG_PROFILE, "name", "", "Profile name"));
    }

    virtual ~Profile(){}

    virtual void start() = 0;
    virtual void stop() = 0;
    void setName(const QString &name) { this->name = name; }
    QString getName() {
        if(name == "") name = getId();
        return name;
    }
    QString getId() {
        if(id == "") id = QUuid::createUuid().toString().mid(1, 34); //Remove braces, full length is 36
        return id;
    }
    void setId(const QString &id) { this->id = id; }
    StbPlugin* getProfilePlugin()
    {
        return this->profilePlugin;
    }

    Datasource* datasource() {
        return datasourceObj;
    }

    void datasource(Datasource* datasource){
        this->datasourceObj = datasource;
    }

    QString getImage() { return this->image; }
    void setImage(const QString &path) { this->image = path; }

    virtual void initDefaults() = 0;

    virtual void configureKeyMap() = 0;

    QHash<QString, QString> userAgents;
    QHash<QString, QSize> portalResolutions;
    QHash<QString, QSize> videoResolutions;

    bool hasFlag(ProfileFlag flag)
    {
        return (flags & flag) == flag;
    }

    void addFlag(ProfileFlag flag)
    {
        this->flags = (ProfileFlag)(this->flags | flag);
    }

    QString get(const QString &name, const QString &defaultValue = "") { return datasource()->get(DB_TAG_PROFILE, name, defaultValue); }
    bool set(const QString &name, const QString &value) { return datasource()->set(DB_TAG_PROFILE, name, value); }

    int get(const QString &name, int defaultValue = 0) { return datasource()->get(DB_TAG_PROFILE, name, defaultValue); }
    bool set(const QString &name, int value) { return datasource()->set(DB_TAG_PROFILE, name, value); }

    ProfileConfig& getProfileConfig() { return profileConfig; }

    bool saveJsonConfig(const QString& jsonConfig)
    {
        Q_ASSERT(datasource());
        QJsonArray arr = QJsonDocument::fromJson(jsonConfig.toUtf8()).array();
        foreach(QJsonValue item, arr)
        {
            QJsonObject obj = item.toObject();
            QStringList names = obj.value("name").toString().split("/");
            Q_ASSERT(names.length() == 2);

            QJsonValue value = obj.value("value");
            qDebug() << "saving" << names.at(1) << " -> " << value;
            datasource()->set(names.at(0), names.at(1), value.toVariant().toString());

        }

        setName(get("name", name));

        return true;
    }

    virtual QString portal() = 0;

protected:
    QString id;
    QString name;
    ProfileFlag flags;
    QString image;
    StbPlugin* profilePlugin;
    QString subModel;
    Datasource* datasourceObj;
    ProfileConfig profileConfig;

signals:

public slots:

friend class StbPlugin;
friend class ProfileManager;

};

}

#endif // PROFILE_H
