// SPDX-License-Identifier: BSL-1.0

#include "ZTest.h"

#include <QSet>

#include <Tui/ZEvent.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZTerminal_p.h>
#include <Tui/ZWidget.h>
#include <Tui/ZImage.h>

#include <QCoreApplication>

TUIWIDGETS_NS_START

namespace ZTest {

    TUIWIDGETS_EXPORT void sendText(ZTerminal *terminal, const QString &text, KeyboardModifiers modifiers) {
        if (text == QStringLiteral(" ")) {
            qFatal("sendText can not be used with \" \", use sendKey instead.");
            return;
        }
        if (text == QStringLiteral("\n")) {
            qFatal("sendText can not be used with \"\\n\", use sendKey instead.");
            return;
        }

        ZKeyEvent event(Key_unknown, modifiers, text);
        terminal->dispatchKeyboardEvent(event);
    }

    TUIWIDGETS_EXPORT void sendKey(ZTerminal *terminal, Key key, KeyboardModifiers modifiers) {
        ZKeyEvent event(key, modifiers, {});
        terminal->dispatchKeyboardEvent(event);
    }

    TUIWIDGETS_EXPORT void sendKeyToWidget(ZWidget *w, Key key, KeyboardModifiers modifiers) {
        ZKeyEvent event(key, modifiers, {});
        w->event(&event);
    }

    void sendPaste(ZTerminal *terminal, const QString &text) {
        ZPasteEvent event(text);
        terminal->dispatchPasteEvent(event);
    }

    TUIWIDGETS_EXPORT ZImage waitForNextRenderAndGetContents(ZTerminal *terminal) {

        std::unique_ptr<ZImage> result;

        QMetaObject::Connection connection =
                QObject::connect(terminal, &ZTerminal::afterRendering, terminal,
                                 [&result, terminal] {
            result = std::make_unique<ZImage>(terminal->grabCurrentImage());
        });

        while (!result) {
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }

        QObject::disconnect(connection);
        return std::move(*result);
    }

    TUIWIDGETS_EXPORT void withLayoutRequestTracking(ZTerminal *terminal, std::function<void (QSet<ZWidget*>*)> closure) {
        auto *const p = ZTerminalPrivate::get(terminal);
        QSet<ZWidget*> layoutRequested;
        if (!p->setTestLayoutRequestTracker([&layoutRequested](ZWidget *w) { layoutRequested.insert(w);})) {
            qWarning("nested layout request tracking not supported");
            return;
        }
        try {
            closure(&layoutRequested);
        } catch (...) {
            p->resetTestLayoutRequestTracker();
            throw;
        }
        p->resetTestLayoutRequestTracker();
    }

}

TUIWIDGETS_NS_END
