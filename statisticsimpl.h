#ifndef STATISTICSIMPL_H
#define STATISTICSIMPL_H

#include "statistics.h"
#include "networkstatisticsimpl.h"

namespace yasem
{

class StatisticsImpl: public SDK::Statistics
{
public:
    StatisticsImpl(QObject* parent);
    virtual ~StatisticsImpl();


    virtual SDK::NetworkStatistics *network();
protected:
    SDK::NetworkStatistics* m_network;

    // Statistics interface
public:
    virtual void reset();
    virtual void print();
};

}

#endif // STATISTICSIMPL_H
