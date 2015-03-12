
#include "stbpluginobject.h"
#include "stbpluginobject_p.h"
#include "profilemanager.h"

using namespace yasem;

StbPluginObject::StbPluginObject(Plugin* plugin):
    AbstractPluginObject(plugin),
    d_ptr(new StbPluginObjectPrivate(this))
{

}

StbPluginObject::~StbPluginObject()
{

}

QHash<QString, QObject *> StbPluginObject::getStbApiList()
{
    Q_D(StbPluginObject);
    return d->api;
}

PluginObjectResult StbPluginObject::init()
{
    ProfileManager::instance()->registerProfileClassId(getProfileClassId(), this);
    return PLUGIN_OBJECT_RESULT_OK;
}

void StbPluginObject::gui(GuiPluginObject *gui)
{
    Q_D(StbPluginObject);
    d->guiPlugin = gui;
}

GuiPluginObject *StbPluginObject::gui()
{
    Q_D(StbPluginObject);
    return d->guiPlugin;
}

void StbPluginObject::player(MediaPlayerPluginObject *player)
{
    Q_D(StbPluginObject);
    d->mediaPlayer = player;
}

MediaPlayerPluginObject *StbPluginObject::player()
{
    Q_D(StbPluginObject);
    return d->mediaPlayer;
}

void StbPluginObject::browser(BrowserPluginObject *browser)
{
    Q_D(StbPluginObject);
    d->browserPlugin = browser;
}

BrowserPluginObject *StbPluginObject::browser()
{
    Q_D(StbPluginObject);
    return d->browserPlugin;
}

QList<WebObjectInfo> StbPluginObject::getWebObjects()
{
    Q_D(StbPluginObject);
    return d->webObjects;
}

QHash<int, RC_KEY> StbPluginObject::getKeyCodeMap()
{
    Q_D(StbPluginObject);
    return d->keyCodeMap;
}

bool StbPluginObject::addWebObject(const QString &name, QWidget *widget, const QString &mimeType, const QString &classid, const QString &description)
{
    Q_D(StbPluginObject);
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

bool StbPluginObject::addWebObject(const QString &name, const QString &mimeType, const QString &classid, const QString &description, std::function<QWidget *()> widgetFactory)
{
    Q_D(StbPluginObject);
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

QHash<QString, QObject *> &StbPluginObject::getApi()
{
    Q_D(StbPluginObject);
    return d->api;
}

QList<StbSubmodel> &StbPluginObject::getSubmodels()
{
    Q_D(StbPluginObject);
    return d->subModels;
}

StbSubmodel &StbPluginObject::findSubmodel(const QString &id)
{
    Q_D(StbPluginObject);
    for(StbSubmodel &submodel: d->subModels)
        if(submodel.m_id == id)
            return submodel;
    throw new std::runtime_error(QString("Requested undefined submodel ID: %1").arg(id).toUtf8().constData());
}

QString StbPluginObject::listSubmodels()
{
    Q_D(StbPluginObject);
    QStringList result;
    for(StbSubmodel model: d->subModels)
    {
        result << model.m_name;
    }
    return result.join(", ");
}

QString StbPluginObject::getSubmodelDatasourceGroup()
{
    Q_D(StbPluginObject);
    return d->submodelDatasourceGroup;
}

QString StbPluginObject::getSubmodelDatasourceField()
{
    Q_D(StbPluginObject);
    return d->submodelDatasourceField;
}

void StbPluginObject::setSubmodelDatasourceField(const QString &group, const QString &field)
{
    Q_D(StbPluginObject);
    d->submodelDatasourceGroup = group;
    d->submodelDatasourceField = field;
}


StbPluginObjectPrivate::StbPluginObjectPrivate(StbPluginObject *q):
    q_ptr(q),
    datasourceInstance(NULL),
    mediaPlayer(NULL),
    guiPlugin(NULL),
    browserPlugin(NULL)
{
}
