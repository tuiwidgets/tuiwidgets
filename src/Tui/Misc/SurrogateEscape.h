#ifndef TUIWIDGETS_MISC_SURROGATEESCAPE_INCLUDED
#define TUIWIDGETS_MISC_SURROGATEESCAPE_INCLUDED

#include <QString>
#include <QByteArray>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

namespace Misc {

namespace SurrogateEscape {
    TUIWIDGETS_EXPORT QString decode(const QByteArray& data);
    TUIWIDGETS_EXPORT QString decode(const char *data, int len);
    TUIWIDGETS_EXPORT QByteArray encode(const QString& str);
    TUIWIDGETS_EXPORT QByteArray encode(const QChar* str, int len);
}

}

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_MISC_SURROGATEESCAPE_INCLUDED
