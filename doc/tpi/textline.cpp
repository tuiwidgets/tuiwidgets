#include "tpi-image-builder.h"

#include <QObject>
#include <QRect>

#include <Tui/ZDialog.h>
#include <Tui/ZPalette.h>
#include <Tui/ZRoot.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZTextLine.h>

void textline() {
    Tui::ZTerminal terminal (Tui::ZTerminal::OffScreen(40, 40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);

    Tui::ZDialog dialog(&root);
    dialog.setGeometry({0, 0, 10, 10});
    dialog.setFocus();
    dialog.setBorderEdges({});

    // default
    {
        Tui::ZTextLine textline(&dialog);
        textline.setGeometry({1, 1, 8, 1});
        textline.setMarkup("T<m>e</m>xtLine");

        export_tpi(&terminal, "textline", 0, 0, 10, 3);
    }

    // focus
    {
        Tui::ZTextLine textline(&dialog);
        textline.setGeometry({1, 1, 8, 1});
        textline.setMarkup("T<m>e</m>xtLine");
        textline.setFocus();

        export_tpi(&terminal, "textline-focus", 0, 0, 10, 3);
    }

    // disabled
    {
        Tui::ZTextLine textline(&dialog);
        textline.setGeometry({1, 1, 8, 1});
        textline.setMarkup("T<m>e</m>xtLine");
        textline.setEnabled(false);

        export_tpi(&terminal, "textline-disabled", 0, 0, 10, 3);
    }

    // color
    {
        Tui::ZPalette p;
        p.setColors({{"control.bg", Tui::Colors::red},
                     {"control.shortcut.bg", Tui::Colors::red},
                     {"control.fg", Tui::Colors::yellow}});

        Tui::ZTextLine textline(&dialog);
        textline.setGeometry({1, 1, 8, 1});
        textline.setMarkup("T<m>e</m>xtLine");
        textline.setPalette(p);

        export_tpi(&terminal, "textline-color", 0, 0, 10, 3);
    }
}
