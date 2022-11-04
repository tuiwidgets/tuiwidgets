#include "label.h"

#include <QObject>
#include <QRect>

#include <Tui/ZDialog.h>
#include <Tui/ZImage.h>
#include <Tui/ZLabel.h>
#include <Tui/ZPalette.h>
#include <Tui/ZRoot.h>
#include <Tui/ZTerminal.h>

#include "export_tpi.h"

void label() {
    Tui::ZTerminal terminal (Tui::ZTerminal::OffScreen(40,40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);
    root.setGeometry({0,0,10,10});
    root.setFocus();

    Tui::ZDialog dialog(&root);
    dialog.setGeometry({0,0,10,10});
    dialog.setFocus();
    dialog.setBorderEdges({});

    // default
    {
        Tui::ZLabel label(&dialog);
        label.setGeometry({0,1,9,1});
        label.setMarkup("L<m>a</m>bel");

        export_tpi(&terminal, "label", 0,0,10,3);
    }

    // focus
    {
        Tui::ZLabel label(&dialog);
        label.setGeometry({0,1,9,1});
        label.setMarkup("L<m>a</m>bel");
        label.setFocus();

        export_tpi(&terminal, "label-focus", 0,0,10,3);
    }

    // color
    {
        Tui::ZPalette p;
        p.setColors({{"control.bg", Tui::Colors::red},
                     {"control.shortcut.bg", Tui::Colors::red},
                     {"control.fg", Tui::Colors::yellow}});

        Tui::ZLabel label(&dialog);
        label.setGeometry({0,1,9,1});
        label.setMarkup("L<m>a</m>bel");
        label.setPalette(p);

        export_tpi(&terminal, "label-color", 0,0,10,3);
    }

    // disabled
    {
        Tui::ZLabel label(&dialog);
        label.setGeometry({0,1,9,1});
        label.setMarkup("L<m>a</m>bel");
        label.setEnabled(false);

        export_tpi(&terminal, "label-disabled", 0,0,10,3);
    }
}
