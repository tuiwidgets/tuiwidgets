#include "checkBox.h"

void checkBox() {
    Tui::ZTerminal terminal (Tui::ZTerminal::OffScreen(40,40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);
    root.setGeometry({0,0,20,10});
    root.setFocus();

    Tui::ZWindow win(&root);
    win.setGeometry({0,0,20,10});
    win.setFocus();
    win.setBorderEdges({});

    // default
    {
        Tui::ZCheckBox cb(&win);
        cb.setGeometry({0,1,14,1});
        cb.setMarkup("C<m>h</m>eckBox");

        export_tpi(&terminal, "checkbox", 0,0,14,3);
    }

    // focus
    {
        Tui::ZCheckBox cb(&win);
        cb.setGeometry({0,1,14,1});
        cb.setMarkup("C<m>h</m>eckBox");
        cb.setFocus();

        export_tpi(&terminal, "checkbox-focus", 0,0,14,3);
    }

    // checked
    {
        Tui::ZCheckBox cb(&win);
        cb.setGeometry({0,1,14,1});
        cb.setMarkup("C<m>h</m>eckBox");
        cb.setCheckState(Tui::CheckState::Checked);

        export_tpi(&terminal, "checkbox-checked", 0,0,14,3);
    }

    // partially checked
    {
        Tui::ZCheckBox cb(&win);
        cb.setGeometry({0,1,14,1});
        cb.setMarkup("C<m>h</m>eckBox");
        cb.setCheckState(Tui::CheckState::PartiallyChecked);

        export_tpi(&terminal, "checkbox-partially-checked", 0,0,14,3);
    }

    // disabled
    {
        Tui::ZCheckBox cb(&win);
        cb.setGeometry({0,1,14,1});
        cb.setMarkup("C<m>h</m>eckBox");
        cb.setEnabled(false);

        export_tpi(&terminal, "checkbox-disabeld", 0,0,14,3);
    }
}
