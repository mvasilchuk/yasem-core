#ifndef MACROS_H
#define MACROS_H

#include "logger.h"
#include "core.h"

namespace yasem
{
    #define WARN() qWarning()
    #define DEBUG() qDebug()
    #define LOG() qDebug() << "$LOG$"
    #define INFO() qDebug() << "$INFO$"
    #define ERROR() qWarning()
    #define STUB() qDebug() << "$STUB$"
    #define WTF() qDebug() << "$WTF$"
    #define NOT_IMPLEMENTED() << "$NI$"
    #define FIX_ME() << "$FIX$"
}

#endif // MACROS_H
