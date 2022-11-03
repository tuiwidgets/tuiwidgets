#include "demo_gettingstarted.h"

#include <QCoreApplication>
#include <QRect>

#include <Tui/ZButton.h>
#include <Tui/ZRoot.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZWindow.h>

void demo_gettingstarted() {
    Tui::ZTerminal terminal (Tui::ZTerminal::OffScreen(80,24));
    Tui::ZRoot root;
    terminal.setMainWidget(&root);

    Tui::ZWindow *win = new Tui::ZWindow("Hello World", &root);
    win->setGeometry({5, 3, 20, 10});

    Tui::ZButton *button = new Tui::ZButton(Tui::withMarkup, "<m>Q</m>uit", win);
    button->setGeometry({6, 7, 10, 1});
    button->setFocus();

    export_tpi(&terminal, "demo_gettingstarted", 0,0,40,15);
}


