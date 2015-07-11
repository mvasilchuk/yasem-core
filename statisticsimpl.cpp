#include "statisticsimpl.h"
#include "macros.h"
#include "networkstatisticsimpl.h"
#include "systemstatisticsimpl.h"
#include "core.h"

#include <QNetworkRequest>
#include <QUrlQuery>
#include <QNetworkAccessManager>

using namespace yasem;

StatisticsImpl::StatisticsImpl(QObject* parent):
    SDK::Statistics(parent),
    m_network(new NetworkStatisticsImpl(this)),
    m_system_statistics(new SystemStatisticsImpl(this))
{

}

StatisticsImpl::~StatisticsImpl()
{

}

SDK::NetworkStatistics *yasem::StatisticsImpl::network() const
{
    return m_network;
}

SDK::SystemStatistics *StatisticsImpl::system() const
{
    return m_system_statistics;
}
