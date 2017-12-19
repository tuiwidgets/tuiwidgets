#include "ZShortcutManager_p.h"

#include <QPointer>
#include <QVector>

#include <Tui/ZShortcut.h>
#include <Tui/ZTerminal.h>

TUIWIDGETS_NS_START

ZShortcutManager::ZShortcutManager(ZTerminal *terminal)
    : terminal(terminal)
{
}

void ZShortcutManager::addShortcut(ZShortcut *s) {
    shortcuts.insert(s);
}

void ZShortcutManager::removeShortcut(ZShortcut *s) {
    shortcuts.remove(s);
}

bool ZShortcutManager::process(const ZKeyEvent *event) {
    ZWidget *focusWidget = terminal->focusWidget();
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

TUIWIDGETS_NS_END
