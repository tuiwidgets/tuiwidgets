#include "label.h"

void label() {
    Tui::ZTerminal terminal (Tui::ZTerminal::OffScreen(40,40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);
    root.setGeometry({0,0,10,10});
    root.setFocus();

    Tui::ZWindow win(&root);
    win.setGeometry({0,0,10,10});
    win.setFocus();
    win.setBorderEdges({});

    // default
    {
        Tui::ZLabel l(&win);
        l.setGeometry({0,1,9,1});
        l.setMarkup("L<m>a</m>bel");

        export_tpi(&terminal, "label", 0,0,10,3);
    }

    // focus
    {
        Tui::ZLabel l(&win);
        l.setGeometry({0,1,9,1});
        l.setMarkup("L<m>a</m>bel");
        l.setFocus();

        export_tpi(&terminal, "label-focus", 0,0,10,3);
    }

    // color
    {
        Tui::ZPalette p;
        p.setColors({{"control.bg", Tui::Colors::red},
                     {"control.shortcut.bg", Tui::Colors::red},
                     {"control.fg", Tui::Colors::yellow}});

        Tui::ZLabel l(&win);
        l.setGeometry({0,1,9,1});
        l.setMarkup("L<m>a</m>bel");
        l.setPalette(p);

        export_tpi(&terminal, "label-color", 0,0,10,3);
    }
}
