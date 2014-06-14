#include "yasemapplication.h"
#include "macros.h"

YasemApplication::YasemApplication(int &argc, char *argv[])
    : QApplication(argc, argv)
{
}


bool YasemApplication::notify(QObject *receiver, QEvent *event)
{
    try
    {
        return QApplication::notify( receiver, event );
    }
    catch ( std::exception& e )
    {
        ERROR() << e.what();
        return false;
    }
    catch(...)
    {
        ERROR() << "Generic exception";
        return false;
    }
}
