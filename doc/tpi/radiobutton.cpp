#include "tpi-image-builder.h"

#include <QObject>
#include <QRect>

#include <Tui/ZDialog.h>
#include <Tui/ZRadioButton.h>
#include <Tui/ZRoot.h>
#include <Tui/ZTerminal.h>

void radiobutton() {
    Tui::ZTerminal terminal (Tui::ZTerminal::OffScreen(40, 40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);

    Tui::ZDialog dialog(&root);
    dialog.setGeometry({0, 0, 17, 10});
    dialog.setFocus();
    dialog.setBorderEdges({});

    // default
    {
        Tui::ZRadioButton radiobutton1(&dialog);
        radiobutton1.setGeometry({0, 1, 17, 1});
        radiobutton1.setMarkup("<m>R</m>ed");
        radiobutton1.setChecked(true);

        Tui::ZRadioButton radiobutton2(&dialog);
        radiobutton2.setGeometry({0, 3, 17, 1});
        radiobutton2.setMarkup("<m>G</m>reen");

        Tui::ZRadioButton radiobutton3(&dialog);
        radiobutton3.setGeometry({0, 5, 17, 1});
        radiobutton3.setMarkup("<m>B</m>lue");

        export_tpi(&terminal, "radiobutton", 0, 0, 17, 7);
    }

    // focus
    {
        Tui::ZRadioButton radiobutton1(&dialog);
        radiobutton1.setGeometry({0, 1, 17, 1});
        radiobutton1.setMarkup("<m>R</m>ed");
        radiobutton1.setChecked(true);

        Tui::ZRadioButton radiobutton2(&dialog);
        radiobutton2.setGeometry({0, 3, 17, 1});
        radiobutton2.setMarkup("<m>G</m>reen");
        radiobutton2.setFocus();

        Tui::ZRadioButton radiobutton3(&dialog);
        radiobutton3.setGeometry({0, 5, 17, 1});
        radiobutton3.setMarkup("<m>B</m>lue");

        export_tpi(&terminal, "radiobutton-focus", 0, 0, 17, 7);
    }

    // checked
    {
        Tui::ZRadioButton radiobutton(&dialog);
        radiobutton.setGeometry({0, 1, 17, 1});
        radiobutton.setMarkup("R<m>a</m>dioButton");
        radiobutton.setChecked(true);

        export_tpi(&terminal, "radiobutton-checked", 0, 0, 17, 3);
    }

    // disabled
    {
        Tui::ZRadioButton radiobutton1(&dialog);
        radiobutton1.setGeometry({0, 1, 17, 1});
        radiobutton1.setMarkup("<m>R</m>ed");
        radiobutton1.setChecked(true);
        radiobutton1.setEnabled(false);

        Tui::ZRadioButton radiobutton2(&dialog);
        radiobutton2.setGeometry({0, 3, 17, 1});
        radiobutton2.setMarkup("<m>G</m>reen");
        radiobutton2.setEnabled(false);

        Tui::ZRadioButton radiobutton3(&dialog);
        radiobutton3.setGeometry({0, 5, 17, 1});
        radiobutton3.setMarkup("<m>B</m>lue");
        radiobutton3.setEnabled(false);

        export_tpi(&terminal, "radiobutton-disabled", 0, 0, 17, 7);
    }
}
