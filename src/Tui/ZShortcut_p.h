#ifndef TUIWIDGETS_ZSHORTCUT_P_INCLUDED
#define TUIWIDGETS_ZSHORTCUT_P_INCLUDED

#include <QPointer>

#include <Tui/ZShortcut.h>

TUIWIDGETS_NS_START

class ZTerminal;

class ZShortcutPrivate {
public:
    Qt::ShortcutContext context;
    ZKeySequence key;

    QPointer<ZTerminal> terminal;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZSHORTCUT_P_INCLUDED
