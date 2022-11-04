#include "window.h"

void window() {
    Tui::ZTerminal terminal (Tui::ZTerminal::OffScreen(40,40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);
    root.setGeometry({0,0,10,10});
    root.setFocus();

    // default
    {
        Tui::ZWindow win(&root);
        win.setGeometry({1,1,18,4});

        export_tpi(&terminal, "window", 0,0,20,6);
    }

    // focus
    {
        Tui::ZWindow win(&root);
        win.setGeometry({1,1,18,4});
        win.setFocus();

        export_tpi(&terminal, "window-focus", 0,0,20,6);
    }

    // WindowTitle
    {
        Tui::ZWindow win(&root);
        win.setGeometry({1,1,18,4});
        win.setWindowTitle("WindowTitle");
        win.setFocus();

        export_tpi(&terminal, "window-title", 0,0,20,6);
    }

    // options
    {
        Tui::ZWindow win(&root);
        win.setGeometry({1,1,18,4});
        win.setWindowTitle("Title");
        win.setFocus();
        win.setOptions({Tui::ZWindow::MoveOption |
                           Tui::ZWindow::ResizeOption |
                           Tui::ZWindow::CloseOption |
                           Tui::ZWindow::AutomaticOption |
                           Tui::ZWindow::ContainerOptions });

        export_tpi(&terminal, "window-options", 0,0,20,6);
    }

}
