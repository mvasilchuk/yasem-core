#ifndef STBPLUGIN_H
#define STBPLUGIN_H

#include "enums.h"
#include "macros.h"
#include "stbprofileplugin.h"
#include "datasource.h"


#include <functional>

#include <QtPlugin>
#include <QUrl>
#include <QHash>
#include <QDebug>
#include <QSize>

class QWidget;

namespace yasem
{
class DatasourcePlugin;
class BrowserPlugin;
class GuiPlugin;
class BaseWidget;

class MediaPlayerPlugin;

class StbPlugin: public virtual StbProfilePlugin
{
public:
    StbPlugin(): Plugin() {}
    virtual ~StbPlugin(){}

    QHash<QString, QObject*> getStbApiList()
    {
        return api;
    }

    typedef std::function<QWidget*()> WidgetFactory;

    struct WebObjectInfo {
        QString name;
        QWidget* webObject;
        QString mimeType;
        QList<QString> fileExtensions;
        QString description;
        QString classid;
        WidgetFactory widgetFactory;
    };

protected:
    QHash<QString, QObject*> api;
    Datasource* datasourceInstance;
    MediaPlayerPlugin* mediaPlayer;
    QUrl portalUrl;
    GuiPlugin* guiPlugin;
    BrowserPlugin* browserPlugin;
    QList<WebObjectInfo> webObjects;
    QHash<int, RC_KEY> keyCodeMap;
    QList<QString> subModels;

public:
    QString description;
    QString webName;

public slots:
    void gui(GuiPlugin* gui)
    {
        this->guiPlugin = gui;
    }

    GuiPlugin *gui()
    {
        return this->guiPlugin;
    }

    void player(MediaPlayerPlugin* player)
    {
        this->mediaPlayer = player;
    }
    MediaPlayerPlugin* player()
    {
        return this->mediaPlayer;
    }

    void browser(BrowserPlugin* browser)
    {
        this->browserPlugin = browser;
    }

    BrowserPlugin* browser()
    {
        return browserPlugin;
    }

    QList<WebObjectInfo> getWebObjects() const
    {
        return webObjects;
    }

    QHash<int, RC_KEY> getKeyCodeMap()
    {
        return keyCodeMap;
    }

    bool addWebObject(const QString &name, QWidget* widget, const QString &mimeType, const QString &classid, const QString &description)
    {
        WebObjectInfo webObject;
        webObject.name = name;
        webObject.webObject = widget;
        webObject.mimeType = mimeType;
        webObject.description = description;
        webObject.classid = classid;
        webObject.widgetFactory = NULL;

        webObjects.append(webObject);
        return true;
    }

    bool addWebObject(const QString &name, const QString &mimeType, const QString &classid, const QString &description, std::function < QWidget*() > widgetFactory)
    {
        WebObjectInfo webObject;
        webObject.name = name;
        webObject.widgetFactory = widgetFactory;
        webObject.webObject = NULL;
        webObject.mimeType = mimeType;
        webObject.description = description;
        webObject.classid = classid;

        webObjects.append(webObject);
        return true;
    }

    virtual QUrl handleUrl(QUrl &url) { return url; }
    virtual void applyFixes() {}

};

}

#define StbPlugin_iid "com.mvas.yasem.StbPlugin/1.0"

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(yasem::StbPlugin, StbPlugin_iid)
QT_END_NAMESPACE

#endif // STBPLUGIN_H
