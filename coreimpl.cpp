#include "coreimpl.h"
#include "pluginmanager.h"
#include "networkimpl.h"
#include "macros.h"
#include "yasemsettingsimpl.h"
#include "statisticsimpl.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif //Q_OS_WIN

#include <QDebug>
#include <QProcess>
#include <QRegularExpression>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QMetaEnum>

using namespace yasem;

CoreImpl::CoreImpl(QObject *parent ):
    Core(parent),
    m_network(new NetworkImpl(this)),
    m_yasem_settings(new YasemSettingsImpl(this)),
    m_statistics(new StatisticsImpl(this)),
    m_detected_vm(VM_NOT_SET)
{
    Q_UNUSED(parent)
    setObjectName("Core");

#ifdef USE_OSX_BUNDLE
    LOG() << "App dir" << qApp->applicationDirPath();
    QDir config_dir(qApp->applicationDirPath().append("/../../../config"));
#else
    QDir config_dir(qApp->applicationDirPath().append("/config"));
#endif //Q_OS_DARWIN

    if(config_dir.exists())
    {
        LOG() << "Found local config directory. Trying to use it.";
        m_config_dir = config_dir.absolutePath().append("/");
    }
    else
    {
        QString app_name = QFileInfo(qAppName()).baseName();
        m_config_dir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation).append("/%1/").arg(app_name);
        config_dir.setPath(m_config_dir);

        if(!config_dir.exists())
        {
            LOG() << "Config dir " << m_config_dir << "doesn't exists. Creating...";
            if(config_dir.mkdir(m_config_dir))
                LOG() << "... successful";
            else
                WARN() << "...failed";
        }
    }

    LOG() << "Using config directory" << m_config_dir;

    m_app_settings = new QSettings(getConfigDir().append(CONFIG_NAME), QSettings::IniFormat, this);

    LOG() << qPrintable(QString("Starting YASEM... Core version: %1, rev. %2").arg(version()).arg(revision()));
    DEBUG() << "Settings directory" << QFileInfo(m_app_settings->fileName()).absoluteDir().absolutePath();
    fillKeymapHashTable();

    // Regular expressions to extract information from hwinfo's output.
    hwinfo_regex_list.insert(HwinfoLineTypes::TITLE,           QRegularExpression("^(\\d+):\\s+(\\w+)\\s+\\d+\\.\\d+:\\s+\\d+\\s+\\w+\n"));
    hwinfo_regex_list.insert(HwinfoLineTypes::HARDWARE_CLASS,  QRegularExpression("Hardware Class:\\s+(\\w+)\n"));
    hwinfo_regex_list.insert(HwinfoLineTypes::UNIQUE_ID,       QRegularExpression("Unique ID:\\s+(.*)\n"));
    hwinfo_regex_list.insert(HwinfoLineTypes::PARENT_ID,       QRegularExpression("Parent ID:\\s+(.*)\n"));
    hwinfo_regex_list.insert(HwinfoLineTypes::SYS_FS_ID,       QRegularExpression("SysFS ID:\\s+(.*)\n"));
    hwinfo_regex_list.insert(HwinfoLineTypes::MODEL,           QRegularExpression("Model:\\s+\\\"(.*)\\\"\n"));
    hwinfo_regex_list.insert(HwinfoLineTypes::VENDOR,          QRegularExpression("Vendor:\\s+(?:.*\\s+)?\\\"(.*)\\\"\n"));
    hwinfo_regex_list.insert(HwinfoLineTypes::DEVICE,          QRegularExpression("Device:\\s+(?:.*\\s+)?\\\"(.*)\\\"\n"));
    hwinfo_regex_list.insert(HwinfoLineTypes::REVISION,        QRegularExpression("Revision:\\s+\\\"(.*)\\\"\n"));
    hwinfo_regex_list.insert(HwinfoLineTypes::DEVICE_FILE,     QRegularExpression("Device File:\\s+(/dev/\\w+)\n"));

    //mountPointChanged();

    initBuiltInSettingsGroup();
    initSettings();
}

CoreImpl::~CoreImpl()
{

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
    keycode_hashes["RC_KEY_PLAY"]           = RC_KEY_PLAY;
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
    return m_app_settings;
}

QSettings *CoreImpl::settings(const QString &filename)
{
    return new QSettings(getConfigDir().append(filename), QSettings::IniFormat, this);
}

void CoreImpl::onClose()
{
    LOG() << "onClose";
    PluginManager::instance()->deinitPlugins();
}

void CoreImpl::initBuiltInSettingsGroup()
{
    ConfigTreeGroup* appearence = new ConfigTreeGroup(CONFIG_NAME, YasemSettings::SETTINGS_GROUP_APPEARANCE,tr("Appearance"));
    ConfigTreeGroup* media = new ConfigTreeGroup(CONFIG_NAME, YasemSettings::SETTINGS_GROUP_MEDIA, tr("Media"));
    ConfigTreeGroup* plugins = new ConfigTreeGroup(CONFIG_NAME, YasemSettings::SETTINGS_GROUP_PLUGINS, tr("Plugins"));
    ConfigTreeGroup* other = new ConfigTreeGroup(CONFIG_NAME, YasemSettings::SETTINGS_GROUP_OTHER, tr("Other"));

    m_yasem_settings->addBuiltInConfigGroup(appearence);
    m_yasem_settings->addBuiltInConfigGroup(media);
    m_yasem_settings->addBuiltInConfigGroup(plugins);
    m_yasem_settings->addBuiltInConfigGroup(other);
}

void CoreImpl::initSettings()
{
    ConfigTreeGroup* media = m_yasem_settings->getDefaultGroup(YasemSettings::SETTINGS_GROUP_MEDIA);
    ConfigTreeGroup* video = new ConfigTreeGroup("video", tr("Video"));

    ListConfigItem* aspect_ratio = new ListConfigItem("aspect_ratio", tr("Default aspect ratio"), ASPECT_RATIO_16_9);
    QMap<QString, QVariant>& ar_list = aspect_ratio->options();

    ar_list.insert(tr("Auto"),      ASPECT_RATIO_AUTO);
    ar_list.insert(tr("1:1"),       ASPECT_RATIO_1_1);
    ar_list.insert(tr("5:4"),       ASPECT_RATIO_5_4);
    ar_list.insert(tr("4:3"),       ASPECT_RATIO_4_3);
    ar_list.insert(tr("11:8"),      ASPECT_RATIO_11_8);
    ar_list.insert(tr("14:10"),     ASPECT_RATIO_14_10);
    ar_list.insert(tr("3:2"),       ASPECT_RATIO_3_2);
    ar_list.insert(tr("14:9"),      ASPECT_RATIO_14_9);
    ar_list.insert(tr("16:10"),     ASPECT_RATIO_16_10);
    ar_list.insert(tr("16:9"),      ASPECT_RATIO_16_9);
    ar_list.insert(tr("2.35:1"),    ASPECT_RATIO_2_35_1);
    ar_list.insert(tr("20:9"),      ASPECT_RATIO_20_9);

    video->addItem(aspect_ratio);
    media->addItem(video);
}

void CoreImpl::mountPointChanged()
{
    QProcess df;
    QRegularExpression dfRegEx("(/dev/\\w+\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+%)\\s+(.*)");

    df.start("df");
    if (!df.waitForStarted())
           return;

    if (!df.waitForFinished())
    {
        qWarning() << "Not finished!";
        return;
    }

    m_disks.clear();

    int counter = 1;

    QByteArray result;
    while(df.canReadLine())
    {
        result = df.readLine();
        QRegularExpressionMatch matcher = dfRegEx.match(result);
        if(matcher.hasMatch())
        {
            //DEBUG(QString("DRIVES: ").append(result.remove(result.length()-1, 1)));
            StorageInfo* info = new StorageInfo();
            info->index = counter;
            info->blockDevice = matcher.captured(1);
            info->size = matcher.captured(2).toLong();
            info->used = matcher.captured(3).toLong();
            info->available = matcher.captured(4).toLong();
            info->percentComplete = matcher.captured(5).toInt();
            info->mountPoint = matcher.captured(6);
            m_disks.append(info);

            DEBUG() << info->toString();

            counter++;
        }
    }

    df.close();
    df.kill();
    if(df.state() != QProcess::NotRunning)
    {
        df.terminate();
    }

    df.waitForFinished();
    buildBlockDeviceTree();

    for(StorageInfo* disk: m_disks)
    {
        QString device = disk->blockDevice;
        for(BlockDeviceInfo* block_device: block_device_tree)
        {
            if(device.startsWith(block_device->device_file))
            {
                disk->model = block_device->device;
                disk->vendor = block_device->vendor;
                break;
            }
        }
    }
}

/**
 * @brief CoreImpl::buildBlockDeviceTree
 *
 * Builds block device tree.
 *
 * This method required hwinfo to be installed.
 */
void CoreImpl::buildBlockDeviceTree()
{
    DEBUG() << "updateDisksExtraInfo";

    QProcess hwinfo;
    hwinfo.start("hwinfo", QStringList() << "--block" );
    if (!hwinfo.waitForStarted())
           return;

    if (!hwinfo.waitForFinished())
    {
        qWarning() << "Not finished!";
        return;
    }

    QString result = QString(hwinfo.readAll());

    hwinfo.close();
    hwinfo.kill();
    if(hwinfo.state() != QProcess::NotRunning)
    {
        hwinfo.terminate();
    }

    hwinfo.waitForFinished();

    // Parsing output
    QStringList data = result.split("\n\n");

    block_device_tree.clear();
    for(QString part_data: data)
    {
        BlockDeviceInfo* block_device = new BlockDeviceInfo(this);
        for(HwinfoLineTypes index: hwinfo_regex_list.keys())
        {
            QRegularExpressionMatch matcher = hwinfo_regex_list.value(index).match(part_data);
            if(matcher.hasMatch())
            {
                switch(index) {
                    case HwinfoLineTypes::TITLE: {
                        block_device->index = matcher.captured(1).toInt();
                        break;
                    }
                    case HwinfoLineTypes::HARDWARE_CLASS: {
                        QString hw_class_name = matcher.captured(1);
                        if(hw_class_name == "disk")
                            block_device->hardware_type = BlockDeviceType::DEVICE_TYPE_DISK;
                        else if(hw_class_name == "partition")
                            block_device->hardware_type = BlockDeviceType::DEVICE_TYPE_PARTITION;
                        else if(hw_class_name == "cdrom")
                            block_device->hardware_type = BlockDeviceType::DEVICE_TYPE_CD_ROM;
                        else
                            block_device->hardware_type = BlockDeviceType::DEVICE_TYPE_UNKNOWN;
                        break;
                    }
                    case HwinfoLineTypes::UNIQUE_ID: {
                        block_device->unique_id = matcher.captured(1);
                        break;
                    }
                    case HwinfoLineTypes::PARENT_ID: {
                        block_device->parent_id = matcher.captured(1);
                        break;
                    }
                    case HwinfoLineTypes::SYS_FS_ID: {
                        block_device->sys_fs_id = matcher.captured(1);
                        break;
                    }
                    case HwinfoLineTypes::MODEL: {
                        block_device->model = matcher.captured(1);
                        break;
                    }
                    case HwinfoLineTypes::VENDOR: {
                        block_device->vendor = matcher.captured(1);
                        break;
                    }
                    case HwinfoLineTypes::DEVICE: {
                        block_device->device = matcher.captured(1);
                        break;
                    }
                    case HwinfoLineTypes::REVISION: {
                        block_device->revision = matcher.captured(1);
                        break;
                    }
                    case HwinfoLineTypes::DEVICE_FILE: {
                        block_device->device_file = matcher.captured(1);
                        break;
                    }
                }
            }
        }

        DEBUG() << "block device" << block_device->toString();
        if(block_device->hardware_type == DEVICE_TYPE_DISK)
        {
            block_device_tree.insert(block_device->unique_id, block_device);
        }
        else if(block_device->hardware_type == DEVICE_TYPE_PARTITION)
        {
            block_device_tree.value(block_device->parent_id)->children.append(block_device);
        }
    }
}

QList<StorageInfo *> CoreImpl::storages()
{
    return m_disks;
}

CoreNetwork* CoreImpl::network()
{
    return m_network;
}

QThread* CoreImpl::mainThread()
{
    return this->thread();
}

QHash<QString, RC_KEY> CoreImpl::getKeycodeHashes()
{
    return keycode_hashes;
}


QString yasem::CoreImpl::version()
{
    return MODULE_VERSION;
}


QString yasem::CoreImpl::revision()
{
    return GIT_VERSION;
}

QString CoreImpl::getConfigDir() const
{
    return m_config_dir;
}

Core::VirtualMachine CoreImpl::getVM()
{
    if(m_detected_vm != VM_NOT_SET) return m_detected_vm;

#ifdef Q_OS_WIN
    if(LoadLibrary(L"VBoxHook.dll") != NULL)
    {
        m_detected_vm = VM_VIRTUAL_BOX;
    }
    else if(CreateFile(L"\\\\.\\VBoxMiniRdrDN", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL) != INVALID_HANDLE_VALUE)
    {
        m_detected_vm = VM_VIRTUAL_BOX;
    }
    else
#endif // Q_OS_WIN
        /*
     if(m_detected_vm == VM_NOT_SET)
     {
        // Blue pill, red pill detection
        // http://stackoverflow.com/questions/154163/detect-virtualized-os-from-an-application
        // I have no idea what does this code do,
        unsigned char m[2+4], rpill[] = "\x0f\x01\x0d\x00\x00\x00\x00\xc3";
           *((unsigned*)&rpill[3]) = (unsigned)m;
           ((void(*)())&rpill)();
        m_detected_vm = (m[5]>0xd0) ? VM_UNKNOWN : VM_NONE;
    }
    */

    // TODO: Write virtual machine detection
    m_detected_vm = VM_NONE;

    if(m_detected_vm != VM_NONE)
    {
        int keyEnumIndex = staticMetaObject.indexOfEnumerator("VirtualMachine");
        LOG() << "Virtual machine" << staticMetaObject.enumerator(keyEnumIndex).valueToKey(m_detected_vm) << "detected. OpenGL rendering will be disabled.";
    }

    return m_detected_vm;
}


YasemSettings *yasem::CoreImpl::yasem_settings()
{
    Q_ASSERT(m_yasem_settings);
    return m_yasem_settings;
}


Statistics *yasem::CoreImpl::statistics()
{
    Q_ASSERT(m_statistics);
    return m_statistics;
}
