#include "dialog.h"

void dialog() {
    Tui::ZTerminal terminal (Tui::ZTerminal::OffScreen(40,40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);
    root.setGeometry({0,0,20,6});
    root.setFocus();

    Tui::ZWindow win(&root);
    win.setGeometry({0,0,20,6});
    win.setFocus();
    win.setBorderEdges({});

    // bug
    {
        Tui::ZDialog dialog(&win);
        dialog.setGeometry({0,0,18,4});

        export_tpi(&terminal, "dialog-bug", 0,0,20,6);
    }

    // default
    {
        Tui::ZDialog dialog(&win);
        dialog.setGeometry({1,1,18,4});

        export_tpi(&terminal, "dialog", 0,0,20,6);
    }

    // focus
    {
        Tui::ZDialog dialog(&win);
        dialog.setGeometry({1,1,18,4});
        dialog.setFocus();

        export_tpi(&terminal, "dialog-focus", 0,0,20,6);
    }

    // WindowTitle
    {
        Tui::ZDialog dialog(&win);
        dialog.setGeometry({1,1,18,4});
        dialog.setFocus();
        dialog.setWindowTitle("WindowTitle");

        export_tpi(&terminal, "dialog-windowtitle", 0,0,20,6);
    }

    // options
    {
        Tui::ZDialog dialog(&win);
        dialog.setGeometry({1,1,18,4});
        dialog.setFocus();
        dialog.setWindowTitle("Title");
        dialog.setOptions({Tui::ZWindow::MoveOption |
                           Tui::ZWindow::ResizeOption |
                           Tui::ZWindow::CloseOption |
                           Tui::ZWindow::AutomaticOption |
                           Tui::ZWindow::ContainerOptions });

        //TODO: how to open

        export_tpi(&terminal, "dialog-options", 0,0,20,6);
    }
}
