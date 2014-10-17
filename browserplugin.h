#ifndef BROWSERPLUGIN_H
#define BROWSERPLUGIN_H

#include "enums.h"

#include <QSize>
#include <QObject>
#include <QMetaMethod>

class QRect;
class QUrl;
class QResizeEvent;
class QWidget;
class QEvent;

namespace yasem
{
class StbPlugin;
class CustomKeyEvent;
class BrowserPlugin
{
public:
    virtual ~BrowserPlugin(){}

    virtual void parent(QWidget *parent) = 0;
    virtual QWidget* parent() = 0;
    virtual bool load(const QUrl &url) = 0;
    virtual void evalJs(const QString &js) = 0;
    virtual QWidget* widget() = 0 ;
    virtual void resize(QResizeEvent* = 0) = 0;
    virtual void rect(const QRect &rect) = 0;
    virtual QRect rect() = 0;
    virtual void scale(qreal scale) = 0;
    virtual qreal scale() = 0;
    virtual void stb(StbPlugin* stbPlugin) = 0;
    virtual StbPlugin* stb() = 0;
    virtual void raise() = 0;
    virtual void show() = 0;
    virtual void hide() = 0;
    virtual void setInnerSize(int width, int height) = 0;
    virtual void setInnerSize(const QSize &size) = 0;
    QSize getInnerSize() { return innerSize; }
    virtual void fullscreen(bool setFullscreen) = 0;
    virtual bool fullscreen() = 0;

    virtual QUrl url() = 0;
    virtual QString browserRootDir() = 0;
    virtual void setUserAgent(const QString &userAgent) = 0;
    virtual void addFont(const QString &fileName) = 0;
    virtual bool receiveKeyCode(RC_KEY keyCode) = 0;
    virtual void registerKeyEvent(RC_KEY rc_key, int keyCode, int which, bool alt = false, bool ctrl = false, bool shift = false) = 0;
    virtual void registerKeyEvent(RC_KEY rc_key, int keyCode, int which, int keyCode2, int which2, bool alt = false, bool ctrl = false, bool shift = false) = 0;
    virtual void clearKeyEvents() = 0;
    virtual void passEvent(QEvent *event) = 0;

    virtual void setupMousePositionHandler(const QObject *receiver, const char* method) = 0;

    virtual void setOpacity(qint32 alpha) = 0;
    virtual qint32 getOpacity() = 0;

protected:
    QSize innerSize;
    QWidget* activeWebView;

};
}

#define BrowserPlugin_iid "com.mvas.yasem.BrowserPlugin/1.0"

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(yasem::BrowserPlugin, BrowserPlugin_iid)
QT_END_NAMESPACE

#endif // BROWSERPLUGIN_H
