#ifndef DISKINFO_H
#define DISKINFO_H

#include <QObject>
#include <QString>
#include <QStringList>

namespace yasem {
    class DiskInfo {
    public:
        QString blockDevice;
        QString mountPoint;
        int percentComplete;
        qint64 size;
        qint64 used;
        qint64 available;
        QString model;
        QString vendor;

        QString toString();

    };

    enum ConnectionInterface {
        NONE,
        IDE,
        SATA
    };

    enum BlockDeviceType {
        DEVICE_TYPE_UNKNOWN,
        DEVICE_TYPE_DISK,
        DEVICE_TYPE_PARTITION,
        DEVICE_TYPE_CD_ROM
    };

    enum HwinfoLineTypes {
        TITLE,
        HARDWARE_CLASS,
        UNIQUE_ID,
        PARENT_ID,
        SYS_FS_ID,
        MODEL,
        VENDOR,
        DEVICE,
        REVISION,
        DEVICE_FILE
    };

    class BlockDeviceInfo: public QObject
    {
        Q_OBJECT
    public:
        BlockDeviceInfo(QObject* parent = 0): QObject(parent) {
            index = -1;
            conn_interface = ConnectionInterface::NONE;
            hardware_type = DEVICE_TYPE_UNKNOWN;
        }
        virtual ~BlockDeviceInfo(){};
        int index;
        ConnectionInterface conn_interface;
        BlockDeviceType hardware_type;
        QString unique_id;
        QString parent_id;
        QString sys_fs_id;
        QString model;
        QString vendor;
        QString device;
        QString revision;
        QStringList drivers;
        QString device_file;

        QString toString() {
            return QString("block device: [%1, %2, %3, %4, %5, %6, %7, %8, %9]")
                    .arg(QString::number(index))
                    .arg(unique_id)
                    .arg(parent_id)
                    .arg(sys_fs_id)
                    .arg(model)
                    .arg(vendor)
                    .arg(device)
                    .arg(revision)
                    .arg(device_file);
        }

        QList<BlockDeviceInfo*> children;
    };
}



#endif // DISKINFO_H
