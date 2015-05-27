#ifndef NETWORKSTATISTICS
#define NETWORKSTATISTICS

#include <QObject>
#include <QHash>

namespace yasem {

class NetworkStatistics: public QObject
{
    Q_OBJECT
public:
    NetworkStatistics(QObject* parent): QObject(parent){}

    virtual void reset() = 0;
    virtual void incTotalCount() = 0;
    virtual void intSuccessfulCount() = 0;
    virtual void incFailedCount() = 0;
    virtual void incPendingConnection() = 0;
    virtual void decPendingConnections() = 0;
    virtual void incTooSlowConnections() = 0;

    virtual quint32 totalCount() const = 0;
    virtual quint32 successfulCount() const = 0;
    virtual quint32 failedCount() const = 0;
    virtual quint32 pendingConnectionsCount() const = 0;
    virtual quint32 tooSlowConnectionsCount() const = 0;

signals:
    void reseted();
    void totalCountIncreased();
    void successfulCountIncreased();
    void failedCountIncreased();
    void pendingCountIncreased();
    void pendingCountDecreased();
    void tooSlowCountIncreased();
};

}

#endif // NETWORKSTATISTICS

