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
    if (qobject_cast<ZWidget*>(parent())
            && !qobject_cast<ZWidget*>(parent())->isEnabled()) {
        return false;
    }
    return p->enabled;
}

void ZShortcut::setEnabled(bool enable) {
    auto *const p = tuiwidgets_impl();
    p->enabled = enable;
}

bool ZShortcut::matches(ZWidget *focusWidget, const ZKeyEvent *event) {
    auto *const p = tuiwidgets_impl();
    if (!isEnabled()) return false;
    bool keyMatches = false;
    if (p->key._forMnemonic.size()) {
        if (event->modifiers() == Qt::AltModifier && event->text().toLower() == p->key._forMnemonic.toLower()) {
            keyMatches = true;
        }
    } else if (p->key._forKey != 0) {
        if (event->modifiers() == p->key._modifiers && event->key() == p->key._forKey) {
            keyMatches = true;
        }
    } else if (p->key._forShortcut.size()) {
        if (event->modifiers() == p->key._modifiers && event->text() == p->key._forShortcut) {
            keyMatches = true;
        }
    }
    if (!keyMatches) {
        return false;
    }
    QObject *par = parent();
    switch (p->context) {
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
                while (window->parent()) {
                    if (window->paletteClass().contains(QStringLiteral("window"))) {
                        break;
                    }
                    window = window->parentWidget();
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

bool ZShortcut::event(QEvent *event) {
    auto *const p = tuiwidgets_impl();
    if (event->type() == ZEventType::otherChange()
            && !static_cast<ZOtherChangeEvent*>(event)->unchanged().contains(TUISYM_LITERAL("terminal"))) {
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

ZKeySequence::ZKeySequence() {
}

ZKeySequence ZKeySequence::forMnemonic(const QString &c) {
    ZKeySequence s;
    s._forMnemonic = c;
    return s;
}

ZKeySequence ZKeySequence::forKey(int key, Qt::KeyboardModifiers modifiers) {
    ZKeySequence s;
    s._forKey = key;
    s._modifiers = modifiers;
    return s;
}

ZKeySequence ZKeySequence::forShortcut(const QString &c, Qt::KeyboardModifiers modifiers) {
    ZKeySequence s;
    s._forShortcut = c;
    s._modifiers = modifiers;
    return s;
}

TUIWIDGETS_NS_END
