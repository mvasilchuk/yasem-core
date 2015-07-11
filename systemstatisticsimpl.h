#ifndef SYSTEMSTATISTICSIMPL_H
#define SYSTEMSTATISTICSIMPL_H

#include "systemstatistics.h"

namespace yasem
{

class OsInfoImpl: public SDK::OsInfo {
    Q_OBJECT

    // OsInfo interface
public:
    OsInfoImpl(QObject* parent);
    virtual ~OsInfoImpl();

    virtual QString name() const;
    virtual QString version() const;
    virtual QString description() const;
};

class HostInfoImpl: public SDK::HostInfo {
    Q_OBJECT
public:
    HostInfoImpl(QObject* parent);
    virtual ~HostInfoImpl();

    // HostInfo interface
public:
    virtual QString name() const;
    virtual QString ip() const;
};

class PlatformInfoImpl: public SDK::PlatformInfo {
    Q_OBJECT
public:
    PlatformInfoImpl(QObject* parent);
    virtual ~PlatformInfoImpl();

    // PlatformInfo interface
public:
    virtual Arch arch() const;
    virtual QString archName() const;
    virtual SDK::Core::VirtualMachine vm() const;
};

class SystemStatisticsImpl : public SDK::SystemStatistics
{
    Q_OBJECT
public:
    explicit SystemStatisticsImpl(QObject *parent = 0);

signals:

public slots:

    // SystemStatistics interface
public:
    virtual SDK::OsInfo *OS() const;
    virtual SDK::HostInfo *host() const;
    virtual SDK::PlatformInfo *platform() const;

protected:
    OsInfoImpl* m_os_info;
    HostInfoImpl* m_host_info;
    PlatformInfoImpl* m_platform_info;


    // SystemStatistics interface
public:
    virtual void print() const;
};

}

#endif // SYSTEMSTATISTICSIMPL_H
