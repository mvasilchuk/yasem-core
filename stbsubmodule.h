#ifndef STBSUBMODULE_H
#define STBSUBMODULE_H

#include <QString>


namespace yasem{
class StbSubmodel {
public:
    StbSubmodel() {}

    StbSubmodel(const QString &id, const QString &name):
        m_id(id),
        m_name(name)
    {
    }

    StbSubmodel(const QString &id)
        :m_id(id),
         m_name("")
    {
    }

    QString m_id;
    QString m_name;
    QString m_logo;
};
}

#endif // STBSUBMODULE_H

