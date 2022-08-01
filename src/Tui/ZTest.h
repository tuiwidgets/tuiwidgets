#ifndef TUIWIDGETS_ZTEST_INCLUDED
#define TUIWIDGETS_ZTEST_INCLUDED

#include <functional>

#include <QString>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZImage;
class ZTerminal;
class ZWidget;

namespace ZTest {

    TUIWIDGETS_EXPORT void sendText(ZTerminal *terminal, const QString &text, Qt::KeyboardModifiers modifiers);
    TUIWIDGETS_EXPORT void sendKey(ZTerminal *terminal, Qt::Key key, Qt::KeyboardModifiers modifiers);
    TUIWIDGETS_EXPORT void sendKeyToWidget(ZWidget *w, Qt::Key key, Qt::KeyboardModifiers modifiers);
    TUIWIDGETS_EXPORT void sendPaste(ZTerminal *terminal, const QString &text);

    TUIWIDGETS_EXPORT ZImage waitForNextRenderAndGetContents(ZTerminal *terminal);

    TUIWIDGETS_EXPORT void withLayoutRequestTracking(ZTerminal *terminal,
                                                     std::function<void(QSet<ZWidget*> *layoutRequested)> closure);
}

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZTEST_INCLUDED
