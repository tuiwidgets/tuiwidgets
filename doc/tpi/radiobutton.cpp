#include "radiobutton.h"

void radiobutton() {
    Tui::ZTerminal terminal (Tui::ZTerminal::OffScreen(40,40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);
    root.setGeometry({0,0,17,10});
    root.setFocus();

    Tui::ZWindow win(&root);
    win.setGeometry({0,0,17,10});
    win.setFocus();
    win.setBorderEdges({});

    // default
    {
        Tui::ZRadioButton radiobutton(&win);
        radiobutton.setGeometry({0,1,17,1});
        radiobutton.setMarkup("R<m>a</m>dioButton");

        export_tpi(&terminal, "radiobutton", 0,0,17,3);
    }

    // focuse
    {
        Tui::ZRadioButton radiobutton(&win);
        radiobutton.setGeometry({0,1,17,1});
        radiobutton.setMarkup("R<m>a</m>dioButton");
        radiobutton.setFocus();

        export_tpi(&terminal, "radiobutton-focus", 0,0,17,3);
    }

    // checked
    {
        Tui::ZRadioButton radiobutton(&win);
        radiobutton.setGeometry({0,1,17,1});
        radiobutton.setMarkup("R<m>a</m>dioButton");
        radiobutton.setChecked(true);

        export_tpi(&terminal, "radiobutton-checked", 0,0,17,3);
    }

    // diesabeld
    {
        Tui::ZRadioButton radiobutton(&win);
        radiobutton.setGeometry({0,1,17,1});
        radiobutton.setMarkup("R<m>a</m>dioButton");
        radiobutton.setChecked(true);
        radiobutton.setEnabled(false);

        export_tpi(&terminal, "radiobutton-diesabeld", 0,0,17,3);
    }
}
