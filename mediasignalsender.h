#ifndef MEDIASIGNALSENDER_H
#define MEDIASIGNALSENDER_H

#include "enums.h"

#include <QObject>

namespace yasem
{

class MediaSignalSender: public QObject
{
    Q_OBJECT
public:
    MediaSignalSender() {}
signals:
    //void mediaStatusChanged(QtAV::MediaStatus status); //explictly use QtAV::MediaStatus
    //void error(const QtAV::AVError& e); //explictly use QtAV::AVError in connection for Qt4 syntax
    void paused(bool p);
    void started();
    void stopped();
    void speedChanged(qreal speed);
    void repeatChanged(int r);
    void currentRepeatChanged(int r);
    void startPositionChanged(qint64 position);
    void stopPositionChanged(qint64 position);
    void positionChanged(qint64 position);
    void brightnessChanged(int val);
    void contrastChanged(int val);
    void saturationChanged(int val);
    void statusChanged(MediaStatus status);
};

}
#endif // MEDIASIGNALSENDER_H
