#ifndef CORE_H
#define CORE_H

#include "enums.h"
#include "core-network.h"
#include "diskinfo.h"

#include <functional>
#include <stdio.h>

// Includes for backtrace
#ifdef Q_OS_UNIX
#include <execinfo.h>
#endif // defined(Q_OS_UNIX)

#include <QObject>
#include <QSettings>
#include <QCoreApplication>
#include <QDebug>
#include <QMetaType>
#include <QThread>
#include <QRegularExpression>

#define CONFIG_NAME "config.ini"
#define CONFIG_PROFILES_DIR "profiles"


#define CALLSTACK_SIZE 2048

namespace yasem
{
class StorageInfo;
class YasemSettings;
class Statistics;

class Core: public QObject
{
    Q_OBJECT
    Q_ENUMS(VirtualMachine)
public:
    enum VirtualMachine {
        VM_NOT_SET, // VM value is not set yet
        VM_NONE,
        VM_UNKNOWN,
        VM_VIRTUAL_BOX,
        VM_VMWARE
    };

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

    virtual void init() = 0;

    virtual QSettings* settings() = 0;
    virtual QSettings* settings(const QString &filename) = 0;
    virtual YasemSettings* yasem_settings() = 0;

    virtual QList<StorageInfo*> storages() = 0;
    virtual CoreNetwork* network() = 0;
    virtual Statistics* statistics() = 0;
    virtual VirtualMachine getVM() = 0;

    virtual QThread* mainThread() = 0;
    virtual QHash<QString, RC_KEY> getKeycodeHashes() = 0;

    static void printCallStack()
    {
        #ifdef Q_OS_UNIX
            //print call stack (needs #include <execinfo.h>)
            void* callstack[CALLSTACK_SIZE];
            int i, frames = backtrace(callstack, CALLSTACK_SIZE);
            char** strs = backtrace_symbols(callstack, frames);

            printf("\nCallstack:\n");

            for(i = 0; i < frames; i++){
                printf("%d: %s\n", i, strs[i]);
            }
            free(strs);
        #else
            printf("[FIXME]: printCallStack() is only supported in desktop Unix-based systems\n");
        #endif // Q_OS_UNIX
    }

    virtual QString version() = 0;
    virtual QString revision() = 0;
    virtual QString compiler() = 0;
    QHash<QString, BlockDeviceInfo*> getBlockDeviceTree() {
        return block_device_tree;
    }
    virtual QString getConfigDir() const = 0;

protected:
    Core(QObject* parent): QObject(parent){}
    Core(Core const&);

    QHash<QString, RC_KEY> keycode_hashes;
    QHash<QString, BlockDeviceInfo*> block_device_tree;
    QHash<HwinfoLineTypes, QRegularExpression> hwinfo_regex_list;
private:

    void operator=(Core const&);
signals:
    void methodNotImplemented(const QString &name);

public slots:
    virtual void onClose() = 0;
    virtual void mountPointChanged() = 0;
};


}

#endif // CORE_H
