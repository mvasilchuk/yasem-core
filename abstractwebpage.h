#ifndef ABSTRACTWEBPAGE
#define ABSTRACTWEBPAGE

#include <QString>
#include <QSize>
#include <QRect>
#include <QUrl>
#include <QColor>

namespace yasem
{

class AbstractWebPage {
public:
    virtual bool load(const QUrl &url) = 0;
    virtual void close() = 0;
    virtual void evalJs(const QString &js) = 0;
    virtual void setPageViewportSize(QSize new_size) = 0;
    virtual QSize getVieportSize() = 0;
    virtual qreal scale() = 0;
    virtual QRect getPageRect() = 0;

    virtual bool isChromaKeyEnabled() const = 0;
    virtual void setChromaKeyEnabled(bool enabled) = 0;
    virtual QColor getChromaKey() const = 0;
    virtual void setChromaKey(QColor color) = 0;
    virtual QColor getChromaMask() const = 0;
    virtual void setChromaMask(QColor color) = 0;
    virtual float getOpacity() const = 0;
    virtual void setOpacity(float opacity) = 0;

    virtual void reset() = 0;
};

}

#endif // ABSTRACTWEBPAGE

