#include "networkstatisticsimpl.h"
#include "statistics.h"
#include "macros.h"

using namespace yasem;

NetworkStatisticsImpl::NetworkStatisticsImpl(SDK::Statistics* statistics):
    SDK::NetworkStatistics(statistics),
    m_statistics(statistics),
    m_total_count(0),
    m_successful_count(0),
    m_failed_count(0),
    m_pending_connections(0),
    m_too_slow_connections(0)
{

}

NetworkStatisticsImpl::~NetworkStatisticsImpl()
{

}

void NetworkStatisticsImpl::print() const
{
    DEBUG() << "=============== STATISTICS ==============";
    DEBUG() << "----------------- NETWORK ---------------";
    DEBUG() << " Total requests:" <<totalCount();
    DEBUG() << " Successful requests:" << successfulCount();
    DEBUG() << " Failed requests:" << failedCount();
    DEBUG() << " Slow requests:" << tooSlowConnectionsCount();
    DEBUG() << " Pending requests:" << pendingConnectionsCount();
    DEBUG() << "-----------------------------------------";
    DEBUG() << "=========================================";
}

void yasem::NetworkStatisticsImpl::reset()
{
    m_total_count = 0;
    m_successful_count = 0;
    m_failed_count = 0;
    m_pending_connections = 0;
    m_too_slow_connections = 0;
    emit reseted();
}

void NetworkStatisticsImpl::incTotalCount()
{
    m_total_count++;
    emit totalCountIncreased();
}

void NetworkStatisticsImpl::intSuccessfulCount()
{
    m_successful_count++;
    emit successfulCountIncreased();
}

void NetworkStatisticsImpl::incFailedCount()
{
    m_failed_count++;
    emit failedCountIncreased();
}

void NetworkStatisticsImpl::incPendingConnection()
{
    m_pending_connections++;
    emit pendingCountIncreased();
}

void NetworkStatisticsImpl::decPendingConnections()
{
    Q_ASSERT(m_pending_connections > 0);
    m_pending_connections--;
    emit pendingCountDecreased();
}

void NetworkStatisticsImpl::incTooSlowConnections()
{
    m_too_slow_connections++;
    emit tooSlowCountIncreased();
}

quint32 NetworkStatisticsImpl::totalCount() const
{
    return m_total_count;
}

quint32 NetworkStatisticsImpl::successfulCount() const
{
    return m_successful_count;
}

quint32 NetworkStatisticsImpl::failedCount() const
{
    return m_failed_count;
}

quint32 NetworkStatisticsImpl::pendingConnectionsCount() const
{
    return m_pending_connections;
}

quint32 NetworkStatisticsImpl::tooSlowConnectionsCount() const
{
    return m_too_slow_connections;
}
