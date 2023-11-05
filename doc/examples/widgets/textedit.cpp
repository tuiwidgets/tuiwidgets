#include <Tui/ZHBoxLayout.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZTextEdit.h>
#include <Tui/ZWidget.h>

void exampleTextEdit(Tui::ZWidget *dialog, Tui::ZHBoxLayout *layout) {
// snippet-start
    Tui::ZTextEdit *text = new Tui::ZTextEdit(dialog->terminal()->textMetrics(), dialog);
    text->setMinimumSize(10, 4);
    layout->addWidget(text);
// snippet-end
}

