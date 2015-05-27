#include "networkstatisticsimpl.h"
#include "statistics.h"

using namespace yasem;

NetworkStatisticsImpl::NetworkStatisticsImpl(Statistics* statistics):
    NetworkStatistics(statistics),
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

void yasem::NetworkStatisticsImpl::reset()
{
    m_total_count = 0;
    m_successful_count = 0;
    m_failed_count = 0;
    m_pending_connections = 0;
    m_too_slow_connections = 0;
    emit reseted();
}

void yasem::NetworkStatisticsImpl::incTotalCount()
{
    m_total_count++;
    emit totalCountIncreased();
}

void yasem::NetworkStatisticsImpl::intSuccessfulCount()
{
    m_successful_count++;
    emit successfulCountIncreased();
}

void yasem::NetworkStatisticsImpl::incFailedCount()
{
    m_failed_count++;
    emit failedCountIncreased();
}

void yasem::NetworkStatisticsImpl::incPendingConnection()
{
    m_pending_connections++;
    emit pendingCountIncreased();
}

void yasem::NetworkStatisticsImpl::decPendingConnections()
{
    m_pending_connections--;
    Q_ASSERT(m_pending_connections >= 0);
    emit pendingCountDecreased();
}

void yasem::NetworkStatisticsImpl::incTooSlowConnections()
{
    m_too_slow_connections++;
    emit tooSlowCountIncreased();
}

quint32 yasem::NetworkStatisticsImpl::totalCount() const
{
    return m_total_count;
}

quint32 yasem::NetworkStatisticsImpl::successfulCount() const
{
    return m_successful_count;
}

quint32 yasem::NetworkStatisticsImpl::failedCount() const
{
    return m_failed_count;
}

quint32 yasem::NetworkStatisticsImpl::pendingConnectionsCount() const
{
    return m_pending_connections;
}

quint32 yasem::NetworkStatisticsImpl::tooSlowConnectionsCount() const
{
    return m_too_slow_connections;
}
