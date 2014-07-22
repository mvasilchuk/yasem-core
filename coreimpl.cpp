#include "coreimpl.h"
#include "pluginmanager.h"
#include "networkimpl.h"

#include <QDebug>
#include <QProcess>
#include <QRegularExpression>
#include <QFileInfo>
#include <QDir>

using namespace yasem;

CoreImpl::CoreImpl(QObject *parent ): Core(parent)
{
    Q_UNUSED(parent)
    setObjectName("Core");
    appSettings = new QSettings(QSettings::IniFormat, QSettings::UserScope, CONFIG_DIR, CONFIG_NAME);

    DEBUG() << "Settings directory" << QFileInfo(appSettings->fileName()).absoluteDir().absolutePath();

    networkObj = new NetworkImpl(parent);
    fillKeymapHashTable();


    //mountPointChanged();
}

CoreImpl::~CoreImpl()
{
    STUB();
    delete networkObj;
}

void CoreImpl::fillKeymapHashTable()
{
    keycode_hashes["RC_KEY_NO_KEY"]         = RC_KEY_NO_KEY;

    keycode_hashes["RC_KEY_OK"]             = RC_KEY_OK;
    keycode_hashes["RC_KEY_RIGHT"]          = RC_KEY_RIGHT;
    keycode_hashes["RC_KEY_LEFT"]           = RC_KEY_LEFT;
    keycode_hashes["RC_KEY_UP"]             = RC_KEY_UP;
    keycode_hashes["RC_KEY_DOWN"]           = RC_KEY_DOWN;
    keycode_hashes["RC_KEY_PAGE_UP"]        = RC_KEY_PAGE_UP;
    keycode_hashes["RC_KEY_PAGE_DOWN"]      = RC_KEY_PAGE_DOWN;
    keycode_hashes["RC_KEY_MENU"]           = RC_KEY_MENU;
    keycode_hashes["RC_KEY_BACK"]           = RC_KEY_BACK;
    keycode_hashes["RC_KEY_REFRESH"]        = RC_KEY_REFRESH;
    keycode_hashes["RC_KEY_RED"]            = RC_KEY_RED;
    keycode_hashes["RC_KEY_GREEN"]          = RC_KEY_GREEN;
    keycode_hashes["RC_KEY_YELLOW"]         = RC_KEY_YELLOW;
    keycode_hashes["RC_KEY_BLUE"]           = RC_KEY_BLUE;
    keycode_hashes["RC_KEY_CHANNEL_PLUS"]   = RC_KEY_CHANNEL_PLUS;
    keycode_hashes["RC_KEY_CHANNEL_MINUS"]  = RC_KEY_CHANNEL_MINUS;
    keycode_hashes["RC_KEY_SERVICE"]        = RC_KEY_SERVICE;
    keycode_hashes["RC_KEY_TV"]             = RC_KEY_TV;
    keycode_hashes["RC_KEY_PHONE"]          = RC_KEY_PHONE;
    keycode_hashes["RC_KEY_WEB"]            = RC_KEY_WEB;
    keycode_hashes["RC_KEY_FRAME"]          = RC_KEY_FRAME;
    keycode_hashes["RC_KEY_VOLUME_PLUS"]    = RC_KEY_VOLUME_UP;
    keycode_hashes["RC_KEY_VOLUME_MINUS"]   = RC_KEY_VOLUME_DOWN;
    keycode_hashes["RC_KEY_REWIND"]         = RC_KEY_REWIND;
    keycode_hashes["RC_KEY_FAST_FORWARD"]   = RC_KEY_FAST_FORWARD;
    keycode_hashes["RC_KEY_STOP"]           = RC_KEY_STOP;
    keycode_hashes["RC_KEY_PLAY_PAUSE"]     = RC_KEY_PLAY_PAUSE;
    keycode_hashes["RC_KEY_REC"]            = RC_KEY_REC;
    keycode_hashes["RC_KEY_MIC"]            = RC_KEY_MIC;
    keycode_hashes["RC_KEY_MUTE"]           = RC_KEY_MUTE;
    keycode_hashes["RC_KEY_POWER"]          = RC_KEY_POWER;
    keycode_hashes["RC_KEY_INFO"]           = RC_KEY_INFO;
    keycode_hashes["RC_KEY_NUMBER_0"]       = RC_KEY_NUMBER_0;
    keycode_hashes["RC_KEY_NUMBER_1"]       = RC_KEY_NUMBER_1;
    keycode_hashes["RC_KEY_NUMBER_2"]       = RC_KEY_NUMBER_2;
    keycode_hashes["RC_KEY_NUMBER_3"]       = RC_KEY_NUMBER_3;
    keycode_hashes["RC_KEY_NUMBER_4"]       = RC_KEY_NUMBER_4;
    keycode_hashes["RC_KEY_NUMBER_5"]       = RC_KEY_NUMBER_5;
    keycode_hashes["RC_KEY_NUMBER_6"]       = RC_KEY_NUMBER_6;
    keycode_hashes["RC_KEY_NUMBER_7"]       = RC_KEY_NUMBER_7;
    keycode_hashes["RC_KEY_NUMBER_8"]       = RC_KEY_NUMBER_8;
    keycode_hashes["RC_KEY_NUMBER_9"]       = RC_KEY_NUMBER_9;
    keycode_hashes["RC_KEY_EXIT"]           = RC_KEY_EXIT;
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


QHash<QString, RC_KEY> CoreImpl::getKeycodeHashes()
{
    return keycode_hashes;
}


