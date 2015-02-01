#ifndef MACROS_H
#define MACROS_H

#include "logger.h"
#include "core.h"

#include <QDebug>

namespace yasem
{
    #define WARN() qWarning()
    #define DEBUG() qDebug()
    #define LOG() qDebug() << "$LOG$"
    #define INFO() qDebug() << "$INFO$"
    #define ERROR() qWarning()
    #define STUB() qDebug() << "$STUB$"
    #define WTF() qDebug() << "$WTF$"
    #define NOT_IMPLEMENTED() qDebug() << "$NI$"
    #define FIXME() qDebug() << "$FIXME$"
    #define FATAL(msg)  qFatal(msg)

    #define NOT_SUPPORTED_ON_OS(os) STUB() << "[this method is not supported on" << os << "]"
    #define NOT_SUPPORTED_ON_UNIX NOT_SUPPORTED_ON_OS("unix")
    #define NOT_SUPPORTED_ON_LINUX NOT_SUPPORTED_ON_OS("linux")
    #define NOT_SUPPORTED_ON_WINDOWS NOT_SPPORTED_ON_OS("Windows")

    #define ONLY_SUPPORTED_ON(os) STUB() << "[this method is only supported on" << os << "]"
    #define ONLY_SUPPORTED_ON_UNIX ONLY_SUPPORTED_ON("unix")
    #define ONLY_SUPPORTED_ON_LINUX ONLY_SUPPORTED_ON("linux")
    #define ONLY_SUPPORTED_ON_WINDOWS ONLY_SUPPORTED_ON("Windows")
}

#endif // MACROS_H
