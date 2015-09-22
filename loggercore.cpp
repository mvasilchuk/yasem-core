#include "core.h"
#include "loggercore.h"

#include <cstdio>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QFileInfo>

#define LOG_TYPE_DEBUG QtDebugMsg
#define LOG_TYPE_WARN QtWarningMsg
#define LOG_TYPE_CRITICAL QtCriticalMsg
#define LOG_TYPE_FATAL QtFatalMsg

#define LOG_TYPE_STUB 1000
#define LOG_TYPE_LOG 1001
#define LOG_TYPE_INFO 1002

#define LOG_TYPE_WTF 1010
#define LOG_TYPE_FIXME 1011
#define LOG_TYPE_BUG 1015

static const char* LOG_PREFIX_STUB = "$STUB$";
static const char* LOG_PREFIX_LOG = "$LOG$";
static const char* LOG_PREFIX_INFO = "$INFO$";
static const char* LOG_PREFIX_WTF = "$WTF$";
static const char* LOG_PREFIX_FIXME = "$FIXME$";

static const QString TIME_FORMAT = "hh:mm:ss:zzz";

using namespace yasem;

QFile* LoggerCore::m_log_file = NULL;

LoggerCore::LoggerCore(QObject *parent) :
    QObject(parent)
{

}

LoggerCore::~LoggerCore()
{
}

void LoggerCore::MessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    #ifdef QT_DEBUG
        static bool verboseOutput = true;
    #else
        static bool verboseOutput = qApp->arguments().contains("--verbose");
    #endif

    /**
     * @brief localMsg
     *
        %{appname}	QCoreApplication::applicationName()
        %{file}	Path to source file
        %{function}	Function
        %{line}	Line in source file
        %{message}	The actual message
        %{pid}	QCoreApplication::applicationPid()
        %{threadid}	ID of current thread
        %{type}	"debug", "warning", "critical" or "fatal"
     */
    QByteArray localMsg = msg.toLocal8Bit();

    int msgType = (int) type;

    if(localMsg.startsWith(LOG_PREFIX_STUB))
    {
        msgType = LOG_TYPE_STUB;
        localMsg = localMsg.right(localMsg.length() - strlen(LOG_PREFIX_STUB));
    }
    else if(localMsg.startsWith(LOG_PREFIX_LOG))
    {
        msgType = LOG_TYPE_LOG;
        localMsg = localMsg.right(localMsg.length() - strlen(LOG_PREFIX_LOG));
    }
    else if(localMsg.startsWith(LOG_PREFIX_INFO))
    {
        msgType = LOG_TYPE_INFO;
        localMsg = localMsg.right(localMsg.length() - strlen(LOG_PREFIX_INFO));
    }
    else if(localMsg.startsWith(LOG_PREFIX_WTF))
    {
        msgType = LOG_TYPE_WTF;
        localMsg = localMsg.right(localMsg.length() - strlen(LOG_PREFIX_WTF));
    }
    else if(localMsg.startsWith(LOG_PREFIX_FIXME))
    {
        msgType = LOG_TYPE_FIXME;
        localMsg = localMsg.right(localMsg.length() - strlen(LOG_PREFIX_FIXME));
    }

    QString message = localMsg;

    QString line = "";

#ifdef EXTRA_DEBUG_INFO
    if(context.file != NULL)
        line = QString("(%1:%2) ").arg(context.file).arg(QString::number(context.line));
#endif

    QDateTime time = QDateTime::currentDateTime();
    QString current_time;

    if(time.isValid())
    {
        try {
            current_time = time.toString(TIME_FORMAT);
            //Sometimes it throws exception if the app is going to close
        }catch(...)
        {
            current_time = "";
        }
    }
    else
    {
        current_time = "";
    }

    FILE* output_channel = stdout;
    QString output_line;

    switch (msgType) {
        case LOG_TYPE_DEBUG:
        {
            if(verboseOutput)
            {
                output_line = QString("[DEBUG][%1] %2%3\n").arg(current_time).arg(line).arg(message);
            }
            break;
        }
        case LOG_TYPE_INFO:
        {
            output_line = QString("[INFO ][%1] %2%3\n").arg(current_time).arg(line).arg(message);
            break;
        }
        case LOG_TYPE_WARN:
        {
            output_line = QString("[WARN ][%1] %2%3\n").arg(current_time).arg(line).arg(message);
            break;
        }
        case LOG_TYPE_CRITICAL:
        {
            output_channel = stderr;
            output_line = QString("[CRIT ][%1] %2%3\n").arg(current_time).arg(line).arg(message);
            fprintf(output_channel, "%s", output_line.toUtf8().constData());

            SDK::Core::printCallStack();
            break;
        }
        case LOG_TYPE_FATAL:
        {
            output_channel = stderr;
            output_line = QString("[FATAL][%1] %2%3\n").arg(current_time).arg(line).arg(message);
            fprintf(output_channel, "%s", output_line.toUtf8().constData());

            SDK::Core::printCallStack();
            abort();
        }
        case LOG_TYPE_STUB:
        {
            if(context.function != NULL || !(message == NULL || message.length() == 0))
            {
                output_line = QString("[STUB ][%1] %2%3%4%5\n").arg(current_time).arg(line).arg(context.function).arg(message.length() > 0 ? ": " : "", message);
            }
            break;
        }
        case LOG_TYPE_LOG:
        {
            output_line = QString("[LOG  ][%1] %2%3\n").arg(current_time).arg(line).arg(message);
            break;
        }
        case LOG_TYPE_FIXME:
        {
            output_line = QString("[FIXME][%1] (%2:%3): %4\n").arg(current_time).arg(context.function).arg(QString::number(context.line)).arg(message);
            break;
        }
        case LOG_TYPE_BUG:
        {
            output_channel = stderr;
            output_line = QString("[BUG  ][%1] (%2:%3) %4 -> %5\n").arg(current_time).arg(context.file).arg(context.line).arg(context.function).arg(message);
            break;
        }
        case LOG_TYPE_WTF:
        {
            output_channel = stderr;
            output_line = QString("[WTF  ][%1] (%2:%3) %4 -> %5\n").arg(current_time).arg(context.file).arg(QString::number(context.line)).arg(context.function).arg(message);
            break;
        }
        default:
        {
            output_line = QString( "[OTHER][%1] (%2:%3 | %4): %5\n").arg(current_time).arg(context.file).arg(QString::number(context.line)).arg(context.function).arg(message);
        }

    }

#ifdef QT_DEBUG
    static bool colorOutput = false;
#else
    static bool colorOutput = qApp->arguments().contains("--color");
#endif

    if(colorOutput)
    {
        // Sometimes crashes the app on exit
        //output_line = colorize(output_line);
    }

    fprintf(output_channel, "%s", output_line.toUtf8().constData());
    if(LoggerCore::m_log_file != NULL && LoggerCore::m_log_file->isOpen() && m_log_file->isWritable())
        LoggerCore::m_log_file->write(output_line.toUtf8());

    fflush(stdout);
    fflush(stderr);
}

QString LoggerCore::colorize(const QString &str)
{
    static int debugColor = 36;
    static int stubColor = 32;
    static int logColor = 37;
    static int errorColor = 31;
    static int infoColor = 35;
    //static int methodColor = 33;
    //static int methodParamsColor  = 32;
    //static int normalColor = 39;

    static QRegExp debugRegExp = QRegExp("^(?:\\[)(DEBUG)(?:\\s*\\])");
    static QRegExp stubRegExp = QRegExp("^(?:\\[)(STUB)(?:\\s*\\])");
    static QRegExp bugRegExp = QRegExp("^(?:\\[)(BUG)(?:\\s*\\])");
    static QRegExp warnRegExp = QRegExp("^(?:\\[)(WARN)(?:\\s*\\])");
    static QRegExp logRegExp = QRegExp("^(?:\\[)(LOG)(?:\\s*\\])");
    static QRegExp errorRegExp = QRegExp("^(?:\\[)(ERROR)(?:\\s*\\])");
    static QRegExp fatalRegExp = QRegExp("^(?:\\[)(FATAL)(?:\\s*\\])");
    static QRegExp infoRegExp = QRegExp("^(?:\\[)(INFO)(?:\\s*\\])");
    //static QRegExp methodCallRegExp = QRegExp("(\\w+)(::)(\\w+)(?:(\\s*\\()(.*)(\\))){0,1}");

    try {
        QString result = str;
        result.replace(debugRegExp, Colorizer::format("[DEBUG]", debugColor));
        result.replace(stubRegExp,  Colorizer::format("[STUB ]", stubColor));
        result.replace(bugRegExp,   Colorizer::format("[BUG  ]", stubColor));
        result.replace(warnRegExp,  Colorizer::format("[WARN ]", stubColor));
        result.replace(logRegExp,   Colorizer::format("[LOG  ]", logColor));
        result.replace(errorRegExp, Colorizer::format("[ERROR]", errorColor));
        result.replace(fatalRegExp, Colorizer::format("[FATAL]", errorColor));
        result.replace(infoRegExp,  Colorizer::format("[INFO ]", infoColor));

        /*QString methodCall = QString("%1%2%3%4%5%6")
                .arg(Colorizer::format("\\1", methodColor))         // class
                .arg(Colorizer::format("\\2", stubColor))           // ::
                .arg(Colorizer::format("\\3", methodColor))         // methodName
                .arg(Colorizer::format("\\4", normalColor))         // (
                .arg(Colorizer::format("\\5", methodParamsColor))   // params
                .arg(Colorizer::format("\\6", normalColor));        // )*/

        //result.replace(methodCallRegExp, methodCall);
        return result;
    }
    catch(std::exception e)
    {
        fprintf(stderr, "%s", e.what());
    }
    return str;
}

void LoggerCore::initLogFile(QObject* parent)
{
    for(const QString& arg: qApp->arguments())
    {
        if(arg.startsWith("--log"))
        {
            QStringList data = arg.split('=');
            if(data.length() != 2)
            {
                qWarning() << qPrintable(QString("Incorrect argument %1!").arg(arg));
            }
            else
            {
                qDebug() << "Trying to open log file " << data.at(1);
                LoggerCore::m_log_file = new QFile(parent);
                LoggerCore::m_log_file->setFileName(data.at(1));
                QFileInfo f_info(*LoggerCore::m_log_file);
                QDir dir = f_info.absoluteDir();
                if(!dir.exists())
                {
                    bool ok = dir.mkdir(dir.absolutePath());
                    if(!ok)
                    {
                        qWarning() << qPrintable(QString("Cannot create a directory %1 to write logs!").arg(dir.absolutePath()));
                        return;
                    }
                }
                if(!LoggerCore::m_log_file->open(QFile::ReadWrite | QIODevice::Text))
                {
                    qWarning() << qPrintable(QString("Cannot open a log file %1!").arg(LoggerCore::m_log_file->fileName()));
                }
            }
            break;
        }
    }
}
