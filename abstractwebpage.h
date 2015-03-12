#ifndef ABSTRACTWEBPAGE
#define ABSTRACTWEBPAGE

#include <QString>
#include <QSize>
#include <QRect>

namespace yasem
{

class AbstractWebPage {
public:
    virtual void close() = 0;
    virtual void evalJs(const QString &js) = 0;
    virtual void setVieportSize(QSize new_size) = 0;
    virtual QSize getVieportSize() = 0;
    virtual qreal scale() = 0;
    virtual QRect getPageRect() = 0;
};

}

#endif // ABSTRACTWEBPAGE

