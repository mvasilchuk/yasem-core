
#include "coreimpl.h"
#include "pluginmanagerimpl.h"
#include "profilemanageimpl.h"
#include "loggercore.h"
#include "yasemapplication.h"
#include "profileconfigparserimpl.h"

#include "crashhandler.h"

#include <signal.h>
#include <stdio.h>

#include <QDebug>
#include <QPalette>
#include <QStyleFactory>

#ifdef Q_OS_LINUX
#ifndef Q_OS_ANDROID
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

void setupPalette()
{
    qDebug() << "Setting up dark palette...";
    qApp->setStyle(QStyleFactory::create("Fusion"));

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53,53,53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25,25,25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53,53,53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    qApp->setPalette(darkPalette);

    qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
}

using namespace yasem;
int main(int argc, char *argv[])
{
#ifdef USE_BREAKPAD
    Breakpad::CrashHandler::instance()->init();
#endif //USE_BREAKPAD
    QString appPath = QFileInfo(argv[0]).dir().path();
    QStringList paths = QCoreApplication::libraryPaths();
    paths.append(appPath);
    paths.append(appPath.append("/libs"));
    paths.append(appPath.append("/plugins"));

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

    #ifdef Q_OS_LINUX
    #ifndef Q_OS_ANDROID
    //int stdout_fd = startErrorRedirect();
    //setupSignalHandlers();
    #endif
    #endif //Q_OS_LINUX

    Core::setInstance(new CoreImpl(qApp));
    a.setProperty("Core", QVariant::fromValue(Core::instance()));

    setupPalette();

    qDebug() << "Library paths: " << QCoreApplication::libraryPaths();

    ProfileManager::setInstance(new ProfileManageImpl());
    a.setProperty("ProfileManager", QVariant::fromValue(ProfileManager::instance()));

    Core::instance()->mountPointChanged();

    qDebug() << "Starting application...";

    PluginManager::setInstance(new PluginManagerImpl());
    a.setProperty("PluginManager", QVariant::fromValue(PluginManager::instance()));

#ifndef STATIC_BUILD
    PluginErrorCodes listResult = PluginManager::instance()->listPlugins();
#else
    PLUGIN_ERROR_CODES listResult = PLUGIN_ERROR_NO_ERROR;
#endif
    if(listResult == PLUGIN_ERROR_NO_ERROR)
    {
        PluginErrorCodes initResult = PluginManager::instance()->initPlugins();
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

    qApp->setQuitOnLastWindowClosed(true);

    execCode = a.exec();
    qDebug() <<  "Closing application... code:"  << execCode;

    #ifdef Q_OS_LINUX
    //stopErrorRedirect(stdout_fd);
    #endif //Q_OS_LINUX

    return execCode;
}
