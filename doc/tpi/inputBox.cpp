#include "inputBox.h"

void inputBox() {
    Tui::ZTerminal terminal (Tui::ZTerminal::OffScreen(40,40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);
    root.setGeometry({0,0,12,10});
    root.setFocus();

    Tui::ZDialog win(&root);
    win.setGeometry({0,0,12,10});
    win.setFocus();
    win.setBorderEdges({});

    // default
    {
        Tui::ZInputBox inputbox(&win);
        inputbox.setGeometry({1,1,10,1});
        inputbox.setText("InputBox");

        export_tpi(&terminal, "inputbox", 0,0,12,3);
    }

    // focus
    {
        Tui::ZInputBox inputbox(&win);
        inputbox.setGeometry({1,1,10,1});
        inputbox.setText("InputBox");
        inputbox.setFocus();

        export_tpi(&terminal, "inputbox-focus", 0,0,12,3);
    }

    // password
    {
        Tui::ZInputBox inputbox(&win);
        inputbox.setGeometry({1,1,10,1});
        inputbox.setText("InputBox");
        inputbox.setEchoMode(Tui::ZInputBox::Password);

        export_tpi(&terminal, "inputbox-password", 0,0,12,3);
    }
}
