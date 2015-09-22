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
#include <QFileInfoList>
#if QT_VERSION >= 0x050400
#include <QStorageInfo>
#endif

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

    parseCommandLineArgs();
    checkCmdLineArgs();

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
    qDeleteAll(m_disks);
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
    m_disks.clear();
    int counter = 1;

#if QT_VERSION >= 0x050400
    QList<QStorageInfo> drive_list = QStorageInfo::mountedVolumes();

    for(const QStorageInfo &drive: drive_list)
    {
        SDK::StorageInfo* info = new SDK::StorageInfo();
        info->index = counter;
        info->blockDevice = drive.device();
        info->mountPoint = drive.rootPath();
        info->size = drive.bytesTotal();
        info->available = drive.bytesAvailable();
        info->used = info->size - info->available;
        if(info->size > 0)
            info->percentComplete = ((double)info->used / info->size) * 100;
        else
            info->percentComplete = 0;

        info->model = drive.displayName();

        m_disks.append(info);

        DEBUG() << info->toString();

        counter++;
    }
#else
#if defined(Q_OS_LINUX) || defined(Q_OS_DARWIN)
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
#endif // defined(Q_OS_LINUX) || defined(Q_OS_DARWIN)
#endif
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

#if defined(Q_OS_LINUX) || defined(Q_OS_DARWIN)

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
                        block_device->m_index = matcher.captured(1).toInt();
                        break;
                    }
                    case SDK::HwinfoLineTypes::HARDWARE_CLASS: {
                        QString hw_class_name = matcher.captured(1);
                        if(hw_class_name == "disk")
                            block_device->m_hardware_type = SDK::BlockDeviceType::DEVICE_TYPE_DISK;
                        else if(hw_class_name == "partition")
                            block_device->m_hardware_type = SDK::BlockDeviceType::DEVICE_TYPE_PARTITION;
                        else if(hw_class_name == "cdrom")
                            block_device->m_hardware_type = SDK::BlockDeviceType::DEVICE_TYPE_CD_ROM;
                        else
                            block_device->m_hardware_type = SDK::BlockDeviceType::DEVICE_TYPE_UNKNOWN;
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
        if(block_device->m_hardware_type == SDK::DEVICE_TYPE_DISK)
        {
            block_device_tree.insert(block_device->unique_id, block_device);
        }
        else if(block_device->m_hardware_type == SDK::DEVICE_TYPE_PARTITION)
        {
            block_device_tree.value(block_device->parent_id)->children.append(block_device);
        }
    }
#endif // defined(Q_OS_LINUX) || defined(Q_OS_DARWIN)

}

void CoreImpl::checkCmdLineArgs()
{
    if(m_cmd_line_args.contains("--help") || m_cmd_line_args.contains("-h"))
    {
        printHelp();
    }
}

void CoreImpl::printHelp()
{
    const int width = 32;
    INFO() << "Yasem (" << qPrintable(version()) << ")";
    INFO() << "Available command line arguments:";
    INFO() << "    "
           << qPrintable(QString("--help or -h").leftJustified(width, ' '))
           << "Print this help.";
    INFO() << "    "
           << qPrintable(QString("--color").leftJustified(width, ' '))
           << "Colorize output (Linux/OS X only).";
    INFO() << "    "
           << qPrintable(QString("--fullscreen").leftJustified(width, ' '))
           << "Run application in full screen mode.";
    INFO() << "    "
           << qPrintable(QString("--developer-tools").leftJustified(width, ' '))
           << "Open developer tools on startup.";
    INFO() << "    "
           << qPrintable(QString("--verbose").leftJustified(width, ' '))
           << "Add some more debug output if app built in release mode (Linux/OS X).";
    INFO() << "    "
           << qPrintable(QString("--log=<file name>").leftJustified(width, ' '))
           << "Write log into a file.";
    INFO() << "    "
           << qPrintable(QString("--window-size=<size or auto>").leftJustified(width, ' '))
           << "Set window size to WIDTHxHEIGHT (e.g. 1920x1080) or auto (fill screen). To fill the screen use with --fullscreen option.";

    exit(0);
}

QList<SDK::StorageInfo *> CoreImpl::storages()
{
    return m_disks;
}

SDK::CoreNetwork* CoreImpl::network() const
{
    return m_network;
}

QThread* CoreImpl::mainThread()
{
    return this->thread();
}

QString CoreImpl::version() const
{
    return MODULE_VERSION;
}

QString CoreImpl::revision() const
{
    return GIT_VERSION;
}

QString CoreImpl::compiler() const
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


SDK::Config* yasem::CoreImpl::yasem_settings() const
{
    Q_ASSERT(m_yasem_settings);
    return m_yasem_settings;
}


SDK::Statistics *CoreImpl::statistics() const
{
    Q_ASSERT(m_statistics);
    return m_statistics;
}


void yasem::CoreImpl::init()
{
    m_yasem_settings->load();
    statistics()->system()->print();
}


void yasem::CoreImpl::parseCommandLineArgs()
{
    m_cmd_line_args.clear();
    for(const QString& arg: qApp->arguments())
    {
        QStringList data = arg.split("=");
        if(data.length() == 1)
            m_cmd_line_args.insert(data[0], "");
        else
        {
            QString name = data[0];
            const QString& value = arg.right(arg.length() - name.length() - 1);
            m_cmd_line_args.insert(name, value);
        }
    }
}

QHash<QString, QString> yasem::CoreImpl::arguments() const
{
    return m_cmd_line_args;
}
