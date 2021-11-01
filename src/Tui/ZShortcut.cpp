#include "ZShortcut.h"
#include <Tui/ZShortcut_p.h>

#include <Tui/ZWidget_p.h>
#include <Tui/ZWidget.h>

#include <Tui/ZTerminal_p.h>
#include <Tui/ZShortcutManager_p.h>
#include <Tui/ZSymbol.h>

TUIWIDGETS_NS_START

ZShortcut::ZShortcut(const ZKeySequence &key, ZWidget *parent, Qt::ShortcutContext context)
    : QObject(parent), tuiwidgets_pimpl_ptr(std::make_unique<ZShortcutPrivate>())
{
    auto *const p = tuiwidgets_impl();
    p->key = key;
    p->context = context;

    p->terminal = parent->terminal();
    if (p->terminal) {
        ZTerminalPrivate::get(p->terminal)->ensureShortcutManager()->addShortcut(this);
    }
}

ZShortcut::~ZShortcut() {
    auto *const p = tuiwidgets_impl();
    if (p->terminal) {
        ZTerminalPrivate::get(p->terminal)->ensureShortcutManager()->removeShortcut(this);
    }
}

bool ZShortcut::isEnabled() const {
    auto *const p = tuiwidgets_impl();
    if (!p->enabledDelegate) {
        if (qobject_cast<ZWidget*>(parent())
                && !qobject_cast<ZWidget*>(parent())->isEnabled()) {
            return false;
        }
    } else {
        if (!p->enabledDelegate()) {
            return false;
        }
    }
    return p->enabled;
}

void ZShortcut::setEnabled(bool enable) {
    auto *const p = tuiwidgets_impl();
    p->enabled = enable;
}

void ZShortcut::setEnabledDelegate(Private::ZMoFunc<bool()>&& delegate) {
    auto *const p = tuiwidgets_impl();
    p->enabledDelegate = std::move(delegate);
}

bool ZShortcutPrivate::isContextActive(QObject *par, ZWidget *focusWidget) {
    switch (context) {
        case Qt::WidgetShortcut:
            return focusWidget == par;
            break;
        case Qt::WidgetWithChildrenShortcut:
            while (focusWidget) {
                if (focusWidget == par) {
                    return true;
                }
                focusWidget = focusWidget->parentWidget();
            }
            break;
        case Qt::WindowShortcut:
            {
                ZWidget *window = focusWidget;
                if (!window) {
                    return false;
                }
                while (window) {
                    if (window->paletteClass().contains(QStringLiteral("window"))) {
                        break;
                    }
                    window = window->parentWidget();
                }
                if (!window) {
                    return false;
                }
                QObject *w = par;
                while (w) {
                    if (w == window) {
                        return true;
                    }
                    w = w->parent();
                }
            }
            break;
        case Qt::ApplicationShortcut:
            return true;
            break;
    }
    return false;
}

bool ZShortcut::matches(ZWidget *focusWidget, const ZKeyEvent *event) {
    auto *const p = tuiwidgets_impl();
    if (!isEnabled()) return false;
    bool keyMatches = false;
    auto *const kp = p->key.tuiwidgets_impl();
    if (kp->forMnemonic.size()) {
        if (event->modifiers() == Qt::AltModifier && event->text().toLower() == kp->forMnemonic.toLower()) {
            keyMatches = true;
        }
    } else if (kp->forKey != 0) {
        if (event->modifiers() == kp->modifiers && event->key() == kp->forKey) {
            keyMatches = true;
        }
    } else if (kp->forShortcut.size()) {
        if (event->modifiers() == kp->modifiers && event->text() == kp->forShortcut) {
            keyMatches = true;
        }
    }
    if (!keyMatches) {
        return false;
    }
    QObject *par = parent();
    return p->isContextActive(par, focusWidget);
}

bool ZShortcut::event(QEvent *event) {
    auto *const p = tuiwidgets_impl();
    if (ZOtherChangeEvent::match(event, TUISYM_LITERAL("terminal"))) {
        auto newTerminal = static_cast<ZWidget*>(parent())->terminal();
        if (p->terminal != newTerminal) {
            if (p->terminal) {
                ZTerminalPrivate::get(p->terminal)->ensureShortcutManager()->removeShortcut(this);
            }
            p->terminal = newTerminal;
            if (p->terminal) {
                ZTerminalPrivate::get(p->terminal)->ensureShortcutManager()->addShortcut(this);
            }
        }
    }
    return QObject::event(event);
}

bool ZShortcut::eventFilter(QObject *watched, QEvent *event) {
    return QObject::eventFilter(watched, event);
}

void ZShortcut::timerEvent(QTimerEvent *event) {
    QObject::timerEvent(event);
}

void ZShortcut::childEvent(QChildEvent *event) {
    QObject::childEvent(event);
}

void ZShortcut::customEvent(QEvent *event) {
    QObject::customEvent(event);
}

void ZShortcut::connectNotify(const QMetaMethod &signal) {
    // XXX needs to be thread-safe
    QObject::connectNotify(signal);
}

void ZShortcut::disconnectNotify(const QMetaMethod &signal) {
    // XXX needs to be thread-safe
    QObject::disconnectNotify(signal);
}

ZKeySequence::ZKeySequence() = default;

ZKeySequence::~ZKeySequence() = default;

ZKeySequence ZKeySequence::forMnemonic(const QString &c) {
    ZKeySequence s;
    auto *const p = s.tuiwidgets_impl();
    p->forMnemonic = c;
    return s;
}

ZKeySequence ZKeySequence::forKey(int key, Qt::KeyboardModifiers modifiers) {
    ZKeySequence s;
    auto *const p = s.tuiwidgets_impl();
    p->forKey = key;
    p->modifiers = modifiers;
    return s;
}

ZKeySequence ZKeySequence::forShortcut(const QString &c, Qt::KeyboardModifiers modifiers) {
    ZKeySequence s;
    auto *const p = s.tuiwidgets_impl();
    p->forShortcut = c;
    p->modifiers = modifiers;
    return s;
}

TUIWIDGETS_NS_END
