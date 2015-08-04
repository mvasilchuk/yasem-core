
#include "coreimpl.h"
#include "pluginmanagerimpl.h"
#include "profilemanageimpl.h"
#include "loggercore.h"
#include "yasemapplication.h"
#include "profileconfigparserimpl.h"

#include "crashhandler.h"

#include <QDebug>
#include <signal.h>
#include <stdio.h>

#ifdef Q_OS_LINUX
#ifndef Q_OS_ANDROID
#include <unistd.h>

using namespace yasem;

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
            SDK::Core::printCallStack();
            abort();
        break;
        default:
            printf("APPLICATION EXITING\r\n");
            SDK::Core::printCallStack();
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

    FILE* fp;

    char* filename = (char*)"/tmp/yasem-debug.log";

    if((fp=freopen(filename, "w" ,stdout))==NULL) {
        printf("Can't open file %s\n", filename);
    }
    return stdout_fd;
}

void stopErrorRedirect(int stdout_fd)
{
    fclose(stdout);
    dup2(stdout_fd, STDERR_FILENO);
    stdout = fdopen(STDERR_FILENO, "w");
    close(stdout_fd);
}
#endif //Q_OS_ANDROID
#endif //Q_OS_LINUX

using namespace yasem;
int main(int argc, char *argv[])
{
    QString appPath = QFileInfo(argv[0]).dir().path();
    QStringList paths = QCoreApplication::libraryPaths();
    qDebug() << paths << appPath;

#ifdef Q_OS_DARWIN
#if (defined(QT_DEBUG) or (USE_SYS_LIBS))
    paths.append(appPath);
    paths.append(appPath.append("/libs"));
    paths.append(appPath.append("/plugins"));
#else
    // Release shouldn't use system libs
    paths.clear();
    paths.append(appPath.append("/../PlugIns"));
#endif

#else
    paths.append("./");
    paths.append(appPath);
    paths.append(appPath.append("/libs"));
    paths.append(appPath.append("/plugins"));
#endif

#ifdef USE_BREAKPAD
    Breakpad::CrashHandler::instance()->init();
#endif //USE_BREAKPAD


//#endif //Q_OS_DARWIN

#ifdef Q_OS_WIN
    // A patch for Windows to support LD_LIBRARY_PATH
    QString libraryPath = qgetenv("LD_LIBRARY_PATH");
    QStringList pathList = libraryPath.split(";");
    for(QString path: pathList)
        paths.append(path);
#endif

    QCoreApplication::setLibraryPaths(paths);

    qInstallMessageHandler(LoggerCore::MessageHandler);
    YasemApplication a(argc, argv);
    int execCode;

    LoggerCore::initLogFile(qApp);

    #ifdef Q_OS_LINUX
    #ifndef Q_OS_ANDROID
    //int stdout_fd = startErrorRedirect();
    //setupSignalHandlers();
    #endif
    #endif //Q_OS_LINUX

    SDK::Core* core = new CoreImpl(qApp);
    SDK::Core::setInstance(core);
    a.setProperty("Core", QVariant::fromValue(core));
    SDK::Core::instance()->init();

    qDebug() << "Library paths: " << QApplication::libraryPaths();

    SDK::ProfileManager::setInstance(new ProfileManageImpl(core));
    a.setProperty("ProfileManager", QVariant::fromValue(SDK::ProfileManager::instance()));

    SDK::Core::instance()->mountPointChanged();

    qDebug() << "Starting application...";

    SDK::PluginManager::setInstance(new PluginManagerImpl(core));
    a.setProperty("PluginManager", QVariant::fromValue(SDK::PluginManager::instance()));

#ifndef STATIC_BUILD
    SDK::PluginErrorCodes listResult = SDK::PluginManager::instance()->listPlugins();
#else
    SDK::PLUGIN_ERROR_CODES listResult = SDK::PLUGIN_ERROR_NO_ERROR;
#endif
    if(listResult == SDK::PLUGIN_ERROR_NO_ERROR)
    {
        SDK::PluginErrorCodes initResult = SDK::PluginManager::instance()->initPlugins();
        if(initResult != SDK::PLUGIN_ERROR_NO_ERROR)
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

    qApp->setQuitOnLastWindowClosed(true);
    execCode = a.exec();
    qDebug() <<  "Closing application... code:"  << execCode;

    SDK::PluginManager::instance()->deinitPlugins();

    #ifdef Q_OS_LINUX
    //stopErrorRedirect(stdout_fd);
    #endif //Q_OS_LINUX

    return execCode;
}
