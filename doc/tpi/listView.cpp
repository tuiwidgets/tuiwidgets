#include "listView.h"

#include <QObject>
#include <QRect>
#include <QVariant>

#include <Tui/ZDialog.h>
#include <Tui/ZImage.h>
#include <Tui/ZListView.h>
#include <Tui/ZRoot.h>
#include <Tui/ZTerminal.h>

#include "export_tpi.h"

void listView() {
    Tui::ZTerminal terminal (Tui::ZTerminal::OffScreen(40,40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);
    root.setGeometry({0,0,12,10});
    root.setFocus();

    Tui::ZDialog dialog(&root);
    dialog.setGeometry({0,0,12,10});
    dialog.setFocus();
    dialog.setBorderEdges({});

    // default
    {
        Tui::ZListView listview(&dialog);
        listview.setGeometry({1,1,10,5});
        listview.setItems({"item1","item2","item3"});

        export_tpi(&terminal, "listview", 0,0,12,7);
    }

    // focus
    {
        Tui::ZListView listview(&dialog);
        listview.setGeometry({1,1,10,5});
        listview.setItems({"item1","item2","item3"});
        listview.setFocus();

        export_tpi(&terminal, "listview-focus", 0,0,12,7);
    }

    // disabel
    {
        Tui::ZListView listview(&dialog);
        listview.setGeometry({1,1,10,5});
        listview.setItems({"item1","item2","item3"});
        listview.setEnabled(false);

        export_tpi(&terminal, "listview-disabel", 0,0,12,7);
    }
}
