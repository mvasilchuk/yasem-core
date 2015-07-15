#include "coreimpl.h"
#include "pluginmanager.h"
#include "networkimpl.h"
#include "macros.h"
#include "configimpl.h"
#include "statisticsimpl.h"
#include "configuration_items.h"
#include "systemstatistics.h"

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
#include <QUuid>

using namespace yasem;

CoreImpl::CoreImpl(QObject *parent ):
    Core(parent),
    m_network(new NetworkImpl(this)),
    m_yasem_settings(new ConfigImpl(this)),
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

    // Save app installation id for analytics
    QString iid = m_app_settings->value("installation_id", "").toString();
    if(iid.isEmpty())
    {
        iid = QUuid::createUuid().toString();
        iid = iid.mid(1, iid.length() - 2); // Removing {}
        m_app_settings->setValue("installation_id", iid);
    }
    DEBUG() << "App installation ID" << iid;

    fillKeymapHashTable();

    // Regular expressions to extract information from hwinfo's output.
    hwinfo_regex_list.insert(SDK::HwinfoLineTypes::TITLE,           QRegularExpression("^(\\d+):\\s+(\\w+)\\s+\\d+\\.\\d+:\\s+\\d+\\s+\\w+\n"));
    hwinfo_regex_list.insert(SDK::HwinfoLineTypes::HARDWARE_CLASS,  QRegularExpression("Hardware Class:\\s+(\\w+)\n"));
    hwinfo_regex_list.insert(SDK::HwinfoLineTypes::UNIQUE_ID,       QRegularExpression("Unique ID:\\s+(.*)\n"));
    hwinfo_regex_list.insert(SDK::HwinfoLineTypes::PARENT_ID,       QRegularExpression("Parent ID:\\s+(.*)\n"));
    hwinfo_regex_list.insert(SDK::HwinfoLineTypes::SYS_FS_ID,       QRegularExpression("SysFS ID:\\s+(.*)\n"));
    hwinfo_regex_list.insert(SDK::HwinfoLineTypes::MODEL,           QRegularExpression("Model:\\s+\\\"(.*)\\\"\n"));
    hwinfo_regex_list.insert(SDK::HwinfoLineTypes::VENDOR,          QRegularExpression("Vendor:\\s+(?:.*\\s+)?\\\"(.*)\\\"\n"));
    hwinfo_regex_list.insert(SDK::HwinfoLineTypes::DEVICE,          QRegularExpression("Device:\\s+(?:.*\\s+)?\\\"(.*)\\\"\n"));
    hwinfo_regex_list.insert(SDK::HwinfoLineTypes::REVISION,        QRegularExpression("Revision:\\s+\\\"(.*)\\\"\n"));
    hwinfo_regex_list.insert(SDK::HwinfoLineTypes::DEVICE_FILE,     QRegularExpression("Device File:\\s+(/dev/\\w+)\n"));

    //mountPointChanged();

    initBuiltInSettingsGroup();
    initSettings();
}

CoreImpl::~CoreImpl()
{

}

void CoreImpl::fillKeymapHashTable()
{
    keycode_hashes["RC_KEY_NO_KEY"]         = SDK::RC_KEY_NO_KEY;

    keycode_hashes["RC_KEY_OK"]             = SDK::RC_KEY_OK;
    keycode_hashes["RC_KEY_RIGHT"]          = SDK::RC_KEY_RIGHT;
    keycode_hashes["RC_KEY_LEFT"]           = SDK::RC_KEY_LEFT;
    keycode_hashes["RC_KEY_UP"]             = SDK::RC_KEY_UP;
    keycode_hashes["RC_KEY_DOWN"]           = SDK::RC_KEY_DOWN;
    keycode_hashes["RC_KEY_PAGE_UP"]        = SDK::RC_KEY_PAGE_UP;
    keycode_hashes["RC_KEY_PAGE_DOWN"]      = SDK::RC_KEY_PAGE_DOWN;
    keycode_hashes["RC_KEY_MENU"]           = SDK::RC_KEY_MENU;
    keycode_hashes["RC_KEY_BACK"]           = SDK::RC_KEY_BACK;
    keycode_hashes["RC_KEY_REFRESH"]        = SDK::RC_KEY_REFRESH;
    keycode_hashes["RC_KEY_RED"]            = SDK::RC_KEY_RED;
    keycode_hashes["RC_KEY_GREEN"]          = SDK::RC_KEY_GREEN;
    keycode_hashes["RC_KEY_YELLOW"]         = SDK::RC_KEY_YELLOW;
    keycode_hashes["RC_KEY_BLUE"]           = SDK::RC_KEY_BLUE;
    keycode_hashes["RC_KEY_CHANNEL_PLUS"]   = SDK::RC_KEY_CHANNEL_PLUS;
    keycode_hashes["RC_KEY_CHANNEL_MINUS"]  = SDK::RC_KEY_CHANNEL_MINUS;
    keycode_hashes["RC_KEY_SERVICE"]        = SDK::RC_KEY_SERVICE;
    keycode_hashes["RC_KEY_TV"]             = SDK::RC_KEY_TV;
    keycode_hashes["RC_KEY_PHONE"]          = SDK::RC_KEY_PHONE;
    keycode_hashes["RC_KEY_WEB"]            = SDK::RC_KEY_WEB;
    keycode_hashes["RC_KEY_FRAME"]          = SDK::RC_KEY_FRAME;
    keycode_hashes["RC_KEY_VOLUME_PLUS"]    = SDK::RC_KEY_VOLUME_UP;
    keycode_hashes["RC_KEY_VOLUME_MINUS"]   = SDK::RC_KEY_VOLUME_DOWN;
    keycode_hashes["RC_KEY_REWIND"]         = SDK::RC_KEY_REWIND;
    keycode_hashes["RC_KEY_FAST_FORWARD"]   = SDK::RC_KEY_FAST_FORWARD;
    keycode_hashes["RC_KEY_STOP"]           = SDK::RC_KEY_STOP;
    keycode_hashes["RC_KEY_PLAY_PAUSE"]     = SDK::RC_KEY_PLAY_PAUSE;
    keycode_hashes["RC_KEY_PLAY"]           = SDK::RC_KEY_PLAY;
    keycode_hashes["RC_KEY_REC"]            = SDK::RC_KEY_REC;
    keycode_hashes["RC_KEY_MIC"]            = SDK::RC_KEY_MIC;
    keycode_hashes["RC_KEY_MUTE"]           = SDK::RC_KEY_MUTE;
    keycode_hashes["RC_KEY_POWER"]          = SDK::RC_KEY_POWER;
    keycode_hashes["RC_KEY_INFO"]           = SDK::RC_KEY_INFO;
    keycode_hashes["RC_KEY_NUMBER_0"]       = SDK::RC_KEY_NUMBER_0;
    keycode_hashes["RC_KEY_NUMBER_1"]       = SDK::RC_KEY_NUMBER_1;
    keycode_hashes["RC_KEY_NUMBER_2"]       = SDK::RC_KEY_NUMBER_2;
    keycode_hashes["RC_KEY_NUMBER_3"]       = SDK::RC_KEY_NUMBER_3;
    keycode_hashes["RC_KEY_NUMBER_4"]       = SDK::RC_KEY_NUMBER_4;
    keycode_hashes["RC_KEY_NUMBER_5"]       = SDK::RC_KEY_NUMBER_5;
    keycode_hashes["RC_KEY_NUMBER_6"]       = SDK::RC_KEY_NUMBER_6;
    keycode_hashes["RC_KEY_NUMBER_7"]       = SDK::RC_KEY_NUMBER_7;
    keycode_hashes["RC_KEY_NUMBER_8"]       = SDK::RC_KEY_NUMBER_8;
    keycode_hashes["RC_KEY_NUMBER_9"]       = SDK::RC_KEY_NUMBER_9;
    keycode_hashes["RC_KEY_EXIT"]           = SDK::RC_KEY_EXIT;
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
    SDK::PluginManager::instance()->deinitPlugins();
}

void CoreImpl::initBuiltInSettingsGroup()
{
    SDK::ConfigTreeGroup* appearence = new SDK::ConfigTreeGroup(CONFIG_NAME, SETTINGS_GROUP_APPEARANCE,   tr("Appearance"));
    SDK::ConfigTreeGroup* media      = new SDK::ConfigTreeGroup(CONFIG_NAME, SETTINGS_GROUP_MEDIA,        tr("Media"));
    SDK::ConfigTreeGroup* plugins    = new SDK::ConfigTreeGroup(CONFIG_NAME, SETTINGS_GROUP_PLUGINS,      tr("Plugins"));
    SDK::ConfigTreeGroup* other      = new SDK::ConfigTreeGroup(CONFIG_NAME, SETTINGS_GROUP_OTHER,        tr("Other"));

    m_yasem_settings->addBuiltInConfigGroup(appearence);
    m_yasem_settings->addBuiltInConfigGroup(media);
    m_yasem_settings->addBuiltInConfigGroup(plugins);
    m_yasem_settings->addBuiltInConfigGroup(other);
}

void CoreImpl::initSettings()
{
    SDK::ConfigTreeGroup* media = m_yasem_settings->getDefaultGroup(SETTINGS_GROUP_MEDIA);
    SDK::ConfigTreeGroup* other = m_yasem_settings->getDefaultGroup(SETTINGS_GROUP_OTHER);

    SDK::ConfigTreeGroup* video = new SDK::ConfigTreeGroup("video", tr("Video"));

    SDK::ListConfigItem* aspect_ratio = new SDK::ListConfigItem("aspect_ratio", tr("Default aspect ratio"), SDK::ASPECT_RATIO_16_9);
    QMap<QString, QVariant>& ar_list = aspect_ratio->options();

    ar_list.insert(tr("Auto"),          SDK::ASPECT_RATIO_AUTO);
    ar_list.insert(tr("1:1"),           SDK::ASPECT_RATIO_1_1);
    ar_list.insert(tr("5:4"),           SDK::ASPECT_RATIO_5_4);
    ar_list.insert(tr("4:3"),           SDK::ASPECT_RATIO_4_3);
    ar_list.insert(tr("11:8"),          SDK::ASPECT_RATIO_11_8);
    ar_list.insert(tr("14:10"),         SDK::ASPECT_RATIO_14_10);
    ar_list.insert(tr("3:2"),           SDK::ASPECT_RATIO_3_2);
    ar_list.insert(tr("14:9"),          SDK::ASPECT_RATIO_14_9);
    ar_list.insert(tr("16:10"),         SDK::ASPECT_RATIO_16_10);
    ar_list.insert(tr("16:9"),          SDK::ASPECT_RATIO_16_9);
    ar_list.insert(tr("2.35:1"),        SDK::ASPECT_RATIO_2_35_1);
    ar_list.insert(tr("20:9"),          SDK::ASPECT_RATIO_20_9);
    ar_list.insert(tr("Fill"),          SDK::ASPECT_RATIO_FILL);
    ar_list.insert(tr("Zoom in"),       SDK::ASPECT_RATIO_20_9);

    video->addItem(aspect_ratio);
    media->addItem(video);

    SDK::ConfigTreeGroup* network_statistics     = new SDK::ConfigTreeGroup(NETWORK_STATISTICS, tr("Network statistics"));
    SDK::ConfigItem* enable_network_statistics   = new SDK::ConfigItem(NETWORK_STATISTICS_ENABLED, tr("Enable statistics"), "true", SDK::ConfigItem::BOOL);
    SDK::ConfigItem* slow_request_timeout        = new SDK::ConfigItem(NETWORK_STATISTICS_SLOW_REQ_TIMEOUT,
                                                             tr("Mark request as slow if it takes, ms"), "5000", SDK::ConfigItem::INT);

    network_statistics->addItem(enable_network_statistics);
    network_statistics->addItem(slow_request_timeout);

    other->addItem(network_statistics);
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
            SDK::StorageInfo* info = new SDK::StorageInfo();
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

    for(SDK::StorageInfo* disk: m_disks)
    {
        QString device = disk->blockDevice;
        for(SDK::BlockDeviceInfo* block_device: block_device_tree)
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
        SDK::BlockDeviceInfo* block_device = new SDK::BlockDeviceInfo(this);
        for(SDK::HwinfoLineTypes index: hwinfo_regex_list.keys())
        {
            QRegularExpressionMatch matcher = hwinfo_regex_list.value(index).match(part_data);
            if(matcher.hasMatch())
            {
                switch(index) {
                    case SDK::HwinfoLineTypes::TITLE: {
                        block_device->index = matcher.captured(1).toInt();
                        break;
                    }
                    case SDK::HwinfoLineTypes::HARDWARE_CLASS: {
                        QString hw_class_name = matcher.captured(1);
                        if(hw_class_name == "disk")
                            block_device->hardware_type = SDK::BlockDeviceType::DEVICE_TYPE_DISK;
                        else if(hw_class_name == "partition")
                            block_device->hardware_type = SDK::BlockDeviceType::DEVICE_TYPE_PARTITION;
                        else if(hw_class_name == "cdrom")
                            block_device->hardware_type = SDK::BlockDeviceType::DEVICE_TYPE_CD_ROM;
                        else
                            block_device->hardware_type = SDK::BlockDeviceType::DEVICE_TYPE_UNKNOWN;
                        break;
                    }
                    case SDK::HwinfoLineTypes::UNIQUE_ID: {
                        block_device->unique_id = matcher.captured(1);
                        break;
                    }
                    case SDK::HwinfoLineTypes::PARENT_ID: {
                        block_device->parent_id = matcher.captured(1);
                        break;
                    }
                    case SDK::HwinfoLineTypes::SYS_FS_ID: {
                        block_device->sys_fs_id = matcher.captured(1);
                        break;
                    }
                    case SDK::HwinfoLineTypes::MODEL: {
                        block_device->model = matcher.captured(1);
                        break;
                    }
                    case SDK::HwinfoLineTypes::VENDOR: {
                        block_device->vendor = matcher.captured(1);
                        break;
                    }
                    case SDK::HwinfoLineTypes::DEVICE: {
                        block_device->device = matcher.captured(1);
                        break;
                    }
                    case SDK::HwinfoLineTypes::REVISION: {
                        block_device->revision = matcher.captured(1);
                        break;
                    }
                    case SDK::HwinfoLineTypes::DEVICE_FILE: {
                        block_device->device_file = matcher.captured(1);
                        break;
                    }
                }
            }
        }

        DEBUG() << "block device" << block_device->toString();
        if(block_device->hardware_type == SDK::DEVICE_TYPE_DISK)
        {
            block_device_tree.insert(block_device->unique_id, block_device);
        }
        else if(block_device->hardware_type == SDK::DEVICE_TYPE_PARTITION)
        {
            block_device_tree.value(block_device->parent_id)->children.append(block_device);
        }
    }
}

QList<SDK::StorageInfo *> CoreImpl::storages()
{
    return m_disks;
}

SDK::CoreNetwork* CoreImpl::network()
{
    return m_network;
}

QThread* CoreImpl::mainThread()
{
    return this->thread();
}

QHash<QString, SDK::RC_KEY> CoreImpl::getKeycodeHashes()
{
    return keycode_hashes;
}


QString CoreImpl::version()
{
    return MODULE_VERSION;
}


QString CoreImpl::revision()
{
    return GIT_VERSION;
}

QString CoreImpl::compiler()
{
    return __VERSION__;
}

QString CoreImpl::getConfigDir() const
{
    return m_config_dir;
}

SDK::Core::VirtualMachine CoreImpl::getVM()
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


SDK::Config *yasem::CoreImpl::yasem_settings()
{
    Q_ASSERT(m_yasem_settings);
    return m_yasem_settings;
}


SDK::Statistics *CoreImpl::statistics()
{
    Q_ASSERT(m_statistics);
    return m_statistics;
}


void yasem::CoreImpl::init()
{
    m_yasem_settings->load();
    statistics()->system()->print();
}
