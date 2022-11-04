#include "root.h"

void root() {
    Tui::ZTerminal terminal (Tui::ZTerminal::OffScreen(40,40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);

    //Default
    {
        root.setGeometry({1,1,18,4});

        export_tpi(&terminal, "root", 0,0,20,6);
    }

    // Background
    {
        root.setGeometry({1,1,18,4});
        root.setFillChar(u'▒');
        export_tpi(&terminal, "root-fillchar", 0,0,20,6);
    }
}
