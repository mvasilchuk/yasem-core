#ifndef STBPLUGIN_P_H
#define STBPLUGIN_P_H

#include "enums.h"
#include "stbsubmodule.h"
#include "webobjectinfo.h"
#include "plugin_p.h"

#include <QString>
#include <QList>
#include <QHash>
#include <QUrl>

namespace yasem {

class StbPluginObject;
class DatasourcePluginObject;
class MediaPlayerPluginObject;
class GuiPluginObject;
class BrowserPluginObject;

class StbPluginObjectPrivate {
    Q_DECLARE_PUBLIC(StbPluginObject)
public:
    explicit StbPluginObjectPrivate(StbPluginObject* q);

protected:
     StbPluginObject* q_ptr;
public:

    QHash<QString, QObject*> api;
    DatasourcePluginObject* datasourceInstance;
    MediaPlayerPluginObject* mediaPlayer;
    QUrl portalUrl;
    GuiPluginObject* guiPlugin;
    BrowserPluginObject* browserPlugin;
    QList<WebObjectInfo> webObjects;
    QHash<int, RC_KEY> keyCodeMap;
    QList<StbSubmodel> subModels;
    QString submodelDatasourceGroup;
    QString submodelDatasourceField;
};

}

#endif // STBPLUGIN_P_H

