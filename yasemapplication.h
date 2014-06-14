#ifndef YASEMAPPLICATION_H
#define YASEMAPPLICATION_H

#include <QApplication>

class YasemApplication : public QApplication
{
public:
    YasemApplication(int &argc, char *argv[]);

private:
    bool notify(QObject *receiver, QEvent *event);
};

#endif // YASEMAPPLICATION_H
