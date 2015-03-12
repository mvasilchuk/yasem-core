#ifndef MEDIAPLAYERPLUGIN_H
#define MEDIAPLAYERPLUGIN_H

#include "enums.h"
#include "abstractpluginobject.h"
#include "macros.h"
#include "mediainfo.h"
#include "browserpluginobject.h"
#include "pluginmanager.h"
#include "abstractwebpage.h"

#include <QWidget>
#include <QtPlugin>


class QString;

#include <QRect>
#include <QSet>
#include <QDebug>

namespace yasem {

class MediaPlayerPluginObject: public AbstractPluginObject
{
    Q_OBJECT
public:

    explicit MediaPlayerPluginObject(Plugin* plugin, QObject* parent = NULL);

    typedef bool (*hook_function)(void);

    enum HookEvent {
        HOOK_ALL,
        BEFORE_PLAY,
        AFTER_PLAY
    };

    class Hook {
    public:
        Hook(hook_function func)
        {
            this->func = func;
        }
    protected:
        hook_function func;
        friend class MediaPlayerPluginObject;
    };

    virtual void parent(QWidget* parent) = 0;
    virtual QWidget* parent() = 0;

    virtual void widget(QWidget* videoWidget) = 0;
    virtual QWidget* widget() = 0;

    virtual bool mediaPlay(const QString &url) = 0;
    virtual bool mediaContinue() = 0;
    virtual bool mediaPause() = 0;
    virtual bool mediaStop() = 0;
    virtual bool mediaReset() = 0;

    virtual int audioPID() = 0;
    virtual void audioPID(int pid) = 0;

    virtual int loop() = 0;
    virtual void loop(int loop) = 0;

    virtual bool mute() = 0;
    virtual void mute(bool value) = 0;


    virtual int bufferLoad() = 0; //persents

    virtual qint64 position() = 0; //ms
    virtual void position(qint64 pos) = 0;

    virtual qint64 duration() = 0; // ms

    virtual int volume() = 0;
    virtual void volume(int vol) = 0;

    virtual void show() = 0;
    virtual void hide() = 0;
    virtual void rect(const QRect &rect) = 0;
    virtual QRect rect() = 0;

    virtual bool isVisible() = 0;

    virtual MediaPlayingState state() = 0;
    virtual bool state(MediaPlayingState state) = 0;

    virtual void aspectRatio(AspectRatio mode) = 0;
    virtual AspectRatio aspectRatio() = 0;

    virtual void move(int x, int y) = 0;    
    virtual void raise() = 0;

    virtual QList<AudioLangInfo> getAudioLanguages() = 0;
    virtual void setAudioLanguage(int index) = 0;

    virtual int getBrightness() = 0;
    virtual int getContrast() = 0;

    virtual MediaMetadata getMediaMetadata() = 0;

    virtual void setViewport(const QRect &requestedRect);
    virtual void setViewport(const QRect &containerRect, const qreal containerScale, const QRect &requestedRect);

    virtual void fullscreen(bool value);
    virtual bool fullscreen() const;

    virtual void resize();

    /*virtual void setUdpxyServer(const QString &server)
    {
        this->udpxyServer = server;
    }

    virtual QString getUdpxyServer()
    {
        return udpxyServer;
    }

    void useUdpxy(bool useUdpxy) { this->useUdpxyServer = useUdpxy; }
    bool useUdpxy() { return this->useUdpxyServer; }*/

    virtual void addHook(HookEvent type, Hook* hook);

    virtual void removeHook(HookEvent type, Hook* hook);

    virtual QSet<Hook*> hooks(HookEvent type);

    virtual bool processHooks(HookEvent type);

    virtual QString getQmlComponentName() const;

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

protected:
    QRect m_videoStoredRect;
    bool isFullscreen;

    //QString udpxyServer;
    //bool useUdpxyServer;

    QHash<HookEvent, QSet<Hook*> > hookList;

signals:
    void reseted();
};

}


#endif // MEDIAPLAYERPLUGIN_H
