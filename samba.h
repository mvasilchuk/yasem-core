#ifndef SAMBA_H
#define SAMBA_H

#include "sambanode.h"

#include <QDebug>
#include <QObject>
#include <QString>
#include <QList>
#include <QStringList>

namespace yasem
{
enum FileType {
    File,
    Dir
};

class Samba: public QObject {
    Q_OBJECT
public:
    Samba(QObject* parent = 0):  QObject(parent) {}
    virtual ~Samba() {}
    virtual QList<SambaNode*> domains() = 0;
    virtual QList<SambaNode*> hosts(const QString &domainName) = 0;
    virtual QList<SambaNode*> shares(const QString &hostName) = 0;
    virtual void makeTree() = 0;
    virtual bool mount(const QString &what, const QString &where, const QString &params) = 0;
    virtual bool unmount(const QString &path) = 0;
};
}

#endif // SAMBA_H
