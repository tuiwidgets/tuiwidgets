#include "listView.h"

void listView() {
    Tui::ZTerminal terminal (Tui::ZTerminal::OffScreen(40,40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);
    root.setGeometry({0,0,12,10});
    root.setFocus();

    Tui::ZWindow win(&root);
    win.setGeometry({0,0,12,10});
    win.setFocus();
    win.setBorderEdges({});

    // default
    {
        Tui::ZListView listview(&win);
        listview.setGeometry({1,1,10,5});
        listview.setItems({"item1","item2","item3"});

        export_tpi(&terminal, "listview", 0,0,12,7);
    }

    // focus
    {
        Tui::ZListView listview(&win);
        listview.setGeometry({1,1,10,5});
        listview.setItems({"item1","item2","item3"});
        listview.setFocus();

        export_tpi(&terminal, "listview-focus", 0,0,12,7);
    }

    // disabel
    {
        Tui::ZListView listview(&win);
        listview.setGeometry({1,1,10,5});
        listview.setItems({"item1","item2","item3"});
        listview.setEnabled(false);

        export_tpi(&terminal, "listview-disabel", 0,0,12,7);
    }
}
