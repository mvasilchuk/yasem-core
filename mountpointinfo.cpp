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
    m_point = "-";
}

MountPointInfo::~MountPointInfo()
{
    this->unmount();
}

bool MountPointInfo::mount(const QString &host, const QString &m_point, bool force)
{
    is_mounted = true;
    #ifdef Q_OS_UNIX
    DEBUG() << "Mounting" << host << "to" << m_point << is_mounted;
    if(is_mounted && !force)
    {
        WARN() << "Share" << this->s_host << "is already mounted to" << this->m_point;
        return true;
    }

    this->s_host = host;
    this->m_point = m_point;

    QDir tmpdir(m_point);
    if(!tmpdir.exists())
    {
        if(!tmpdir.mkpath(tmpdir.path()))
            WARN() << "Cannot create directory" << tmpdir.path();
    }


    QProcess cmd;
    QStringList params;

    //sudo mount -t cifs -o username=guest,password= //ironman/ /tmp/yasem/samba/ironman/
    //sudo mount -t cifs -o guest //ironman/public /tmp/yasem/samba/ironman/

    params << "/usr/bin/env";
    params << "mount";
    params << "-t";
    params << "cifs";
    params << "-o";
    //params << QString("username=%1,password=%2").arg("guest").arg("");
    params << "guest";
    params << host;
    params << m_point;

    cmd.start("sudo", params);


    if (!cmd.waitForStarted())
    {
        ERROR() << "Cannot mount point" << s_host << "to" << m_point;
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
        WARN() << "Share" << this->s_host << "is not mounted to" << this->m_point;
        return false;
    }

    QProcess cmd;
    QStringList params;
    params << "umount";
    params << this->m_point;

    cmd.start("sudo", params);

    is_mounted = false;
    if (!cmd.waitForStarted())
    {
        ERROR() << "Cannot unmount point" << s_host << "from" << m_point;
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
    return m_point;
}
