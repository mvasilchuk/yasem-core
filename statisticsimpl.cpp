#include "statisticsimpl.h"
#include "macros.h"

using namespace yasem;

StatisticsImpl::StatisticsImpl(QObject* parent):
    SDK::Statistics(parent),
    m_network(new NetworkStatisticsImpl(this))
{

}

StatisticsImpl::~StatisticsImpl()
{

}

SDK::NetworkStatistics *yasem::StatisticsImpl::network()
{
    return m_network;
}


void yasem::StatisticsImpl::reset()
{
    m_network->reset();
}

void yasem::StatisticsImpl::print()
{
    DEBUG() << "=============== STATISTICS ==============";
    DEBUG() << "----------------- NETWORK ---------------";
    DEBUG() << " Total requests:" << network()->totalCount();
    DEBUG() << " Successful requests:" << network()->successfulCount();
    DEBUG() << " Failed requests:" << network()->failedCount();
    DEBUG() << " Slow requests:" << network()->tooSlowConnectionsCount();
    DEBUG() << " Pending requests:" << network()->pendingConnectionsCount();
    DEBUG() << "-----------------------------------------";
    DEBUG() << "=========================================";
}
