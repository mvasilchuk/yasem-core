#ifndef SAMBANODE_H
#define SAMBANODE_H

#include <QObject>
#include <QList>
#include <QString>


namespace yasem {

enum SambaNodeType {
    SAMBA_UNDEFINED,
    SAMBA_ROOT,
    SAMBA_DOMAIN,
    SAMBA_HOST,
    SAMBA_SHARE
};

class SambaNode: public QObject
{
    Q_OBJECT
public:
    explicit SambaNode(QObject* parent);

    virtual ~SambaNode();
    SambaNodeType type;
    QString name;
    QString description;
    QList<SambaNode*> children;

    void addChild(SambaNode* child);

    void print();
};

}

#endif // SAMBANODE_H
