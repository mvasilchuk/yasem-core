#ifndef MOUNTPOINTINFO_H
#define MOUNTPOINTINFO_H


#include <QObject>

namespace yasem
{

class MountPointInfo : public QObject
{
    Q_OBJECT
public:
    explicit MountPointInfo(QObject *parent = 0);
    virtual ~MountPointInfo();

    bool mount(const QString &host, const QString &m_point, const QString &options = "", bool force = false);
    bool unmount(bool force = false);
    bool mounted();
    QString host();
    QString mountPoint();


protected:
    QString s_host;
    QString m_point;
    bool is_mounted;

};

}

#endif // MOUNTPOINTINFO_H
