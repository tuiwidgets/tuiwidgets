#include "button.h"

void button() {
    Tui::ZTerminal terminal (Tui::ZTerminal::OffScreen(40,40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);
    root.setGeometry({0,0,10,10});
    root.setFocus();

    Tui::ZDialog win(&root);
    win.setGeometry({0,0,10,10});
    win.setFocus();
    win.setBorderEdges({});

    //button
    {
        Tui::ZButton button(&win);
        button.setGeometry({0,1,10,1});
        button.setMarkup("B<m>u</m>tton");

        export_tpi(&terminal, "button", 0,0,10,3);
    }

    // default
    {
        Tui::ZButton button(&win);
        button.setGeometry({0,1,10,1});
        button.setMarkup("B<m>u</m>tton");
        button.setDefault(true);

        export_tpi(&terminal, "button-default", 0,0,10,3);
    }

    //focus
    {
        Tui::ZButton button(&win);
        button.setGeometry({0,1,10,1});
        button.setMarkup("B<m>u</m>tton");
        button.setFocus();

        export_tpi(&terminal, "button-focus", 0,0,10,3);
    }

    //disabled
    {
        Tui::ZButton button(&win);
        button.setGeometry({0,1,10,1});
        button.setMarkup("B<m>u</m>tton");
        button.setEnabled(false);

        export_tpi(&terminal, "button-disable", 0,0,10,3);
    }
}
