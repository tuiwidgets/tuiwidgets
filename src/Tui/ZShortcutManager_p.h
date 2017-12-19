#ifndef TUIWIDGETS_ZSHORTCUTMANAGER_INCLUDED
#define TUIWIDGETS_ZSHORTCUTMANAGER_INCLUDED

#include <QSet>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZKeyEvent;
class ZShortcut;
class ZTerminal;

class ZShortcutManager {
public:
    explicit ZShortcutManager(ZTerminal *terminal);

    void addShortcut(ZShortcut *s);
    void removeShortcut(ZShortcut *s);

    bool process(const ZKeyEvent *event);

private:
    ZTerminal *terminal;
    QSet<ZShortcut *> shortcuts;
};
TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZSHORTCUTMANAGER_INCLUDED
