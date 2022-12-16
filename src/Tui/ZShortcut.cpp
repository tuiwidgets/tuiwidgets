// SPDX-License-Identifier: BSL-1.0

#include "ZShortcut.h"
#include <Tui/ZShortcut_p.h>

#include <Tui/ZWidget_p.h>
#include <Tui/ZWidget.h>

#include <Tui/ZTerminal_p.h>
#include <Tui/ZShortcutManager_p.h>
#include <Tui/ZSymbol.h>

TUIWIDGETS_NS_START

ZShortcutPrivate::ZShortcutPrivate() {
}

ZShortcutPrivate::~ZShortcutPrivate() {
}

ZShortcutPrivate *ZShortcutPrivate::get(ZShortcut *shortcut) {
    return shortcut->tuiwidgets_impl();
}

const ZShortcutPrivate *ZShortcutPrivate::get(const ZShortcut *shortcut) {
    return shortcut->tuiwidgets_impl();
}

ZShortcut::ZShortcut(const ZKeySequence &key, ZWidget *parent, ShortcutContext context)
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

bool ZShortcutPrivate::isContextActive(QObject *par, ZWidget *focusWidget) const {
    switch (context) {
        case WidgetShortcut:
            return focusWidget == par;
            break;
        case WidgetWithChildrenShortcut:
            while (focusWidget) {
                if (focusWidget == par) {
                    return true;
                }
                focusWidget = focusWidget->parentWidget();
            }
            break;
        case WindowShortcut:
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
        case ApplicationShortcut:
            return true;
            break;
    }
    return false;
}

bool ZShortcut::matches(ZWidget *focusWidget, const ZKeyEvent *event) const {
    auto *const p = tuiwidgets_impl();
    if (!isEnabled()) return false;
    bool keyMatches = false;
    auto *const kp = p->key.tuiwidgets_impl();
    if (kp->forMnemonic.size()) {
        if (event->modifiers() == AltModifier && event->text().toLower() == kp->forMnemonic.toLower()) {
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

ZKeySequencePrivate::ZKeySequencePrivate() {
}

ZKeySequencePrivate::~ZKeySequencePrivate() {
}

ZKeySequencePrivate *ZKeySequencePrivate::get(ZKeySequence *keyseq) {
    return keyseq->tuiwidgets_impl();
}

const ZKeySequencePrivate *ZKeySequencePrivate::get(const ZKeySequence *keyseq) {
    return keyseq->tuiwidgets_impl();
}

ZKeySequence::ZKeySequence() = default;
ZKeySequence::ZKeySequence(const ZKeySequence&) = default;

ZKeySequence::~ZKeySequence() = default;

ZKeySequence& ZKeySequence::operator=(const ZKeySequence&) = default;


ZKeySequence ZKeySequence::forMnemonic(const QString &c) {
    ZKeySequence s;
    auto *const p = s.tuiwidgets_impl();
    p->forMnemonic = c;
    return s;
}

ZKeySequence ZKeySequence::forKey(int key, KeyboardModifiers modifiers) {
    ZKeySequence s;
    auto *const p = s.tuiwidgets_impl();
    p->forKey = key;
    p->modifiers = modifiers;
    return s;
}

ZKeySequence ZKeySequence::forShortcut(const QString &c, KeyboardModifiers modifiers) {
    ZKeySequence s;
    auto *const p = s.tuiwidgets_impl();
    p->forShortcut = c;
    p->modifiers = modifiers;
    return s;
}

ZKeySequence ZKeySequence::forShortcutSequence(const QString &c, KeyboardModifiers modifiers, const QString &c2, KeyboardModifiers modifiers2) {
    ZKeySequence s;
    auto *const p = s.tuiwidgets_impl();
    p->forShortcut = c;
    p->modifiers = modifiers;
    p->forShortcut2 = c2;
    p->modifiers2 = modifiers2;
    return s;
}

ZKeySequence ZKeySequence::forShortcutSequence(const QString &c, KeyboardModifiers modifiers, int key2, KeyboardModifiers modifiers2) {
    ZKeySequence s;
    auto *const p = s.tuiwidgets_impl();
    p->forShortcut = c;
    p->modifiers = modifiers;
    p->forKey2 = key2;
    p->modifiers2 = modifiers2;
    return s;
}


ZPendingKeySequenceCallbacksPrivate::ZPendingKeySequenceCallbacksPrivate() {
}

ZPendingKeySequenceCallbacksPrivate::~ZPendingKeySequenceCallbacksPrivate() {
}

ZPendingKeySequenceCallbacksPrivate *ZPendingKeySequenceCallbacksPrivate::get(ZPendingKeySequenceCallbacks *callbacks) {
    return callbacks->tuiwidgets_impl();
}

const ZPendingKeySequenceCallbacksPrivate *ZPendingKeySequenceCallbacksPrivate::get(const ZPendingKeySequenceCallbacks *callbacks) {
    return callbacks->tuiwidgets_impl();
}

ZPendingKeySequenceCallbacks::ZPendingKeySequenceCallbacks() = default;
ZPendingKeySequenceCallbacks::ZPendingKeySequenceCallbacks(const ZPendingKeySequenceCallbacks&) = default;

ZPendingKeySequenceCallbacks::~ZPendingKeySequenceCallbacks() = default;

ZPendingKeySequenceCallbacks& ZPendingKeySequenceCallbacks::operator=(const ZPendingKeySequenceCallbacks&) = default;


void ZPendingKeySequenceCallbacks::setPendingSequenceStarted(std::function<void ()> callback) {
    auto *const p = tuiwidgets_impl();
    p->started = callback;
}

void ZPendingKeySequenceCallbacks::setPendingSequenceFinished(std::function<void(bool)> callback) {
    auto *const p = tuiwidgets_impl();
    p->finished = callback;
}

TUIWIDGETS_NS_END
