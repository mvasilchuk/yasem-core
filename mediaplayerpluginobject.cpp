#include "mediaplayerpluginobject.h"

using namespace yasem;

MediaPlayerPluginObject::MediaPlayerPluginObject(Plugin *plugin):
    AbstractPluginObject(plugin),
    m_virtual_viewport(QRect(0, 0, 1920, 1080))
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
    m_player_viewport = requestedRect;
    recalculateViewportScale();
    resize();
}

void MediaPlayerPluginObject::setViewport(const QRect &containerRect, const qreal containerScale, const QRect &requestedRect)
{
    //#define USE_RELATIVE_RECT

#ifndef USE_REAL_TRANSPARENCY
#define USE_RELATIVE_RECT
#endif //USE_REAL_TRANSPARENCY

    DEBUG() << "setViewport" << containerRect << containerScale << requestedRect << isFullscreen();
    if(requestedRect.width() >= 0 && requestedRect.height() >= 0)
    {
        if(isFullscreen())
        {
        #ifdef USE_RELATIVE_RECT
            rect(containerRect);
        #else
            rect(QRect(0, 0, containerRect.width(), containerRect.height()));
        #endif // USE_RELATIVE_RECT
        }
        else
        {
            m_player_viewport = requestedRect;
            m_container_rect = containerRect;
            recalculateViewportScale();

            QRect currentVideoWidgetRect = rect();

            DEBUG() << "containerRect: " << containerRect;
            DEBUG() << "containerScale: " << containerScale;
            DEBUG() << "currentVideoWidgetRect:" << currentVideoWidgetRect;
            DEBUG() << "requestedRect:" << requestedRect;
            DEBUG() << "vm scale x" << m_vp_scale_x;
            DEBUG() << "vm scale y" << m_vp_scale_y;


        #ifdef USE_RELATIVE_RECT
            QRect zoomedRect = QRect(
                        (int)((float)requestedRect.left() / m_vp_scale_x + containerRect.left()),
                        (int)((float)requestedRect.top() / m_vp_scale_y + containerRect.top()),
                        (int)((float)requestedRect.width() / m_vp_scale_x),
                        (int)((float)requestedRect.height() / m_vp_scale_y)
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
    #ifdef USE_RELATIVE_RECT
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

/**
 * @brief MediaPlayerPluginObject::setVirtualViewport
 *
 * Sets a virtaul viewport for a player.
 *
 * @param virtual_viewport
 */
void MediaPlayerPluginObject::setVirtualViewport(const QRect &virtual_viewport)
{
    m_virtual_viewport = virtual_viewport;
    recalculateViewportScale();
}

QRect MediaPlayerPluginObject::getVirtualViewport() const
{
    return m_virtual_viewport;
}

void MediaPlayerPluginObject::recalculateViewportScale()
{
    if(m_container_rect.width() > 0 && m_container_rect.height() > 0)
    {
        m_vp_scale_x = (qreal)m_virtual_viewport.width() / m_container_rect.width();
        m_vp_scale_y = (qreal)m_virtual_viewport.height() / m_container_rect.height();
    }
    else
    {
        m_vp_scale_x = 1;
        m_vp_scale_y = 1;
    }

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
            setViewport(rect, scale, m_player_viewport);
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
