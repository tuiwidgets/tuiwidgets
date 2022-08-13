// SPDX-License-Identifier: BSL-1.0

#include "ZSimpleStringLogger.h"

TUIWIDGETS_NS_START

namespace {
    static QString qtLogMessages;

    void qtMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
        (void)type; (void)context;
        qtLogMessages += msg + QStringLiteral("\n");
    }

}

void ZSimpleStringLogger::install() {
    qInstallMessageHandler(qtMessageOutput);
}

void ZSimpleStringLogger::clearMessages() {
    qtLogMessages = QStringLiteral("");
}

QString ZSimpleStringLogger::getMessages() {
    return qtLogMessages;
}

TUIWIDGETS_NS_END
