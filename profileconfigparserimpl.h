#ifndef PROFILECONFIGPARSERIMPL_H
#define PROFILECONFIGPARSERIMPL_H

#include "profile_config_parser.h"

namespace yasem {

class ProfileConfigParserImpl : public SDK::ProfileConfigParser
{

public:
    ProfileConfigParserImpl();

    // ProfileConfigParser interface
public:
    SDK::ProfileConfiguration parseOptions(SDK::ProfileConfiguration &config, const QByteArray &data);

protected:
    SDK::ProfileConfigGroup parseGroup(SDK::ProfileConfigGroup &group, const QJsonObject &data);
    SDK::ConfigOption parseOption(SDK::ConfigOption &option, const QJsonObject &data);
    QHash<QString, QString> parseSubOptions(const QJsonObject &data);
};
}



#endif // PROFILECONFIGPARSERIMPL_H
