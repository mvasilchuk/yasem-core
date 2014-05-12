#ifndef CONSOLELOGGER_H
#define CONSOLELOGGER_H

#include "enums.h"
#include "logger.h"
#include <QObject>
#include <QRegExp>

class ConsoleLogger : public Logger
{
    Q_OBJECT
public:
    explicit ConsoleLogger(QObject *parent = 0);

signals:

public slots:
    virtual void onLog(QObject* obj, const QString &msg);
    virtual void onDebug(QObject* obj, const QString &msg);
    virtual void onInfo(QObject* obj, const QString &msg);
    virtual void onError(QObject* obj, const QString &msg);
    virtual void onStub(QObject* obj, const QString &msg);
    virtual void onBug(QObject* obj, const QString &msg);
    virtual void onWarn(QObject* obj, const QString &msg);
    virtual void onFatal(QObject* obj, const QString &msg);

protected slots:


    // Logger interface
public:


protected:
    QObject* parent;
    bool colorOutput;
    QRegExp debugRegExp;
    QRegExp stubRegExp;
    QRegExp errorRegExp;
    QRegExp infoRegExp;
    QRegExp logRegExp;
    QRegExp methodCallRegExp;
    QRegExp fatalRegExp;
    QRegExp bugRegExp;
    QRegExp warnRegExp;

    void print(const QObject* sender, const QString &tag, const QString &msg, const QString &pattern = "", bool printSenderName = true);
    QString format(const QString &str);

    class Colorizer: public QString {
        QString data;
    public:
        static QString format(const QString &str, int color)
        {
            return QString("\033[%1m%2\033[0m").arg(color).arg(str);
        }
    };
};

#endif // CONSOLELOGGER_H
