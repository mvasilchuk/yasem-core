#include "core.h"
#include "loggercore.h"

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

using namespace yasem;

LoggerCore::LoggerCore(QObject *parent) :
    QObject(parent)
{

}

void LoggerCore::MessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
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

    const char* message = localMsg.constData();

    switch (msgType) {
        case LOG_TYPE_DEBUG:
        {
            fprintf(stdout, "[DEBUG] %s\n", message);
            break;
        }
        case LOG_TYPE_WARN:
        {
            fprintf(stdout, "[WARN ] (%s:%u): %s\n", context.file, context.line, message);
            break;
        }
        case LOG_TYPE_CRITICAL:
        {
            fprintf(stderr, "[CRIT ]: (%s:%u): %s\n", context.file, context.line, message);
            Core::printCallStack();
            break;
        }
        case LOG_TYPE_FATAL:
        {
            fprintf(stderr, "[FATAL] (%s:%u): %s\n", context.file, context.line, message);
            Core::printCallStack();
            abort();
            break;
        }
        case LOG_TYPE_STUB:
        {
            fprintf(stdout, "[STUB ] %s%s%s\n", context.function, strlen(message) > 0 ? ": " : "", message);
            break;
        }
        case LOG_TYPE_LOG:
        {
            fprintf(stdout, "[LOG  ] %s\n", message);
            break;
        }
        case LOG_TYPE_FIXME:
        {
            fprintf(stdout, "[FIXME] (%s:%u): %s\n", context.function, context.line, message);
            break;
        }
        case LOG_TYPE_BUG:
        {
            fprintf(stderr, "[BUG  ] (%s:%u) %s -> %s\n", context.file, context.line, context.function, message);
            break;
        }
        case LOG_TYPE_WTF:
        {
            fprintf(stderr, "[WTF  ] (%s:%u) %s -> %s\n", context.file, context.line, context.function, message);
            break;
        }
        default:
        {
            fprintf(stdout, "[OTHER] (%s:%u | %s): %s\n", context.file, context.line, context.function, message);
        }

    }

    fflush(stdout);
    fflush(stderr);
}
