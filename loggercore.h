#ifndef LOGGERCORE_H
#define LOGGERCORE_H

#include <QObject>


class LoggerCore : public QObject
{
    Q_OBJECT
public:
    explicit LoggerCore(QObject *parent = 0);

signals:

public slots:

    static void MessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
};

#endif // LOGGERCORE_H
