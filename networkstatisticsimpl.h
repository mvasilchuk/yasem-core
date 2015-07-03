#ifndef NETWORKSTATISTICSIMPL_H
#define NETWORKSTATISTICSIMPL_H

#include "networkstatistics.h"

namespace yasem {

namespace SDK {
class Statistics;
}

class NetworkStatisticsImpl: public SDK::NetworkStatistics
{
public:
    NetworkStatisticsImpl(SDK::Statistics* statistics);
    virtual ~NetworkStatisticsImpl();

    // NetworkStatistics interface
public:
    virtual void reset();
    virtual void incTotalCount();
    virtual void intSuccessfulCount();
    virtual void incFailedCount();
    virtual void incPendingConnection();
    virtual void decPendingConnections();
    virtual void incTooSlowConnections();
    virtual quint32 totalCount() const;
    virtual quint32 successfulCount() const;
    virtual quint32 failedCount() const;
    virtual quint32 pendingConnectionsCount() const;
    virtual quint32 tooSlowConnectionsCount() const;

protected:
    SDK::Statistics* m_statistics;
    quint32 m_total_count;
    quint32 m_successful_count;
    quint32 m_failed_count;
    quint32 m_pending_connections;
    quint32 m_too_slow_connections;
};

}

#endif // NETWORKSTATISTICSIMPL_H
