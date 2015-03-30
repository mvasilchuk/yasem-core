#include "mediaplayerpluginobject.h"

using namespace yasem;

MediaPlayerPluginObject::MediaPlayerPluginObject(Plugin *plugin):
    AbstractPluginObject(plugin)
{
    reset();
}

qreal MediaPlayerPluginObject::getOpacity() const
{
    return m_opacity;
}

void MediaPlayerPluginObject::setOpacity(qreal opacity)
{
    m_opacity = opacity;
}

void MediaPlayerPluginObject::reset()
{
    m_opacity = 1;
}

void MediaPlayerPluginObject::setViewport(const QRect &requestedRect)
{
    DEBUG() << "setViewport" << requestedRect << isFullscreen();
    m_videoStoredRect = requestedRect;
    resize();
}

void MediaPlayerPluginObject::setViewport(const QRect &containerRect, const qreal containerScale, const QRect &requestedRect)
{
    //#define USE_RELATIVE_RECT

    DEBUG() << "setViewport" << containerRect << containerScale << requestedRect << isFullscreen();
    if(requestedRect.width() >= 0 && requestedRect.height() >= 0)
    {
        if(isFullscreen())
        {
        //#ifdef USE_RELATIVE_RECT
            rect(containerRect);
        //#else
        //    rect(QRect(0, 0, containerRect.width(), containerRect.height()));
        //#endif // USE_RELATIVE_RECT
        }
        else
        {
            m_videoStoredRect = requestedRect;

            QRect currentVideoWidgetRect = rect();

            DEBUG() << "containerRect: " << containerRect;
            DEBUG() << "containerScale: " << containerScale;
            DEBUG() << "currentVideoWidgetRect:" << currentVideoWidgetRect;
            DEBUG() << "requestedRect:" << requestedRect;

        #ifdef USE_RELATIVE_RECT1
            QRect zoomedRect = QRect(
                        (int)((float)requestedRect.left() * containerScale + containerRect.left()),
                        (int)((float)requestedRect.top() * containerScale + containerRect.top()),
                        (int)((float)requestedRect.width() * containerScale),
                        (int)((float)requestedRect.height() * containerScale)
                        );

        #else

            QRect zoomedRect = QRect(
                        (int)((float)requestedRect.left() * containerScale),
                        (int)((float)requestedRect.top() * containerScale),
                        (int)((float)requestedRect.width() * containerScale),
                        (int)((float)requestedRect.height() * containerScale)
                        );

        #endif // USE_RELATIVE_RECT

            DEBUG() << "rect:" << zoomedRect;

            rect(zoomedRect);

        }
        widget()->repaint();
    }

    else
    {
    #ifdef USE_RELATIVE_RECT1
        move((int)((float)requestedRect.left() * containerScale + containerRect.left()),
             (int)((float)requestedRect.top() * containerScale + containerRect.top()));
    #else
        move((int)((float)requestedRect.left() * containerScale),
             (int)((float)requestedRect.top() * containerScale));
    #endif // USE_RELATIVE_RECT
    }
}

QRect MediaPlayerPluginObject::getViewport() const
{
    return rect();
}

void MediaPlayerPluginObject::setFullscreen(bool value) {
    this->m_is_fullscreen = value;
}

bool MediaPlayerPluginObject::isFullscreen() const {
    return this->m_is_fullscreen;
}

void MediaPlayerPluginObject::resize() {
    BrowserPluginObject* browser = dynamic_cast<BrowserPluginObject*>(PluginManager::instance()->getByRole(ROLE_BROWSER));
    if(browser)
    {
        AbstractWebPage* page = browser->getActiveWebPage();
        if(page)
        {
            QRect rect = page->getPageRect();
            qreal scale = page->scale();
            setViewport(rect, scale, m_videoStoredRect);
        }
    }
}

void MediaPlayerPluginObject::addHook(MediaPlayerPluginObject::HookEvent type, MediaPlayerPluginObject::Hook *hook)
{
    if(!this->hookList.contains(type))
        this->hookList.insert(type, QSet<Hook*>());
    QSet<Hook*> hooks = this->hookList.value(type);
    hooks.insert(hook);
}

void MediaPlayerPluginObject::removeHook(MediaPlayerPluginObject::HookEvent type, MediaPlayerPluginObject::Hook *hook)
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

QSet<MediaPlayerPluginObject::Hook *> MediaPlayerPluginObject::hooks(MediaPlayerPluginObject::HookEvent type)
{
    return hookList.value(type);
}

bool MediaPlayerPluginObject::processHooks(MediaPlayerPluginObject::HookEvent type)
{
    QSet<Hook*> list = this->hooks(type);
    foreach(Hook* hook, list)
    {
        if(!hook->func()) return false;
    }
    return true;
}

QString MediaPlayerPluginObject::getQmlComponentName() const
{
    return "";
}
