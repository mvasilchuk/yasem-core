#ifndef PROFILECONFIGPARSERIMPL_H
#define PROFILECONFIGPARSERIMPL_H

#include "profile_config_parser.h"

namespace yasem {

class ProfileConfigParserImpl : public ProfileConfigParser
{

public:
    ProfileConfigParserImpl();

    // ProfileConfigParser interface
public:
    ProfileConfiguration parseOptions(ProfileConfiguration &config, const QByteArray &data);

protected:
    ProfileConfigGroup parseGroup(ProfileConfigGroup &group, const QJsonObject &data);
    ConfigOption parseOption(ConfigOption &option, const QJsonObject &data);
    QHash<QString, QString> parseSubOptions(const QJsonObject &data);
};
}



#endif // PROFILECONFIGPARSERIMPL_H
