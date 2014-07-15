#ifndef WEBOBJECTINFO_H
#define WEBOBJECTINFO_H

#include <QString>
#include <QList>

#include <functional>

class QWidget;

namespace yasem {

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
}



#endif // WEBOBJECTINFO_H
