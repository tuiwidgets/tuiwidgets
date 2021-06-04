#include "ZSimpleFileLogger.h"

#include <QFile>
#include <QDateTime>

TUIWIDGETS_NS_START

namespace {
    QString logFileName;

    void fileMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
        (void)type; (void)context;
        QByteArray localMsg = (QDateTime::currentDateTimeUtc().toString(Qt::ISODate) + QStringLiteral(" ") + msg).toUtf8() + "\n";
        QFile f(logFileName);
        f.open(QIODevice::Append | QIODevice::WriteOnly);
        f.write(localMsg);
    }
}

void ZSimpleFileLogger::install(const QString &path) {
    logFileName = path;
    qInstallMessageHandler(fileMessageOutput);
}

TUIWIDGETS_NS_END
