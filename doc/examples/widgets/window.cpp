#include <QRect>

#include <Tui/ZRoot.h>
#include <Tui/ZWidget.h>
#include <Tui/ZWindow.h>

void exampleWindow(Tui::ZRoot *root) {
// snippet-start
    Tui::ZWindow win(root);
    win.setGeometry({1, 1, 18, 4});
    win.setOptions({Tui::ZWindow::MoveOption |
                    Tui::ZWindow::ResizeOption |
                    Tui::ZWindow::CloseOption |
                    Tui::ZWindow::AutomaticOption |
                    Tui::ZWindow::ContainerOptions });
// snippet-end
}

