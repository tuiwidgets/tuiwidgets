#include "button.h"

#include <QObject>
#include <QRect>

#include <Tui/ZButton.h>
#include <Tui/ZDialog.h>
#include <Tui/ZImage.h>
#include <Tui/ZRoot.h>
#include <Tui/ZTerminal.h>

#include "export_tpi.h"

void button() {
    Tui::ZTerminal terminal (Tui::ZTerminal::OffScreen(40,40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);
    root.setGeometry({0,0,10,10});
    root.setFocus();

    Tui::ZDialog dialog(&root);
    dialog.setGeometry({0,0,10,10});
    dialog.setFocus();
    dialog.setBorderEdges({});

    //button
    {
        Tui::ZButton button(&dialog);
        button.setGeometry({0,1,10,1});
        button.setMarkup("B<m>u</m>tton");

        export_tpi(&terminal, "button", 0,0,10,3);
    }

    // default
    {
        Tui::ZButton button(&dialog);
        button.setGeometry({0,1,10,1});
        button.setMarkup("B<m>u</m>tton");
        button.setDefault(true);

        export_tpi(&terminal, "button-default", 0,0,10,3);
    }

    //focus
    {
        Tui::ZButton button(&dialog);
        button.setGeometry({0,1,10,1});
        button.setMarkup("B<m>u</m>tton");
        button.setFocus();

        export_tpi(&terminal, "button-focus", 0,0,10,3);
    }

    //disabled
    {
        Tui::ZButton button(&dialog);
        button.setGeometry({0,1,10,1});
        button.setMarkup("B<m>u</m>tton");
        button.setEnabled(false);

        export_tpi(&terminal, "button-disable", 0,0,10,3);
    }
}
