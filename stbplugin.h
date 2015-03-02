#ifndef STBPLUGIN_H
#define STBPLUGIN_H

#include "stbprofileplugin.h"

#include "webobjectinfo.h"
#include "macros.h"
#include "datasource.h"

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

class StbSubmodel {
public:
    StbSubmodel() {}

    StbSubmodel(const QString &id, const QString &name){
        this->id = id;
        this->name = name;
    }

    StbSubmodel(const QString &id){
        this->id = id;
        this->name = id;
    }

    QString id;
    QString name;
    QString logo;
};

class StbPluginPrivate: public StbProfilePluginPrivate {
public:
    StbPluginPrivate() {
        datasourceInstance = NULL;
        mediaPlayer = NULL;
        guiPlugin = NULL;
        browserPlugin = NULL;
    }

    QHash<QString, QObject*> api;
    Datasource* datasourceInstance;
    MediaPlayerPlugin* mediaPlayer;
    QUrl portalUrl;
    GuiPlugin* guiPlugin;
    BrowserPlugin* browserPlugin;
    QList<WebObjectInfo> webObjects;
    QHash<int, RC_KEY> keyCodeMap;
    QList<StbSubmodel> subModels;
    QString submodelDatasourceGroup;
    QString submodelDatasourceField;
};

class StbPlugin: public StbProfilePlugin
{
    Q_DECLARE_PRIVATE(StbPlugin)
public:

    StbPlugin(): StbProfilePlugin(*new StbPluginPrivate)  {
    }

    QHash<QString, QObject*> getStbApiList()
    {
        Q_D(StbPlugin);
        return d->api;
    }

protected:
    StbPlugin(StbPluginPrivate &d)
       : StbProfilePlugin(d) {
    }

public:
    QString description;
    QString webName;

public slots:
    void gui(GuiPlugin* gui)
    {
        Q_D(StbPlugin);
        d->guiPlugin = gui;
    }

    GuiPlugin *gui()
    {
        Q_D(StbPlugin);
        return d->guiPlugin;
    }

    void player(MediaPlayerPlugin* player)
    {
        Q_D(StbPlugin);
        d->mediaPlayer = player;
    }
    MediaPlayerPlugin* player()
    {
        Q_D(StbPlugin);
        return d->mediaPlayer;
    }

    void browser(BrowserPlugin* browser)
    {
        Q_D(StbPlugin);
        d->browserPlugin = browser;
    }

    BrowserPlugin* browser()
    {
        Q_D(StbPlugin);
        return d->browserPlugin;
    }

    QList<WebObjectInfo> getWebObjects()
    {
        Q_D(StbPlugin);
        return d->webObjects;
    }

    QHash<int, RC_KEY> getKeyCodeMap()
    {
        Q_D(StbPlugin);
        return d->keyCodeMap;
    }

    bool addWebObject(const QString &name, QWidget* widget, const QString &mimeType, const QString &classid, const QString &description)
    {
        Q_D(StbPlugin);
        WebObjectInfo webObject;
        webObject.name = name;
        webObject.webObject = widget;
        webObject.mimeType = mimeType;
        webObject.description = description;
        webObject.classid = classid;
        webObject.widgetFactory = NULL;

        d->webObjects.append(webObject);
        return true;
    }

    bool addWebObject(const QString &name, const QString &mimeType, const QString &classid, const QString &description, std::function < QWidget*() > widgetFactory)
    {
        Q_D(StbPlugin);
        WebObjectInfo webObject;
        webObject.name = name;
        webObject.widgetFactory = widgetFactory;
        webObject.webObject = NULL;
        webObject.mimeType = mimeType;
        webObject.description = description;
        webObject.classid = classid;

        d->webObjects.append(webObject);
        return true;
    }

    virtual QUrl handleUrl(QUrl &url) { return url; }
    virtual void applyFixes() {}

    QHash<QString, QObject*>& getApi() {
        Q_D(StbPlugin);
        return d->api;
    }

    QList<StbSubmodel> &getSubmodels()
    {
        Q_D(StbPlugin);
        return d->subModels;
    }

    StbSubmodel& findSubmodel(const QString &id)
    {
        Q_D(StbPlugin);
        for(StbSubmodel &submodel: d->subModels)
            if(submodel.id == id)
                return submodel;
        throw new std::runtime_error(QString("Requested undefined submodel ID: %1").arg(id).toUtf8().constData());
    }

    QString listSubmodels()
    {
        Q_D(StbPlugin);
        QStringList result;
        for(StbSubmodel model: d->subModels)
        {
            result << model.name;
        }
        return result.join(", ");
    }

    QString getSubmodelDatasourceGroup() { Q_D(StbPlugin); return d->submodelDatasourceGroup; }
    QString getSubmodelDatasourceField() { Q_D(StbPlugin); return d->submodelDatasourceField; }

protected:
    void setSubmodelDatasourceField(const QString &group, const QString &field)
    {
        Q_D(StbPlugin);
        d->submodelDatasourceGroup = group;
        d->submodelDatasourceField = field;
    }

};

}

#define StbPlugin_iid "com.mvas.yasem.StbPlugin/1.0"

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(yasem::StbPlugin, StbPlugin_iid)
QT_END_NAMESPACE

#endif // STBPLUGIN_H
