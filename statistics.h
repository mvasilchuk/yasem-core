#ifndef STATISTICS
#define STATISTICS

#include <QObject>
#include <QList>

namespace yasem
{

class NetworkStatistics;

class Statistics: public QObject
{
    Q_OBJECT
public:
    Statistics(QObject* parent): QObject(parent) {}

    virtual void reset() = 0;
    virtual void print() = 0;
    virtual NetworkStatistics* network() = 0;

};

}

#endif // STATISTICS

