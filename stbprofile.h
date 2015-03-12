#ifndef PROFILE_H
#define PROFILE_H

#include "datasourceplugin.h"
#include "profile_config_parser.h"
#include "stbsubmodule.h"
#include "stbpluginobject.h"

#include <QUrl>
#include <QUuid>
#include <QSize>
#include <QJsonDocument>
#include <QJsonArray>

static const QString CONFIG_PROFILE_NAME = "name";
static const QString CONFIG_SUBMODEL = "submodel";
static const QString CONFIG_PORTAL_SIZE = "portal_size";
static const QString CONFIG_PORTAL_URL = "portal";

namespace yasem
{

class StbPluginObject;

class Profile
{

public:
    enum ProfileFlag {
        NORMAL = 0,
        HIDDEN = 1,
    };

    explicit Profile(StbPluginObject* profilePlugin, const QString &id):
        m_profile_plugin(profilePlugin),
        m_id(id)
    {
        if(m_id == "")
            this->m_id = QUuid::createUuid().toString().mid(1, 34); //Remove braces, full length is 36
        this->flags = NORMAL;

        name = getId();

        ProfileConfigGroup group(QObject::tr("Main settings"));
        group.options.append(ConfigOption(DB_TAG_PROFILE, CONFIG_PROFILE_NAME, QObject::tr("Profile name"), QObject::tr("New profile")));

        QHash<QString, QString> models;
        QList<StbSubmodel> submodels = profilePlugin->getSubmodels();
        for(StbSubmodel model: submodels)
        {
            models.insert(model.m_id, model.m_name);
        }

        group.options.append(ConfigOption(DB_TAG_PROFILE, CONFIG_SUBMODEL, QObject::tr("Model name"), "","options", "", models));

        profileConfiguration.groups.append(group);
    }

    virtual ~Profile(){}

    virtual void start() = 0;
    virtual void stop() = 0;

    void setName(const QString &name) { this->name = name; }
    QString getName() const { return name; }
    QString getId() const { return m_id; }
    void setId(const QString &id) { this->m_id = id; }
    StbPluginObject* getProfilePlugin() const { return this->m_profile_plugin; }

    DatasourcePluginObject* datasource() const { return datasourceObj; }
    void datasource(DatasourcePluginObject* datasource){ this->datasourceObj = datasource; }

    QString getImage() const { return this->image; }
    void setImage(const QString &path) { this->image = path; }

    virtual void initDefaults() = 0;

    virtual void configureKeyMap() = 0;

    bool hasFlag(ProfileFlag flag) const { return (flags & flag) == flag; }

    void addFlag(ProfileFlag flag)
    {
        this->flags = (ProfileFlag)(this->flags | flag);
    }

    QString get(const QString &name, const QString &defaultValue = "") { return datasource()->get(DB_TAG_PROFILE, name, defaultValue); }
    bool set(const QString &name, const QString &value) { return datasource()->set(DB_TAG_PROFILE, name, value); }

    int get(const QString &name, int defaultValue = 0) { return datasource()->get(DB_TAG_PROFILE, name, defaultValue); }
    bool set(const QString &name, int value) { return datasource()->set(DB_TAG_PROFILE, name, value); }

    bool saveJsonConfig(const QString& jsonConfig)
    {
        Q_ASSERT(datasource());
        DEBUG() << jsonConfig;
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



    ProfileConfiguration &config()
    {
        return profileConfiguration;
    }


    StbSubmodel& getSubmodel() { return submodel; }
    void setSubmodel(const StbSubmodel& submodel) {
        this->submodel = submodel;
    }


protected:
    StbPluginObject* m_profile_plugin;
    QString m_id;

    StbSubmodel submodel;
    QString name;
    ProfileFlag flags;
    QString image;
    DatasourcePluginObject* datasourceObj;
    QHash<QString, QString> userAgents;
    QHash<QString, QSize> portalResolutions;
    QHash<QString, QSize> videoResolutions;
    ProfileConfiguration profileConfiguration;

signals:

public slots:

friend class StbPluginObject;
friend class ProfileManager;

};

}

#endif // PROFILE_H
