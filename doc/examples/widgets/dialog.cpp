#include <QRect>

#include <Tui/ZRoot.h>
#include <Tui/ZWidget.h>
#include <Tui/ZDialog.h>

void exampleDialog(Tui::ZRoot *root) {
// snippet-start
    Tui::ZDialog dialog(root);
    dialog.setGeometry({1, 1, 18, 4});
    dialog.setOptions({Tui::ZWindow::MoveOption |
                       Tui::ZWindow::ResizeOption |
                       Tui::ZWindow::CloseOption |
                       Tui::ZWindow::AutomaticOption |
                       Tui::ZWindow::ContainerOptions });
// snippet-end
}

