#include <QRect>

#include <Tui/ZRoot.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZWidget.h>
#include <Tui/ZWindow.h>

void exampleRoot() {
// snippet-start
    Tui::ZTerminal terminal;

    Tui::ZRoot root;

    terminal.setMainWidget(&root);
// snippet-end
}

