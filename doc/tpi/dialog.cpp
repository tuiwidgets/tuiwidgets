#include "tpi-image-builder.h"

#include <QObject>
#include <QRect>

#include <Tui/ZDialog.h>
#include <Tui/ZRoot.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZTest.h>

void dialog() {
    Tui::ZTerminal terminal (Tui::ZTerminal::OffScreen(40, 40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);

    // default
    {
        Tui::ZDialog dialog(&root);
        dialog.setGeometry({1, 1, 18, 4});

        export_tpi(&terminal, "dialog", 0, 0, 20, 6);
    }

    // focus
    {
        Tui::ZDialog dialog(&root);
        dialog.setGeometry({1, 1, 18, 4});
        dialog.setFocus();

        export_tpi(&terminal, "dialog-focus", 0, 0, 20, 6);
    }

    // WindowTitle
    {
        Tui::ZDialog dialog(&root);
        dialog.setGeometry({1, 1, 18, 4});
        dialog.setFocus();
        dialog.setWindowTitle("WindowTitle");

        export_tpi(&terminal, "dialog-windowtitle", 0, 0, 20, 6);
    }

    // options
    {
        Tui::ZDialog dialog(&root);
        dialog.setGeometry({1, 1, 18, 4});
        dialog.setFocus();
        dialog.setWindowTitle("Title");
        dialog.setOptions({Tui::ZWindow::MoveOption |
                           Tui::ZWindow::ResizeOption |
                           Tui::ZWindow::CloseOption |
                           Tui::ZWindow::AutomaticOption |
                           Tui::ZWindow::ContainerOptions });

        Tui::ZTest::sendText(&terminal, "-", Tui::AltModifier);

        export_tpi(&terminal, "dialog-options", 0, 0, 24, 10);
    }

}
