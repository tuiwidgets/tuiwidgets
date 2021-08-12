#include "ZCommandNotifier.h"

#include "ZCommandNotifier_p.h"

#include <QSet>

#include <Tui/ZCommandManager.h>
#include <Tui/ZSymbol.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZWidget.h>

TUIWIDGETS_NS_START

ZCommandNotifier::ZCommandNotifier(ZImplicitSymbol command, QObject *parent)
    : ZCommandNotifier(command, parent, Qt::ApplicationShortcut)
{
}

ZCommandNotifier::ZCommandNotifier(ZImplicitSymbol command, QObject *parent, Qt::ShortcutContext context)
    : QObject(parent), tuiwidgets_pimpl_ptr(std::make_unique<ZCommandNotifierPrivate>())
{
    auto *const p = tuiwidgets_impl();
    p->command = command;
    p->context = context;
    probeParents();
    p->connectToTerminal(this);
    if (context != Qt::ApplicationShortcut) {
        p->updateContextSatisfied(this);
    }
    enabledChanged(p->enabled && p->contextSatisfied);
}

ZCommandNotifier::~ZCommandNotifier() {
    // Notify ZCommandManager
    setEnabled(false);
}

void ZCommandNotifierPrivate::connectToTerminal(ZCommandNotifier *pub) {
    if (context != Qt::ApplicationShortcut) {
        ZWidget *pw = qobject_cast<ZWidget*>(pub->parent());
        if (pw) {
            ZTerminal *term = pw->terminal();
            if (term) {
                QObject::disconnect(focusChangedConnection);
                focusChangedConnection = QObject::connect(term, &ZTerminal::focusChanged, pub, [this, pub] {
                    updateContextSatisfied(pub);
                });
                // connect to before render to cope with reparenting in the widget hierarchy
                QObject::disconnect(beforeRenderingConnection);
                beforeRenderingConnection = QObject::connect(term, &ZTerminal::beforeRendering, pub, [this, pub] {
                    updateContextSatisfied(pub);
                });
            }
        }
    }
}

void ZCommandNotifierPrivate::updateContextSatisfied(Tui::ZCommandNotifier *pub) {
    const bool effective = enabled && contextSatisfied;
    ZWidget *par = qobject_cast<ZWidget*>(pub->parent());
    if (!par) {
        contextSatisfied = false;
        return;
    }
    ZTerminal *term = par->terminal();
    if (!term) {
        contextSatisfied = false;
        return;
    }
    ZWidget *focusWidget = term->focusWidget();
    if (!focusWidget) {
        contextSatisfied = false;
        return;
    }
    switch (context) {
        case Qt::WidgetShortcut:
            contextSatisfied = (focusWidget == par);
            break;
        case Qt::WidgetWithChildrenShortcut:
            contextSatisfied = false;
            while (focusWidget) {
                if (focusWidget == par) {
                    contextSatisfied = true;
                    break;
                }
                focusWidget = focusWidget->parentWidget();
            }
            break;
        case Qt::WindowShortcut:
            {
                contextSatisfied = false;
                ZWidget *window = focusWidget;
                if (!window) {
                    contextSatisfied = false;
                }
                while (window->parent()) {
                    if (window->paletteClass().contains(QStringLiteral("window"))) {
                        break;
                    }
                    window = window->parentWidget();
                }
                QObject *w = par;
                while (w) {
                    if (w == window) {
                        contextSatisfied = true;
                        break;
                    }
                    w = w->parent();
                }
            }
            break;
        case Qt::ApplicationShortcut:
            contextSatisfied = true;
            break;
    }

    if (effective != (enabled && contextSatisfied)) {
        Q_EMIT pub->enabledChanged(enabled && contextSatisfied);
    }
}

ZSymbol ZCommandNotifier::command() {
    auto *const p = tuiwidgets_impl();
    return p->command;
}

Qt::ShortcutContext ZCommandNotifier::context() {
    auto *const p = tuiwidgets_impl();
    return p->context;
}

bool ZCommandNotifier::isEnabled() {
    auto *const p = tuiwidgets_impl();
    return p->enabled;
}

void ZCommandNotifier::setEnabled(bool s) {
    auto *const p = tuiwidgets_impl();
    if (p->enabled == s) {
        return;
    }
    p->enabled = s;
    if (p->contextSatisfied) {
        enabledChanged(s && p->contextSatisfied);
    }
}

bool ZCommandNotifier::isContextSatisfied() {
    auto *const p = tuiwidgets_impl();
    return p->contextSatisfied;
}

void ZCommandNotifier::probeParents() {
    ZWidget* w = qobject_cast<ZWidget*>(parent());
    while (w) {
        if (w->commandManager()) {
            w->commandManager()->registerCommandNotifier(this);
        }
        w = w->parentWidget();
    }
}

bool ZCommandNotifier::event(QEvent *event) {
    auto *const p = tuiwidgets_impl();
    if (event->type() == Tui::ZEventType::otherChange()) {
        if (!static_cast<Tui::ZOtherChangeEvent*>(event)->unchanged().contains(TUISYM_LITERAL("terminal"))) {
            p->connectToTerminal(this);
        }
    }
    return QObject::event(event);
}

bool ZCommandNotifier::eventFilter(QObject *watched, QEvent *event) {
    return QObject::eventFilter(watched, event);
}

void ZCommandNotifier::timerEvent(QTimerEvent *event) {
    QObject::timerEvent(event);
}

void ZCommandNotifier::childEvent(QChildEvent *event) {
    QObject::childEvent(event);
}

void ZCommandNotifier::customEvent(QEvent *event) {
    QObject::customEvent(event);
}

void ZCommandNotifier::connectNotify(const QMetaMethod &signal) {
    // XXX needs to be thread-safe
    QObject::connectNotify(signal);
}

void ZCommandNotifier::disconnectNotify(const QMetaMethod &signal) {
    // XXX needs to be thread-safe
    QObject::disconnectNotify(signal);
}

TUIWIDGETS_NS_END
