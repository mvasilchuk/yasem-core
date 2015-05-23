#ifndef LOGGERCORE_H
#define LOGGERCORE_H

#include "stdio.h"

#include <QObject>
#include <QRegExp>
#include <QFile>

class LoggerCore : public QObject
{
    Q_OBJECT
public:
    explicit LoggerCore(QObject *parent = 0);
    virtual ~LoggerCore();

    static void initLogFile(QObject* parent);

signals:

protected:


public slots:

    static void MessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
protected:
    static QString colorize(const QString &str);

    static QFile* m_log_file;

    class Colorizer: public QString {
        QString data;
    public:
        static QString format(const QString &str, int color)
        {
            return QString("\033[%1m%2\033[0m").arg(color).arg(str);
        }
    };
};

#endif // LOGGERCORE_H
