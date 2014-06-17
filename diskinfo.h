#ifndef DISKINFO_H
#define DISKINFO_H

#include <QString>

namespace yasem {
    class DiskInfo {
    public:
        QString blockDevice;
        QString mountPoint;
        int percentComplete;
        qint64 size;
        qint64 used;
        qint64 available;


        QString toString();

    };
}



#endif // DISKINFO_H
