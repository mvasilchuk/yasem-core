#ifndef MEDIAPLAYERPLUGIN_H
#define MEDIAPLAYERPLUGIN_H

#include "enums.h"
#include "mediasignalsender.h"

#include <QWidget>
#include <QtPlugin>


class QString;

#include <QRect>
#include <QSet>
#include <QDebug>

namespace yasem {

class SignalSender;
class MediaPlayerPlugin
{
public:
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
        friend class MediaPlayerPlugin;
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

    qreal scale()               { return widgetScale; }
    void scale(qreal value)     { widgetScale = value; }

    QRect fixedRect() { return this->widgetFixedRect; }
    void fixedRect(const QRect &rect) { this->widgetFixedRect = rect; }

    virtual bool isVisible() = 0;

    virtual MediaPlayingState state() = 0;
    virtual bool state(MediaPlayingState state) = 0;

    virtual void aspectRatio(ASPECT_RATIO mode) = 0;
    virtual ASPECT_RATIO aspectRatio() = 0;

    virtual void move(int x, int y) = 0;    
    virtual void raise() = 0;

    MediaSignalSender& getSignalSender() { return mediaSignalSender; }

    void setViewport(const QRect &baseRect, const qreal baseScale, const QRect &newRect)
    {
        if(newRect.width() >= 0 && newRect.height() >= 0)
        {
            if(fullscreen())
            {
                rect(baseRect);
            }
            else
            {
                fixedRect(newRect);

                QRect videoRect = rect();

                qDebug() << "baseRect: " << baseRect;
                qDebug() << "baseScale: " << baseScale;
                qDebug() << "video rect:" << videoRect;
                qDebug() << "videoFixedRect:" << newRect;

                QRect zoomedRect = QRect(
                            (int)((float)newRect.left() * baseScale + baseRect.left()),
                            (int)((float)newRect.top() * baseScale + baseRect.top()),
                            (int)((float)newRect.width() * baseScale ),
                            (int)((float)newRect.height() * baseScale)
                        );

                qDebug() << "rect:" << zoomedRect;

                this->baseRect = baseRect;
                scale(baseScale);
                rect(zoomedRect);

            }
            widget()->repaint();
        }
        else
            move(newRect.left(), newRect.top());
    }

    void fullscreen(bool value) { this->isFullscreen = value; }
    bool fullscreen() { return this->isFullscreen; }

    virtual void resize() {
         setViewport(baseRect, scale(), fixedRect());
    }

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

    void addHook(HookEvent type, Hook* hook)
    {
        if(!this->hookList.contains(type))
            this->hookList.insert(type, QSet<Hook*>());
        QSet<Hook*> hooks = this->hookList.value(type);
        hooks.insert(hook);
    }

    void removeHook(HookEvent type, Hook* hook)
    {
        if(hookList.contains(type))
        {
            QSet<Hook*> hooks = hookList.value(type);
            hooks.remove(hook);
            delete hook;
        }
        else
        {
            qDebug() << "Hook type" << type << "not found. Cannot delete!";
        }
    }

    QSet<Hook*> hooks(HookEvent type)
    {
        return hookList.value(type);
    }

    bool processHooks(HookEvent type)
    {
        QSet<Hook*> list = this->hooks(type);
        foreach(Hook* hook, list)
        {
            if(!hook->func()) return false;
        }
        return true;
    }

protected:
    qreal widgetScale;
    QRect widgetFixedRect;
    QRect baseRect; // Browser rect
    MediaSignalSender mediaSignalSender;
    bool isFullscreen;

    //QString udpxyServer;
    //bool useUdpxyServer;

    QHash<HookEvent, QSet<Hook*> > hookList;

signals:
    void reseted();
};

}

#define MediaPlayerPlugin_iid "com.mvas.yasem.MediaPlayerPlugin/1.0"

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(yasem::MediaPlayerPlugin, MediaPlayerPlugin_iid)
QT_END_NAMESPACE

#endif // MEDIAPLAYERPLUGIN_H
