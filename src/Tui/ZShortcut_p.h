#ifndef TUIWIDGETS_ZSHORTCUT_P_INCLUDED
#define TUIWIDGETS_ZSHORTCUT_P_INCLUDED

#include <QPointer>

#include <Tui/ZShortcut.h>

TUIWIDGETS_NS_START

class ZTerminal;

class ZShortcutPrivate {
public:
    bool isContextActive(QObject *par, ZWidget *focusWidget);

public:
    Qt::ShortcutContext context;
    ZKeySequence key;
    bool enabled = true;

    Private::ZMoFunc<bool()> enabledDelegate;

    QPointer<ZTerminal> terminal;
};

class ZKeySequencePrivate {
public:
    QString forMnemonic;
    QString forShortcut;
    Qt::KeyboardModifiers modifiers = {};
    int forKey = 0;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZSHORTCUT_P_INCLUDED
