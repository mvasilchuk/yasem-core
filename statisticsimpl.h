#ifndef STATISTICSIMPL_H
#define STATISTICSIMPL_H

#include "statistics.h"

#include <QNetworkAccessManager>

namespace yasem
{

class StatisticsImpl: public SDK::Statistics
{
public:
    StatisticsImpl(QObject* parent);
    virtual ~StatisticsImpl();

    virtual SDK::NetworkStatistics *network() const;
    virtual SDK::SystemStatistics *system() const;
protected:
    SDK::NetworkStatistics* m_network;
    SDK::SystemStatistics* m_system_statistics;

};

}

#endif // STATISTICSIMPL_H
