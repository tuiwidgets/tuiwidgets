// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZSHORTCUTMANAGER_INCLUDED
#define TUIWIDGETS_ZSHORTCUTMANAGER_INCLUDED

#include <QSet>
#include <QMap>
#include <QVector>

#include <Tui/ZShortcut.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZKeyEvent;
class ZPendingKeySequenceCallbacks;
class ZShortcut;
class ZTerminal;

class ZShortcutManager {
public:
    class Key {
    public:
        QString c;
        Qt::KeyboardModifiers modifiers = {};

        bool operator==(const Key &rhs) const { return c == rhs.c && modifiers == rhs.modifiers; }
        bool operator<(const Key &rhs) const { return c < rhs.c ? true : modifiers < rhs.modifiers; };
    };

public:
    explicit ZShortcutManager(ZTerminal *terminal);
    virtual ~ZShortcutManager();

    void addShortcut(ZShortcut *s);
    void removeShortcut(ZShortcut *s);

    bool process(const ZKeyEvent *event);

    void activateTwoPart(const Key &prefix);

    void registerPendingKeySequenceCallbacks(const ZPendingKeySequenceCallbacks &callbacks);

private:
    ZTerminal *terminal;
    QSet<ZShortcut*> shortcuts;
    QMap<Key, QSet<ZShortcut*>> twoPartShortcuts;
    QVector<ZPendingKeySequenceCallbacks> pendingCallbacks;
};
TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZSHORTCUTMANAGER_INCLUDED
