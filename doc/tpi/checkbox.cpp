#include "tpi-image-builder.h"

#include <QObject>
#include <QRect>

#include <Tui/ZCheckBox.h>
#include <Tui/ZDialog.h>
#include <Tui/ZRoot.h>
#include <Tui/ZTerminal.h>

void checkBox() {
    Tui::ZTerminal terminal(Tui::ZTerminal::OffScreen(40, 40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);

    Tui::ZDialog dialog(&root);
    dialog.setGeometry({0, 0, 20, 10});
    dialog.setFocus();
    dialog.setBorderEdges({});

    // default
    {
        Tui::ZCheckBox checkbox(&dialog);
        checkbox.setGeometry({0, 1, 14, 1});
        checkbox.setMarkup("C<m>h</m>eckBox");

        export_tpi(&terminal, "checkbox", 0, 0, 14, 3);
    }

    // focus
    {
        Tui::ZCheckBox checkbox(&dialog);
        checkbox.setGeometry({0, 1, 14, 1});
        checkbox.setMarkup("C<m>h</m>eckBox");
        checkbox.setFocus();

        export_tpi(&terminal, "checkbox-focus", 0, 0, 14, 3);
    }

    // checked
    {
        Tui::ZCheckBox checkbox(&dialog);
        checkbox.setGeometry({0, 1, 14, 1});
        checkbox.setMarkup("C<m>h</m>eckBox");
        checkbox.setCheckState(Tui::CheckState::Checked);

        export_tpi(&terminal, "checkbox-checked", 0, 0, 14, 3);
    }

    // partially checked
    {
        Tui::ZCheckBox checkbox(&dialog);
        checkbox.setGeometry({0, 1, 14, 1});
        checkbox.setMarkup("C<m>h</m>eckBox");
        checkbox.setCheckState(Tui::CheckState::PartiallyChecked);

        export_tpi(&terminal, "checkbox-partially-checked", 0, 0, 14, 3);
    }

    // disabled
    {
        Tui::ZCheckBox checkbox(&dialog);
        checkbox.setGeometry({0, 1, 14, 1});
        checkbox.setMarkup("C<m>h</m>eckBox");
        checkbox.setEnabled(false);

        export_tpi(&terminal, "checkbox-disabled", 0, 0, 14, 3);
    }
}
