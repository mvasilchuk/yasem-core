#ifndef CORE_H
#define CORE_H

#include "enums.h"
#include "logger.h"
#include "core-network.h"
#include "diskinfo.h"

#include <functional>
#include <stdio.h>

// Includes for backtrace
#if defined(Q_OS_LINUX)
#include <execinfo.h>
#elif defined(Q_OS_WIN32)
#endif // defined(Q_OS_LINUX)

#include <QObject>
#include <QSettings>
#include <QCoreApplication>
#include <QDebug>
#include <QMetaType>
#include <QThread>

#define CONFIG_DIR "yasem"
#define CONFIG_NAME "config"
#define CONFIG_PROFILES_DIR "profiles"


#define CALLSTACK_SIZE 2048

namespace yasem
{
class DiskInfo;
class Core: public QObject
{
    Q_OBJECT
public:
    static Core* setInstance(Core* inst = 0)
    {
        static Core* instance = inst;// Guaranteed to be destroyed.

        if(instance == NULL)
            instance = static_cast<Core*>(qApp->property("Core").value<QObject*>());

        Q_CHECK_PTR(instance);
        return instance;
    }

    static Core* instance()
    {
       return setInstance();
    }

    virtual QSettings* settings() = 0;
    virtual QList<DiskInfo*> disks() = 0;
    virtual CoreNetwork* network() = 0;
    virtual QThread* mainThread() = 0;
    virtual QHash<QString, RC_KEY> getKeycodeHashes() = 0;

    static void printCallStack()
    {
        #if defined(Q_OS_LINUX)
            //print call stack (needs #include <execinfo.h>)
            void* callstack[CALLSTACK_SIZE];
            int i, frames = backtrace(callstack, CALLSTACK_SIZE);
            char** strs = backtrace_symbols(callstack, frames);
            for(i = 0; i < frames; i++){
                printf("%d: %s\n", i, strs[i]);
            }
            free(strs);
        #else
            printf("[FIXME]: printCallStack() is only supported under linux.\n");
        #endif // defined(Q_OS_LINUX)
    }

protected:
    Core(){}
    Core(Core const&);

    QHash<QString, RC_KEY> keycode_hashes;

private:

    void operator=(Core const&);
signals:
    void loggerChanged(Logger* logger);
    void methodNotImplemented(const QString &name);

public slots:
    virtual void onClose() = 0;
    virtual void mountPointChanged() = 0;
};


}

#endif // CORE_H
