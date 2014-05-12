#include "coreimpl.h"
#include "consolelogger.h"
#include "pluginmanagerimpl.h"
#include "profilemanageimpl.h"
#include "macros.h"

#include <QApplication>
#include <signal.h>
#include <stdio.h>

#ifdef Q_WS_LINUX
#include <execinfo.h>
#include <unistd.h>
#endif //Q_WS_LINUX

#ifdef Q_WS_LINUX

void printCallStack()
{
    //print call stack (needs #include <execinfo.h>)
    int callstack_size = 2048;
    void* callstack[callstack_size];
    int i, frames = backtrace(callstack, callstack_size);
    char** strs = backtrace_symbols(callstack, frames);
    for(i = 0; i < frames; i++){
        printf("%s\n", strs[i]);
    }
    free(strs);
}

void signalHandler(int signal)
{

    //print received signal
    switch(signal){
        case SIGINT: printf("SIGINT\r\n"); break;
        case SIGKILL: printf("SIGKILL\r\n"); break;
        case SIGQUIT: printf("SIGQUIT\r\n"); break;
        case SIGSTOP: printf("SIGSTOP\r\n"); break;
        case SIGTERM: printf("SIGTERM\r\n"); break;
        case SIGSEGV: printf("SIGSEGV\r\n");
            printCallStack();
            abort();
            exit(1);
        break;
        default: printf("APPLICATION EXITING\r\n"); break;
    }
    QCoreApplication::quit();
}

void setupSignalHandlers()
{
     //#ifdef QT_NO_DEBUG
    //configure app's reaction to OS signals
    struct sigaction act, oact;
    memset((void*)&act, 0, sizeof(struct sigaction));
    memset((void*)&oact, 0, sizeof(struct sigaction));
    act.sa_flags = 0;
    act.sa_handler = &signalHandler;
    sigaction(SIGINT, &act, &oact);
    sigaction(SIGKILL, &act, &oact);
    sigaction(SIGQUIT, &act, &oact);
    sigaction(SIGSTOP, &act, &oact);
    sigaction(SIGTERM, &act, &oact);
    sigaction(SIGSEGV, &act, &oact);
   // #endif
}

int startErrorRedirect()
{
    int stdout_fd = dup(STDERR_FILENO);
    freopen("/tmp/yasem-error.log", "w", stdout);
    return stdout_fd;
}

void stopErrorRedirect(int stdout_fd)
{
    fclose(stdout);
    dup2(stdout_fd, STDERR_FILENO);
    stdout = fdopen(STDERR_FILENO, "w");
    close(stdout_fd);
}

#endif //Q_WS_LINUX


using namespace yasem;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    #ifdef Q_WS_LINUX
    //int stdout_fd = startErrorRedirect();
    setupSignalHandlers();
    #endif //Q_WS_LINUX

    Core::setInstance(new CoreImpl());
    a.setProperty("Core", QVariant::fromValue(Core::instance()));

    Logger::setInstance(new ConsoleLogger(Core::instance()));
    a.setProperty("Logger", QVariant::fromValue(Logger::instance()));

    ProfileManager::setInstance(new ProfileManageImpl());
    a.setProperty("ProfileManager", QVariant::fromValue(ProfileManager::instance()));


    Core::instance()->mountPointChanged();

    Logger::debug(NULL, "Starting application...");

    PluginManager::setInstance(new PluginManagerImpl());
    a.setProperty("PluginManager", QVariant::fromValue(PluginManager::instance()));
    PLUGIN_ERROR_CODES listResult = PluginManager::instance()->listPlugins();
    if(listResult == PLUGIN_ERROR_NO_ERROR)
    {

        PLUGIN_ERROR_CODES initResult = PluginManager::instance()->initPlugins();
        if(initResult != PLUGIN_ERROR_NO_ERROR)
        {
            Logger::fatal(NULL, QString("Cannot initialize plugins. Error code %1").arg(initResult));
            return listResult;
        }
    }
    else
    {
        Logger::fatal(NULL, QString("Cannot list plugins. Error code %1").arg(listResult));
        return listResult;
    }

    //QObject::connect(&a, &QCoreApplication::aboutToQuit, Core::instance(), &Core::onClose);
    int execCode = a.exec();
    Logger::debug(NULL, QObject::trUtf8("Closing application... code: %1").arg(execCode));

    #ifdef Q_WS_LINUX
    //stopErrorRedirect(stdout_fd);
    #endif //Q_WS_LINUX
    return execCode;
}
