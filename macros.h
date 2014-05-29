#ifndef MACROS_H
#define MACROS_H

#include "logger.h"
#include "core.h"

namespace yasem
{

    #define LOG(msg) Logger::log(this, msg)
    #define DEBUG(msg) DEBUG_THIS(this, msg)
    #define DEBUG_THIS(sender, msg) Logger::debug(sender, msg)
    #define INFO(msg) Logger::info(this, msg)
    #define ERROR(msg) Logger::error(this, msg)
    #define STUB_WITH_LIST(list) STUB_WITH_PARAMS((list).join(", "))
    #define STUB_WITH_PARAMS(params) Logger::stub(this, QString(Q_FUNC_INFO).append(": [").append(QVariant::fromValue(params).toString()).append("]"))
    #define STATIC_STUB() Logger::stub(NULL, Q_FUNC_INFO)
    #define STUB() Logger::stub(this, Q_FUNC_INFO)
    #define STUB_FINISHED STUB_WITH_PARAMS("--! finished !--")


    #define BUG(msg) Logger::bug(this, msg)
    #define WARN(msg) Logger::warn(this, msg)
    #define FATAL(msg) Logger::fatal(this, msg)

}

#endif // MACROS_H
