#ifndef STBSUBMODULE_H
#define STBSUBMODULE_H

#include <QString>


namespace yasem {
class StbSubmodel
{
public:
    StbSubmodel() {}

    StbSubmodel(const QString &id, const QString &name = "", const QString &logo = ""):
        m_id(id),
        m_name(name),
        m_logo(logo)
    {
    }


    QString getId() const
    {
        return m_id;
    }

    QString getName() const
    {
        return m_name;
    }

    QString getLogo() const
    {
        return m_logo;
    }

protected:
    QString m_id;
    QString m_name;
    QString m_logo;
};
}

#endif // STBSUBMODULE_H

