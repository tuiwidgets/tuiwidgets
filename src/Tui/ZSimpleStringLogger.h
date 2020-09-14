#ifndef ZSIMPLESTRINGLOGGER_H
#define ZSIMPLESTRINGLOGGER_H

#include <QString>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

namespace ZSimpleStringLogger {
    TUIWIDGETS_EXPORT void clearMessages();
    TUIWIDGETS_EXPORT QString getMessages();
    TUIWIDGETS_EXPORT void install();
}

TUIWIDGETS_NS_END

#endif // ZSIMPLESTRINGLOGGER_H
