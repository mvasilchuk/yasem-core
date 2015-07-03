#include "profileconfigparserimpl.h"

#include "macros.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

using namespace yasem;

ProfileConfigParserImpl::ProfileConfigParserImpl()
{
    DEBUG() << "Profile paser initialized";
}


SDK::ProfileConfiguration ProfileConfigParserImpl::parseOptions(SDK::ProfileConfiguration &config, const QByteArray &data)
{
    DEBUG() << data;

    QJsonParseError *error = NULL;
    QJsonDocument doc = QJsonDocument::fromJson(data, error);

    if(doc.isNull())
    {
        WARN() << error->errorString();
        return config;
    }

    QJsonArray root = doc.array();

    for(int index = 0; index < config.groups.size(); index++)
    {
        SDK::ProfileConfigGroup group =  config.groups.at(index);
        config.groups.append(parseGroup(group, root.at(index).toObject()));
    }

    return config;
}

SDK::ProfileConfigGroup ProfileConfigParserImpl::parseGroup(SDK::ProfileConfigGroup &group, const QJsonObject &data)
{
    group.title = data.value("title").toString();
    QJsonArray options = data.value("options").toArray();
    for(int index = 0; index < options.size(); index++)
    {
        SDK::ConfigOption option = group.options.at(index);
        group.options.append(parseOption(option, options.at(index).toObject()));
    }

    return group;
}

SDK::ConfigOption ProfileConfigParserImpl::parseOption(SDK::ConfigOption &option, const QJsonObject &data)
{
    option.tag = data.value("tag").toString();
    option.name = data.value("name").toString();
    option.type = data.value("type").toString();
    option.defaultValue = data.value("default").toString();
    option.title = data.value("title").toString();
    option.comment = data.value("comment").toString();
    option.options = parseSubOptions(data.value("options").toObject());

    return option;
}

QHash<QString, QString> ProfileConfigParserImpl::parseSubOptions(const QJsonObject &data)
{
    QHash<QString, QString> subOptions;
    for(QString key: data.keys())
    {
        QString title = data.value(key).toString();
        subOptions.insert(key, title);
    }
    return subOptions;
}


