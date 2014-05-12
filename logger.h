#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QCoreApplication>
#include <QVariant>

class Logger : public QObject
{
    Q_OBJECT
public:
    static Logger* setInstance(Logger* inst = 0)
    {
        static Logger* instance = inst;// Guaranteed to be destroyed.

        if(instance == NULL)
            instance = static_cast<Logger*>(qApp->property("Logger").value<QObject*>());

        Q_CHECK_PTR(instance);
        return instance;
    }

    static Logger* instance()
    {
       return setInstance();
    }

    template <class T>
    static void log(QObject* obj, const T &msg)     { return instance()->onLog(obj, QString(msg)); }
    template <class T>
    static void debug(QObject* obj, const T &msg)   { return instance()->onDebug(obj, QString(msg)); }
    template <class T>
    static void info(QObject* obj, const T &msg)    { return instance()->onInfo(obj, QString(msg)); }
    template <class T>
    static void error(QObject* obj, const T &msg)   { return instance()->onError(obj, QString(msg)); }
    template <class T>
    static void stub(QObject* obj, const T &msg)    { return instance()->onStub(obj, QString(msg)); }
    template <class T>
    static void bug(QObject* obj, const T &msg)     { return instance()->onBug(obj, QString(msg)); }
    template <class T>
    static void warn(QObject* obj, const T &msg)    { return instance()->onWarn(obj, QString(msg)); }
    template <class T>
    static void fatal(QObject* obj, const T &msg)   { return instance()->onFatal(obj, QString(msg)); }

    //virtual Logger* operator <<(const QString &value) = 0;
    //virtual Logger* operator <<(int value) = 0;
    //virtual Logger* operator <<(const QVariant &value) = 0;


public slots:
    virtual void onLog(QObject* obj, const QString &msg) = 0;
    virtual void onDebug(QObject* obj, const QString &msg) = 0;
    virtual void onInfo(QObject* obj, const QString &msg) = 0;
    virtual void onError(QObject* obj, const QString &msg) = 0;
    virtual void onStub(QObject* obj, const QString &msg) = 0;
    virtual void onBug(QObject* obj, const QString &msg) = 0;
    virtual void onWarn(QObject* obj, const QString &msg) = 0;
    virtual void onFatal(QObject* obj, const QString &msg) = 0;


protected:
    Logger(QObject* parent = 0){ Q_UNUSED(parent)}
    Logger(Logger const&);
};

#endif // LOGGER_H
