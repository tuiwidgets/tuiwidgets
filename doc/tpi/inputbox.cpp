#include "tpi-image-builder.h"

#include <QObject>
#include <QRect>

#include <Tui/ZDialog.h>
#include <Tui/ZInputBox.h>
#include <Tui/ZRoot.h>
#include <Tui/ZTerminal.h>

void inputBox() {
    Tui::ZTerminal terminal (Tui::ZTerminal::OffScreen(40, 40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);

    Tui::ZDialog dialog(&root);
    dialog.setGeometry({0, 0, 12, 10});
    dialog.setFocus();
    dialog.setBorderEdges({});

    // default
    {
        Tui::ZInputBox inputbox(&dialog);
        inputbox.setGeometry({1, 1, 10, 1});
        inputbox.setText("InputBox");

        export_tpi(&terminal, "inputbox", 0, 0, 12, 3);
    }

    // focus
    {
        Tui::ZInputBox inputbox(&dialog);
        inputbox.setGeometry({1, 1, 10, 1});
        inputbox.setText("InputBox");
        inputbox.setFocus();

        export_tpi(&terminal, "inputbox-focus", 0, 0, 12, 3);
    }

    // disabled
    {
        Tui::ZInputBox inputbox(&dialog);
        inputbox.setGeometry({1, 1, 10, 1});
        inputbox.setText("InputBox");
        inputbox.setEnabled(false);

        export_tpi(&terminal, "inputbox-disabled", 0, 0, 12, 3);
    }

    // password
    {
        Tui::ZInputBox inputbox(&dialog);
        inputbox.setGeometry({1, 1, 10, 1});
        inputbox.setText("InputBox");
        inputbox.setEchoMode(Tui::ZInputBox::Password);

        export_tpi(&terminal, "inputbox-password", 0, 0, 12, 3);
    }

    // embedded newline
    {
        Tui::ZInputBox inputbox(&dialog);
        inputbox.setGeometry({1, 1, 10, 1});
        inputbox.setText("one\ntwo");

        export_tpi(&terminal, "inputbox-newline", 0, 0, 12, 3);
    }
}
