#ifndef PROFILE_CONFIG_PARSER_H
#define PROFILE_CONFIG_PARSER_H

#include <QString>
#include <QList>
#include <QVariant>
#include <QHash>

namespace yasem
{

class ConfigOption {
public:
    QString tag;
    QString name;
    QString defaultValue;
    QString type;
    QString title;
    QString comment;
    QHash<QString, QString> options;

    ConfigOption(){}

    ConfigOption(const QString &tag,
                 const QString &name,
                 const QString &title,
                 const QString &defaultValue,
                 const QString &type = "string",
                 const QString &comment = "",
                 const QHash<QString, QString> &options = QHash<QString, QString>())
    {
        this->tag = tag;
        this->name = name;
        this->type = type;
        this->title = title;
        this->defaultValue = defaultValue;
        this->type = type;
        this->comment = comment;
        this->options = options;
    }
};

class ProfileConfigGroup {
public:
    QString title;
    QList<ConfigOption> options;

    ProfileConfigGroup() {}

    ProfileConfigGroup(const QString &title, const QList<ConfigOption> &options = QList<ConfigOption>())
    {
        this->title = title;
        this->options = options;
    }
};

class ProfileConfiguration {
public:
    QList<ProfileConfigGroup> groups;
};

class ProfileConfigParser {
public:
    virtual ProfileConfiguration parseOptions(ProfileConfiguration &config, const QByteArray &data) = 0;
};
}

#endif // PROFILE_CONFIG_PARSER_H
