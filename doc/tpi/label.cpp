#include "tpi-image-builder.h"

#include <QObject>
#include <QRect>

#include <Tui/ZDialog.h>
#include <Tui/ZInputBox.h>
#include <Tui/ZLabel.h>
#include <Tui/ZPalette.h>
#include <Tui/ZRoot.h>
#include <Tui/ZTerminal.h>

void label() {
    Tui::ZTerminal terminal (Tui::ZTerminal::OffScreen(40, 40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);

    Tui::ZDialog dialog(&root);
    dialog.setGeometry({0, 0, 40, 10});
    dialog.setFocus();
    dialog.setBorderEdges({});

    // default
    {
        Tui::ZLabel label(&dialog);
        label.setGeometry({1, 1, 8, 1});
        label.setMarkup("L<m>a</m>bel");

        export_tpi(&terminal, "label", 0, 0, 10, 3);
    }

    // focused buddy
    {
        Tui::ZWidget buddy(&dialog);
        buddy.setFocus();

        Tui::ZLabel label(&dialog);
        label.setGeometry({1, 1, 8, 1});
        label.setMarkup("L<m>a</m>bel");
        label.setBuddy(&buddy);

        export_tpi(&terminal, "label-focus", 0, 0, 10, 3);
    }

    // color
    {
        Tui::ZPalette p;
        p.setColors({{"control.bg", Tui::Colors::red},
                     {"control.shortcut.bg", Tui::Colors::red},
                     {"control.fg", Tui::Colors::yellow}});

        Tui::ZLabel label(&dialog);
        label.setGeometry({1, 1, 8, 1});
        label.setMarkup("L<m>a</m>bel");
        label.setPalette(p);

        export_tpi(&terminal, "label-color", 0, 0, 10, 3);
    }

    // disabled
    {
        Tui::ZLabel label(&dialog);
        label.setGeometry({1, 1, 8, 1});
        label.setMarkup("L<m>a</m>bel");
        label.setEnabled(false);

        export_tpi(&terminal, "label-disabled", 0, 0, 10, 3);
    }

    // buddy
    {
        Tui::ZLabel label(&dialog);
        Tui::ZInputBox textinput(&dialog);

        label.setGeometry({1, 1, 9, 1});
        label.setMarkup("<m>F</m>ilename");

        textinput.setGeometry({11, 1, 6, 1});

        export_tpi(&terminal, "label-buddy", 0, 0, 18, 3);
    }
}
