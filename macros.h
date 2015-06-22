#ifndef MACROS_H
#define MACROS_H

#include "core.h"
#include "pluginmanager.h"

#include <QDebug>

namespace yasem
{
#define WARN() qWarning()
#define DEBUG() qDebug()
#define LOG() qDebug() << "$LOG$"
#define INFO() qDebug() << "$INFO$"
#undef ERROR  // There is another ERROR macro in widgdi.h
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

template <typename T, typename R>
T __get_plugin(R role)
{
    return dynamic_cast<T>(PluginManager::instance()->getByRole(role));
}

#define S1(x) #x
#define S2(x) S1(x)

#define WARN_OBJ_IS_NULL(obj, f) (WARN() << "Can't call method " #f ", because " #obj " is NULL")
#define WARN_NULL_OBJECT(obj) WARN() << "Object " #obj " is NULL";

/**
 * This method is used to call plugin methods and return a default value if plugin not found
 */
template <typename Ret, typename T, typename Fn, typename... Args>
Ret __call_method(T* obj, Fn&& fn, Ret def, Args&&... args)
{
    if(obj)
        return std::bind(fn, obj, args...)();
    return def;
}

/**
 * This method is used to call plugin methods and don't crash if plugin not found
 */
template <typename T, typename Fn, typename... Args>
void __call_method_void(T* obj, Fn&& fn, Args&&... args)
{
    if(obj)
        std::bind(fn, obj, args...)();
}

/**
 * This method is used to call plugin methods and return a default value if plugin not found
 */
template <typename T, typename Fn, typename Ret>
Ret __call_method(T* obj, Fn&& fn, Ret def)
{
    if(obj)
        return std::bind(fn, obj)();
    return def;
}

/**
 * This method is used to call plugin methods and don't crash if plugin not found
 */
template <typename T, typename Fn>
void __call_method(T* obj, Fn&& fn)
{
    if(obj)
        std::bind(fn, obj)();
}

#define CALL_METHOD_VOID_NO_ARGS(obj, method) \
    do { if(obj) __call_method(obj, method) ; else WARN_OBJ_IS_NULL(obj, method); } while(0)

#define CALL_METHOD_NO_ARGS(obj, method, ret) \
    (obj ? __call_method(obj, method, ret) : (WARN_OBJ_IS_NULL(obj, method), ret))

#define CALL_METHOD_VOID(obj, method, ...) \
    do { if(obj) __call_method_void(obj, method, __VA_ARGS__); else WARN_OBJ_IS_NULL(obj, method); } while(0)

#define CALL_METHOD(obj, method, ret, ...) \
    (obj ? __call_method_void(obj, method, ret, __VA_ARGS__) : (WARN_OBJ_IS_NULL(obj, method), ret))

#define CHECK_OR_RETURN(object, ret) \
    do { if(!object) { WARN_NULL_OBJECT(object); return ret; } } while(0)

#define CHECK_OR_RETURN_VOID(object) \
    do { if(!object) { WARN_NULL_OBJECT(object); return; } } while(0)

}

#endif // MACROS_H
