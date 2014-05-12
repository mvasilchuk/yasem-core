#include "consolelogger.h"
#include <QCoreApplication>
#include <QStringList>
#include <QDebug>

ConsoleLogger::ConsoleLogger(QObject *parent) :
    Logger()
{
    this->parent = parent;
    colorOutput = false;
    debugRegExp = QRegExp("^(?:\\[)(DEBUG)(?:\\s*\\])");
    stubRegExp = QRegExp("^(?:\\[)(STUB)(?:\\s*\\])");
    bugRegExp = QRegExp("^(?:\\[)(BUG)(?:\\s*\\])");
    warnRegExp = QRegExp("^(?:\\[)(WARN)(?:\\s*\\])");
    logRegExp = QRegExp("^(?:\\[)(LOG)(?:\\s*\\])");
    errorRegExp = QRegExp("^(?:\\[)(ERROR)(?:\\s*\\])");
    fatalRegExp = QRegExp("^(?:\\[)(FATAL)(?:\\s*\\])");
    infoRegExp = QRegExp("^(?:\\[)(INFO)(?:\\s*\\])");
    methodCallRegExp = QRegExp("(\\w+)(::)(\\w+)(?:(\\s*\\()(.*)(\\))){0,1}");


    if(QCoreApplication::arguments().contains(arguments[COLOR_OUTPUT]))
        colorOutput = true;

    this->setObjectName("ConsoleLogger");

    //connect(this, &ConsoleLogger::log, this, &ConsoleLogger::onLog);
    //connect(this, &ConsoleLogger::debug, this, &ConsoleLogger::onDebug);
    //connect(this, &ConsoleLogger::info, this, &ConsoleLogger::onInfo);
    //connect(this, &ConsoleLogger::error, this, &ConsoleLogger::onError);
    //connect(this, &ConsoleLogger::stub, this, &ConsoleLogger::onStub);
}

void ConsoleLogger::onLog(QObject* obj, const QString &msg)
{
    print(obj, "LOG  ", msg);
}

void ConsoleLogger::onDebug(QObject* obj, const QString &msg)
{
    print(obj, "DEBUG", msg);
}

void ConsoleLogger::onInfo(QObject* obj, const QString &msg)
{
    print(obj, "INFO ", msg);
}

void ConsoleLogger::onError(QObject* obj, const QString &msg)
{
    print(obj, "ERROR", msg);
}

void ConsoleLogger::onStub(QObject* obj, const QString &msg)
{
    print(obj, "STUB ", msg, "[%1] %2", false);
}

void ConsoleLogger::onBug(QObject* obj, const QString &msg)
{
    print(obj, "BUG  ", msg);
}

void ConsoleLogger::onWarn(QObject* obj, const QString &msg)
{
    print(obj, "WARN ", msg);
}

void ConsoleLogger::onFatal(QObject* obj, const QString &msg)
{
    print(obj, "FATAL", msg);
}

void ConsoleLogger::print(const QObject* sender, const QString &tag, const QString &msg, const QString &pattern, bool printSenderName)
{
    //Q_ASSERT(sender != NULL); // "Slot didn't called from signal
    if(printSenderName)
    {
        QString senderName = sender == NULL ? "GLOBAL" : sender->objectName();
        if(senderName == "")
            senderName = sender->metaObject()->className();

        if(pattern == "")
            qDebug() << qPrintable(format(QString("[%1] <%2>: %3").arg(tag, senderName, msg)));
        else
            qDebug() << qPrintable(format(pattern.arg(tag, senderName, msg)));
    }
    else
        qDebug() << qPrintable(format(pattern.arg(tag, msg)));

}

QString ConsoleLogger::format(const QString &str)
{
    if(colorOutput)
    {
        int debugColor = 36;
        int stubColor = 32;
        int logColor = 37;
        int errorColor = 31;
        int infoColor = 35;
        int methodColor = 33;
        int methodParamsColor  = 32;
        int normalColor = 39;

        QString result = str;
        result.replace(debugRegExp, Colorizer::format("[DEBUG]", debugColor));
        result.replace(stubRegExp, Colorizer::format("[STUB ]", stubColor));
        result.replace(bugRegExp, Colorizer::format("[BUG  ]", stubColor));
        result.replace(warnRegExp, Colorizer::format("[WARN ]", stubColor));
        result.replace(logRegExp, Colorizer::format("[LOG  ]", logColor));
        result.replace(errorRegExp, Colorizer::format("[ERROR]", errorColor));
        result.replace(fatalRegExp, Colorizer::format("[FATAL]", errorColor));
        result.replace(infoRegExp, Colorizer::format("[INFO ]", infoColor));

        QString methodCall = QString("%1%2%3%4%5%6")
                .arg(Colorizer::format("\\1", methodColor))         // class
                .arg(Colorizer::format("\\2", stubColor))           // ::
                .arg(Colorizer::format("\\3", methodColor))         // methodName
                .arg(Colorizer::format("\\4", normalColor))         // (
                .arg(Colorizer::format("\\5", methodParamsColor))   // params
                .arg(Colorizer::format("\\6", normalColor));        // )

        result.replace(methodCallRegExp, methodCall);

        return result;
    }
    else
        return str;
}
