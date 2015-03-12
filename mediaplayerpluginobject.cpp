#include "mediaplayerpluginobject.h"

using namespace yasem;

MediaPlayerPluginObject::MediaPlayerPluginObject(Plugin *plugin, QObject *parent):
    AbstractPluginObject(plugin, parent)
{

}

void MediaPlayerPluginObject::setViewport(const QRect &requestedRect)
{
    DEBUG() << "setViewport" << requestedRect << fullscreen();
    m_videoStoredRect = requestedRect;
    resize();
}

void MediaPlayerPluginObject::setViewport(const QRect &containerRect, const qreal containerScale, const QRect &requestedRect)
{
    DEBUG() << "setViewport" << containerRect << containerScale << requestedRect << fullscreen();
    if(requestedRect.width() >= 0 && requestedRect.height() >= 0)
    {
        if(fullscreen())
        {
            rect(containerRect);
        }
        else
        {
            m_videoStoredRect = requestedRect;

            QRect currentVideoWidgetRect = rect();

            DEBUG() << "containerRect: " << containerRect;
            DEBUG() << "containerScale: " << containerScale;
            DEBUG() << "currentVideoWidgetRect:" << currentVideoWidgetRect;
            DEBUG() << "requestedRect:" << requestedRect;

            QRect zoomedRect = QRect(
                        (int)((float)requestedRect.left() * containerScale + containerRect.left()),
                        (int)((float)requestedRect.top() * containerScale + containerRect.top()),
                        (int)((float)requestedRect.width() * containerScale),
                        (int)((float)requestedRect.height() * containerScale)
                        );

            DEBUG() << "rect:" << zoomedRect;

            rect(zoomedRect);

        }
        widget()->repaint();
    }
    else
        move((int)((float)requestedRect.left() * containerScale + containerRect.left()),
             (int)((float)requestedRect.top() * containerScale + containerRect.top()));
}

void MediaPlayerPluginObject::fullscreen(bool value) {
    this->isFullscreen = value;
}

bool MediaPlayerPluginObject::fullscreen() const {
    return this->isFullscreen;
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
