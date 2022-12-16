// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZSHORTCUT_P_INCLUDED
#define TUIWIDGETS_ZSHORTCUT_P_INCLUDED

#include <QPointer>

#include <Tui/ZShortcut.h>

TUIWIDGETS_NS_START

class ZTerminal;

class ZShortcutPrivate {
public:
    ZShortcutPrivate();
    virtual ~ZShortcutPrivate();

public:
    static ZShortcutPrivate *get(ZShortcut *shortcut);
    static const ZShortcutPrivate *get(const ZShortcut *shortcut);

    bool isContextActive(QObject *par, ZWidget *focusWidget) const;

public:
    ShortcutContext context;
    ZKeySequence key;
    bool enabled = true;

    Private::ZMoFunc<bool()> enabledDelegate;

    QPointer<ZTerminal> terminal;
};

class ZKeySequencePrivate {
public:
    ZKeySequencePrivate();
    virtual ~ZKeySequencePrivate();

public:
    static ZKeySequencePrivate *get(ZKeySequence *keyseq);
    static const ZKeySequencePrivate *get(const ZKeySequence *keyseq);

public:
    QString forMnemonic;
    QString forShortcut;
    KeyboardModifiers modifiers = {};
    int forKey = 0;

    // for two part sequences
    QString forShortcut2;
    KeyboardModifiers modifiers2 = {};
    int forKey2 = 0;
};

class ZPendingKeySequenceCallbacksPrivate {
public:
    ZPendingKeySequenceCallbacksPrivate();
    virtual ~ZPendingKeySequenceCallbacksPrivate();

public:
    static ZPendingKeySequenceCallbacksPrivate *get(ZPendingKeySequenceCallbacks *callbacks);
    static const ZPendingKeySequenceCallbacksPrivate *get(const ZPendingKeySequenceCallbacks *callbacks);

public:
    std::function<void()> started;
    std::function<void(bool matched)> finished;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZSHORTCUT_P_INCLUDED
