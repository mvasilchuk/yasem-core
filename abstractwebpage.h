#ifndef ABSTRACTWEBPAGE
#define ABSTRACTWEBPAGE

#include <QString>

namespace yasem
{

class AbstractWebPage {
public:
    virtual void close() = 0;
    virtual void evalJs(const QString &js) = 0;
};

}

#endif // ABSTRACTWEBPAGE

