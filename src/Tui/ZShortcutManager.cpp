#include "ZShortcutManager_p.h"

#include <QPointer>
#include <QVector>

#include <Tui/ZEvent.h>
#include <Tui/ZShortcut.h>
#include <Tui/ZShortcut_p.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZWidget.h>

TUIWIDGETS_NS_START

ZShortcutManager::ZShortcutManager(ZTerminal *terminal)
    : terminal(terminal)
{
}

ZShortcutManager::~ZShortcutManager() {
}

void ZShortcutManager::addShortcut(ZShortcut *s) {
    ZShortcutPrivate::get(s);
    auto *kp = ZKeySequencePrivate::get(&ZShortcutPrivate::get(s)->key);
    if (kp->forShortcut2.size() || kp->forKey2) {
        Key key{kp->forShortcut, kp->modifiers};
        twoPartShortcuts[key].insert(s);
    } else {
        shortcuts.insert(s);
    }
}

void ZShortcutManager::removeShortcut(ZShortcut *s) {
    auto *kp = ZKeySequencePrivate::get(&ZShortcutPrivate::get(s)->key);
    if (kp->forShortcut2.size() || kp->forKey2) {
        Key key{kp->forShortcut, kp->modifiers};
        twoPartShortcuts[key].remove(s);
        if (twoPartShortcuts[key].isEmpty()) {
            twoPartShortcuts.remove(key);
        }
    } else {
        shortcuts.remove(s);
    }
}

bool ZShortcutManager::process(const ZKeyEvent *event) {
    ZWidget *focusWidget = terminal->focusWidget();

    if (focusWidget) {
        for (const auto &key: twoPartShortcuts.keys()) {
            if (key.c.size() && event->text() == key.c && event->modifiers() == key.modifiers) {
                activateTwoPart(key);
                return true;
            }
        }
    }

    QVector<QPointer<ZShortcut>> matching;
    for (ZShortcut *s : shortcuts) {
        if (s->matches(focusWidget, event)) {
            matching.append(s);
        }
    }
    if (matching.size() == 0) {
        return false;
    }
    if (matching.size() == 1) {
        matching[0]->activated();
    } else {
        // TODO ambiguous
    }
    return true;
}

void ZShortcutManager::activateTwoPart(const Key &prefix) {
    ZWidget *focusWidget = terminal->focusWidget();
    for (auto &callbacks : pendingCallbacks) {
        ZPendingKeySequenceCallbacksPrivate::get(&callbacks)->started();
    }
    focusWidget->grabKeyboard([this, prefix, focusWidget] (QEvent* event) {
        if (event->type() == ZEventType::key()) {
            auto *keyEvent = static_cast<ZKeyEvent*>(event);
            QVector<QPointer<ZShortcut>> matching;
            for (ZShortcut *s : twoPartShortcuts[prefix]) {
                auto *sp = ZShortcutPrivate::get(s);
                auto *kp = ZKeySequencePrivate::get(&sp->key);
                if (kp->forShortcut2.size()
                   && keyEvent->text() == kp->forShortcut2 && keyEvent->modifiers() == kp->modifiers2
                   && sp->isContextActive(s->parent(), focusWidget)) {
                    matching.append(s);
                }
                if (kp->forKey2 != 0
                   && keyEvent->key() == kp->forKey2 && keyEvent->modifiers() == kp->modifiers2
                   && sp->isContextActive(s->parent(), focusWidget)) {
                    matching.append(s);
                }
            }
            focusWidget->releaseKeyboard();
            if (matching.size() == 1) {
                matching[0]->activated();
            } else if (matching.size() > 1) {
                // TODO ambiguous
            }
            for (auto &callbacks : pendingCallbacks) {
                ZPendingKeySequenceCallbacksPrivate::get(&callbacks)->finished(matching.size() == 1);
            }
        }
    });
}

void ZShortcutManager::registerPendingKeySequenceCallbacks(const ZPendingKeySequenceCallbacks &callbacks) {
    pendingCallbacks.append(callbacks);
}

TUIWIDGETS_NS_END
