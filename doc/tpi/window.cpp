#include "tpi-image-builder.h"

#include <QObject>
#include <QRect>

#include <Tui/ZRoot.h>
#include <Tui/ZSymbol.h>
#include <Tui/ZTest.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZWindow.h>

void window() {
    Tui::ZTerminal terminal (Tui::ZTerminal::OffScreen(40, 40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);
    root.setFocus();

    // default
    {
        Tui::ZWindow win(&root);
        win.setGeometry({1, 1, 18, 4});

        export_tpi(&terminal, "window", 0, 0, 20, 6);
    }

    // focus
    {
        Tui::ZWindow win(&root);
        win.setGeometry({1, 1, 18, 4});
        win.setFocus();

        export_tpi(&terminal, "window-focus", 0, 0, 20, 6);
    }

    // WindowTitle
    {
        Tui::ZWindow win(&root);
        win.setGeometry({1, 1, 18, 4});
        win.setWindowTitle("WindowTitle");

        export_tpi(&terminal, "window-title", 0, 0, 20, 6);
    }

    // WindowTitle + Focus
    {
        Tui::ZWindow win(&root);
        win.setGeometry({1, 1, 18, 4});
        win.setWindowTitle("WindowTitle");
        win.setFocus();

        export_tpi(&terminal, "window-title-focus", 0, 0, 20, 6);
    }

    // Window interactive move
    {
        Tui::ZWindow win(&root);
        win.setGeometry({1, 1, 18, 4});
        win.setWindowTitle("WindowTitle");
        win.setFocus();
        win.startInteractiveMove();

        export_tpi(&terminal, "window-move", 0, 0, 20, 6);
    }

    // options
    {
        Tui::ZWindow win(&root);
        win.setGeometry({1, 1, 18, 4});
        win.setWindowTitle("Title");
        win.setFocus();
        win.setOptions({Tui::ZWindow::MoveOption |
                        Tui::ZWindow::ResizeOption |
                        Tui::ZWindow::CloseOption |
                        Tui::ZWindow::AutomaticOption |
                        Tui::ZWindow::ContainerOptions });

        Tui::ZTest::sendText(&terminal, "-", Tui::AltModifier);

        export_tpi(&terminal, "window-options", 0, 0, 24, 10);
    }

    // ReducedCharset
    {
        Tui::ZTerminal terminal2 (Tui::ZTerminal::OffScreen(40, 40).withoutCapability(TUISYM_LITERAL("extendedCharset")));

        Tui::ZRoot root2;
        terminal2.setMainWidget(&root2);

        Tui::ZWindow win(&root2);
        win.setGeometry({1, 1, 18, 4});
        win.setWindowTitle("WindowTitle");
        win.setFocus();

        export_tpi(&terminal2, "window-reducedcharset", 0, 0, 20, 6);
    }

}
