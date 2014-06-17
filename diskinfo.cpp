#include "diskinfo.h"

using namespace yasem;


QString DiskInfo::toString()
{
    return QString("disk: [%1, %2, %3, %4, %5, %6]").arg(blockDevice).arg(mountPoint).arg(size).arg(used).arg(available).arg(percentComplete);
}
