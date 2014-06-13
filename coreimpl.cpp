#include "coreimpl.h"
#include "pluginmanager.h"
#include "networkimpl.h"

#include <QDebug>
#include <QProcess>
#include <QRegularExpression>

using namespace yasem;

CoreImpl::CoreImpl(QObject *parent ): Core()
{
    Q_UNUSED(parent)
    setObjectName("Core");
    appSettings = new QSettings(QSettings::IniFormat, QSettings::UserScope, CONFIG_DIR, CONFIG_NAME);
    networkObj = new NetworkImpl(parent);

    //mountPointChanged();
}

QSettings *CoreImpl::settings()
{
    return appSettings;
}

void CoreImpl::onClose()
{
    LOG() << "onClose";
    PluginManager::instance()->deinitPlugins();
}

void CoreImpl::mountPointChanged()
{
    QProcess dfProcess;
    QRegularExpression dfRegEx("(/dev/\\w+\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+%)\\s+(.*)");

    dfProcess.start("df");
    if (!dfProcess.waitForStarted())
           return;

    if (!dfProcess.waitForFinished())
    {
        qWarning() << "Not finished!";
        return;
    }

    disksList.clear();

    QByteArray result;
    while(dfProcess.canReadLine())
    {
        result = dfProcess.readLine();
        QRegularExpressionMatch matcher = dfRegEx.match(result);
        if(matcher.hasMatch())
        {
            //DEBUG(QString("DRIVES: ").append(result.remove(result.length()-1, 1)));
            DiskInfo* info = new DiskInfo();
            info->blockDevice = matcher.captured(1);
            info->size = matcher.captured(2).toLong();
            info->used = matcher.captured(3).toLong();
            info->available = matcher.captured(4).toLong();
            info->percentComplete = matcher.captured(5).toInt();
            info->mountPoint = matcher.captured(6);
            disksList.append(info);

            DEBUG() << info->toString();
        }
    }

    dfProcess.close();
    dfProcess.kill();
    if(dfProcess.state() != QProcess::NotRunning)
    {
        dfProcess.terminate();
    }

    dfProcess.waitForFinished();
}

QList<DiskInfo *> CoreImpl::disks()
{
    return disksList;
}

CoreNetwork* CoreImpl::network()
{
    return networkObj;
}

QThread* CoreImpl::mainThread()
{
    return this->thread();
}


