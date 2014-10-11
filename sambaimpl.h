#ifndef SAMBAIMPL_H
#define SAMBAIMPL_H

#include "samba.h"
#include "sambanode.h"
#include "mountpointinfo.h"

namespace yasem {



class SambaImpl : public Samba
{
    Q_OBJECT
public:

    SambaImpl(QObject* parent);
    virtual ~SambaImpl();

    // Samba interface
public:
    virtual QList<SambaNode*> domains();
    virtual QList<SambaNode*> hosts(const QString &domainName);
    virtual QList<SambaNode*> shares(const QString &hostName);
    virtual void makeTree();
    virtual bool mount(const QString &what, const QString &where, const QString &options);
    virtual bool unmount(const QString &path);


protected:
    QList<MountPointInfo*> mountPoints;
    SambaNode* sambaTree;
};

}


#endif // SAMBAIMPL_H
