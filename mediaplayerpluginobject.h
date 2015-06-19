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
#include <QRect>
#include <QSet>
#include <QDebug>
#include <QPainter>

namespace yasem {

class MediaPlayerPluginObject: public AbstractPluginObject
{
    Q_OBJECT
    Q_PROPERTY(int brightness READ getBrightness WRITE setBrightness)
    Q_PROPERTY(int contrast READ getContrast WRITE setContrast)
    Q_PROPERTY(int saturation READ getSaturation WRITE setSaturation)
    Q_PROPERTY(qint64 position READ getPosition WRITE setPosition)
    Q_PROPERTY(int audioPID READ getAudioPID WRITE setAudioPID)
    Q_PROPERTY(int loop READ getLoop WRITE setLoop)
    Q_PROPERTY(qint64 duration READ getDuration)
    Q_PROPERTY(int volume READ getVolume WRITE setVolume)
public:
    explicit MediaPlayerPluginObject(Plugin* plugin);

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
    virtual QWidget* widget() const = 0;

    virtual bool mediaPlay(const QString &url) = 0;
    virtual bool mediaContinue() = 0;
    virtual bool mediaPause() = 0;
    virtual bool mediaStop() = 0;
    virtual bool mediaReset() = 0;

    virtual int getAudioPID() const = 0;
    virtual void setAudioPID(int pid) = 0;

    virtual int getLoop() const = 0;
    virtual void setLoop(int loop) = 0;

    virtual bool isMute() const = 0;
    virtual void setMute(bool value) = 0;


    virtual int bufferLoad() const = 0; //persents

    virtual qint64 getPosition() const = 0; //ms
    virtual void setPosition(qint64 pos) = 0;

    virtual qint64 getDuration() const = 0; // ms

    virtual int getVolume() const = 0;
    virtual void setVolume(int vol) = 0;

    virtual void show() = 0;
    virtual void hide() = 0;
    virtual void rect(const QRect &rect) = 0;
    virtual QRect rect() const = 0;

    virtual bool isVisible() const = 0;

    virtual MediaPlayingState state() = 0;
    virtual bool state(MediaPlayingState state) = 0;

    virtual void setAspectRatio(AspectRatio mode) = 0;
    virtual AspectRatio getAspectRatio() = 0;

    virtual void move(int x, int y) = 0;    

    virtual QList<AudioLangInfo> getAudioLanguages() = 0;
    virtual void setAudioLanguage(int index) = 0;

    virtual void setBrightness(int brightness) = 0;
    virtual int getBrightness() const = 0;

    virtual void setContrast(int contrast) = 0;
    virtual int getContrast() const = 0;

    virtual void setSaturation(int saturation) = 0;
    virtual int getSaturation() const = 0;

    qreal getOpacity() const;
    void setOpacity(qreal opacity);

    virtual void reset();

    virtual MediaMetadata getMediaMetadata() = 0;

    virtual void setViewport(const QRect &requestedRect);
    virtual void setViewport(const QRect &containerRect, const qreal containerScale, const QRect &requestedRect);
    virtual QRect getViewport() const;

    virtual void setVirtualViewport(const QRect &virtual_viewport);
    virtual QRect getVirtualViewport() const;

    virtual void recalculateViewportScale();

    virtual QPoint getWidgetPos() const = 0;

    virtual void setFullscreen(bool value);
    virtual bool isFullscreen() const;

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

    bool isSupportOpenGL();

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
    QRect m_player_viewport;
    bool m_is_fullscreen;
    qreal m_opacity;
    QRect m_virtual_viewport;
    QRect m_container_rect;

    qreal m_vp_scale_x;
    qreal m_vp_scale_y;

    bool m_support_opengl;

    //QString udpxyServer;
    //bool useUdpxyServer;

    QHash<HookEvent, QSet<Hook*> > hookList;

signals:
    void reseted();
    void rendered();
};

}


#endif // MEDIAPLAYERPLUGIN_H
