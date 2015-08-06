#include "mountpointinfo.h"
#include "macros.h"

#include <QProcess>
#include <QTemporaryDir>

using namespace yasem;

MountPointInfo::MountPointInfo(QObject *parent) :
    QObject(parent)
{
    is_mounted = false;
    s_host = "-";
    m_mount_point = "-";
}

MountPointInfo::~MountPointInfo()
{
    this->unmount();
}

bool MountPointInfo::mount(const QString &host, const QString &mount_point, const QString &options, bool force)
{
    is_mounted = true;
    #ifdef Q_OS_UNIX
    DEBUG() << "Mounting" << host << "to" << mount_point << is_mounted;
    if(is_mounted && !force)
    {
        WARN() << "Share" << this->s_host << "is already mounted to" << this->m_mount_point;
        return true;
    }

    this->s_host = host;
    this->m_mount_point = mount_point;

    QDir tmpdir(mount_point);
    if(!tmpdir.exists())
    {
        if(!tmpdir.mkpath(tmpdir.path()))
            WARN() << "Cannot create directory" << tmpdir.path();
    }

    QProcess cmd;
    QStringList params;

    params << "/usr/bin/env";
    params << "mount";
    params << "-t";
    params << "cifs";
    params << "-o";
    //params << QString("username=%1,password=%2").arg("guest").arg("");
    if(options.length() > 0)
        params << options;
    else
        params << "guest";
    params << host;
    params << mount_point;

    cmd.start("sudo", params);

    if (!cmd.waitForStarted())
    {
        ERROR() << "Cannot mount point" << s_host << "to" << mount_point;
        is_mounted = false;
    }

    if(!cmd.waitForFinished())
    {
        ERROR() << "Not finished";
    }
    //DEBUG() << params;
    DEBUG() << cmd.readAllStandardError() << cmd.readAllStandardOutput();

    cmd.close();
    cmd.kill();

    #else
    ONLY_SUPPORTED_ON_UNIX;
    #endif //Q_OS_UNIX

    return is_mounted;
}

bool MountPointInfo::unmount(bool force)
{
    #ifdef Q_OS_UNIX
    if(!this->is_mounted && !force)
    {
        WARN() << "Share" << this->s_host << "is not mounted to" << this->m_mount_point;
        return false;
    }

    QProcess cmd;
    QStringList params;
    params << "umount";
    params << this->m_mount_point;

    cmd.start("sudo", params);

    is_mounted = false;
    if (!cmd.waitForStarted())
    {
        ERROR() << "Cannot unmount point" << s_host << "from" << m_mount_point;
        is_mounted = true;
    }

    cmd.waitForFinished();

    DEBUG() << "unmount result" << cmd.readAllStandardError() << cmd.readAllStandardOutput();

    cmd.close();
    cmd.kill();
    #else
    ONLY_SUPPORTED_ON_UNIX;
    #endif //Q_OS_UNIX

    return true;
}

bool MountPointInfo::mounted()
{
    return is_mounted;
}

QString MountPointInfo::host()
{
    return s_host;
}

QString MountPointInfo::mountPoint()
{
    return m_mount_point;
}
