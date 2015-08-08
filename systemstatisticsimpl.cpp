#include "systemstatisticsimpl.h"
#include "macros.h"

#include <QSysInfo>
#include <QHostInfo>

using namespace yasem;

SystemStatisticsImpl::SystemStatisticsImpl(QObject *parent) :
    SystemStatistics(parent),
    m_os_info(new OsInfoImpl(this)),
    m_host_info(new HostInfoImpl(this)),
    m_platform_info(new PlatformInfoImpl(this))
{

}

SDK::OsInfo *SystemStatisticsImpl::OS() const
{
    return m_os_info;
}

SDK::HostInfo *SystemStatisticsImpl::host() const
{
    return m_host_info;
}

SDK::PlatformInfo *SystemStatisticsImpl::platform() const
{
    return m_platform_info;
}



OsInfoImpl::OsInfoImpl(QObject* parent):
    SDK::OsInfo(parent)
{

}

OsInfoImpl::~OsInfoImpl()
{

}

QString OsInfoImpl::name() const
{
#if (QT_VERSION >= 0x050400)
    return QSysInfo::productType();
#else
    return "-- old qt version --";
#endif
}

QString OsInfoImpl::version() const
{
#if defined(Q_OS_WINDOWS)
    switch(QSysInfo::WindowsVersion)
    {
        case QSysInfo::WV_NT:           return "4.0";
        case QSysInfo::WV_2000:         return "2000";
        case QSysInfo::WV_XP:           return "xp";
        case QSysInfo::WV_2003:         return "2003";
        case QSysInfo::WV_VISTA:        return "vista";
        case QSysInfo::WV_WINDOWS7:     return "7";
        case QSysInfo::WV_WINDOWS8:     return "8";
        case QSysInfo::WV_WINDOWS8_1:   return "8.1";
        case QSysInfo::WV_WINDOWS10:    return "10";
        default:                        return QString("unknown:%1").arg(QSysInfo::WindowsVersion);
    }
#elseif defined(Q_OS_DARWIN)
    switch(QSysInfo::MacintoshVersion)
    {
        default: return QString("unknown:%1").arg(QSysInfo::MacintoshVersion);
    }
#else
#if (QT_VERSION >= 0x050400)
    return QSysInfo::kernelVersion();
#else
    return  "-- old qt version --";
#endif //(QT_VERSION >= 0x05040)
#endif
}

QString OsInfoImpl::description() const
{
#if (QT_VERSION >= 0x050400)
    return QSysInfo::prettyProductName();
#else
    return  "-- old qt version --";
#endif //(QT_VERSION >= 0x05040)
}


HostInfoImpl::HostInfoImpl(QObject *parent):
    HostInfo(parent)
{

}

HostInfoImpl::~HostInfoImpl()
{

}

QString yasem::HostInfoImpl::name() const
{
    return QHostInfo::localHostName();
}

QString yasem::HostInfoImpl::ip() const
{
    return "";
}

PlatformInfoImpl::PlatformInfoImpl(QObject *parent):
    PlatformInfo(parent)
{

}

PlatformInfoImpl::~PlatformInfoImpl()
{

}

SDK::PlatformInfo::Arch yasem::PlatformInfoImpl::arch() const
{
    return PlatformInfo::UNKNOWN;
}

QString yasem::PlatformInfoImpl::archName() const
{
    return QSysInfo::currentCpuArchitecture();
}

SDK::Core::VirtualMachine yasem::PlatformInfoImpl::vm() const
{
    return SDK::Core::instance()->getVM();
}

void yasem::SystemStatisticsImpl::print() const
{
    LOG() << "System information:";
    LOG() << "OS:";
    LOG() << "    " << qPrintable(OS()->name());
    LOG() << "    " << qPrintable(OS()->version());
    LOG() << "    " << qPrintable(OS()->description());
    LOG() << "Platform:";
    LOG() << "    " << qPrintable(platform()->archName());
    LOG() << "Host:";
    LOG() << "    " << qPrintable(host()->name());
}
