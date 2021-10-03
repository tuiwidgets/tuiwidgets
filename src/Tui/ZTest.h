#ifndef TUIWIDGETS_ZTEST_INCLUDED
#define TUIWIDGETS_ZTEST_INCLUDED

#include <QString>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZImage;
class ZTerminal;
class ZWidget;

namespace ZTest {

    TUIWIDGETS_EXPORT void sendText(ZTerminal *terminal, QString text, Qt::KeyboardModifiers modifiers);
    TUIWIDGETS_EXPORT void sendKey(ZTerminal *terminal, Qt::Key key, Qt::KeyboardModifiers modifiers);
    TUIWIDGETS_EXPORT void sendKeyToWidget(ZWidget *w, Qt::Key key, Qt::KeyboardModifiers modifiers);

    TUIWIDGETS_EXPORT ZImage waitForNextRenderAndGetContents(ZTerminal *terminal);
}

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZTEST_INCLUDED
