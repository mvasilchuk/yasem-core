
#include "coreimpl.h"
#include "pluginmanagerimpl.h"
#include "profilemanageimpl.h"
#include "loggercore.h"
#include "yasemapplication.h"

#include <QDebug>
#include <signal.h>
#include <stdio.h>

#ifdef Q_OS_LINUX
#include <unistd.h>

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
            yasem::Core::printCallStack();
            abort();
            exit(1);
        break;
        default:
            printf("APPLICATION EXITING\r\n");
            yasem::Core::printCallStack();
            break;
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

#endif //Q_OS_LINUX


using namespace yasem;

int main(int argc, char *argv[])
{

    qInstallMessageHandler(LoggerCore::MessageHandler);

    YasemApplication a(argc, argv);
    int execCode;

    #ifdef Q_OS_LINUX
    //int stdout_fd = startErrorRedirect();
    setupSignalHandlers();
    #endif //Q_OS_LINUX

    Core::setInstance(new CoreImpl());
    a.setProperty("Core", QVariant::fromValue(Core::instance()));

    ProfileManager::setInstance(new ProfileManageImpl());
    a.setProperty("ProfileManager", QVariant::fromValue(ProfileManager::instance()));


    Core::instance()->mountPointChanged();

    qDebug() << "Starting application...";

    PluginManager::setInstance(new PluginManagerImpl());
    a.setProperty("PluginManager", QVariant::fromValue(PluginManager::instance()));
    PLUGIN_ERROR_CODES listResult = PluginManager::instance()->listPlugins();
    if(listResult == PLUGIN_ERROR_NO_ERROR)
    {

        PLUGIN_ERROR_CODES initResult = PluginManager::instance()->initPlugins();
        if(initResult != PLUGIN_ERROR_NO_ERROR)
        {
            qCritical() << "Cannot initialize plugins. Error code" << initResult;
            return listResult;
        }
    }
    else
    {
        qCritical() << "Cannot list plugins. Error code" << listResult;
        return listResult;
    }

    //QObject::connect(&a, &QCoreApplication::aboutToQuit, Core::instance(), &Core::onClose);

    execCode = a.exec();
    qDebug() <<  "Closing application... code:"  << execCode;

    #ifdef Q_OS_LINUX
    //stopErrorRedirect(stdout_fd);
    #endif //Q_OS_LINUX


    return execCode;
}
