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

    TUIWIDGETS_EXPORT void sendText(ZTerminal *terminal, QString text, Qt::KeyboardModifiers modifiers) {
        ZKeyEvent event(Qt::Key_unknown, modifiers, text);
        terminal->dispatchKeyboardEvent(event);
    }

    TUIWIDGETS_EXPORT void sendKey(ZTerminal *terminal, Qt::Key key, Qt::KeyboardModifiers modifiers) {
        ZKeyEvent event(key, modifiers, {});
        terminal->dispatchKeyboardEvent(event);
    }

    TUIWIDGETS_EXPORT void sendKeyToWidget(ZWidget *w, Qt::Key key, Qt::KeyboardModifiers modifiers) {
        ZKeyEvent event(key, modifiers, {});
        w->event(&event);
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
    }

}

TUIWIDGETS_NS_END
