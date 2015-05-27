#ifndef STATISTICSIMPL_H
#define STATISTICSIMPL_H

#include "statistics.h"
#include "networkstatisticsimpl.h"

namespace yasem
{

class StatisticsImpl: public Statistics
{
public:
    StatisticsImpl(QObject* parent);
    virtual ~StatisticsImpl();


    virtual NetworkStatistics *network();
protected:
    NetworkStatistics* m_network;

    // Statistics interface
public:
    virtual void reset();
    virtual void print();
};

}

#endif // STATISTICSIMPL_H
