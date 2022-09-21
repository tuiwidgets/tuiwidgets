#include <Tui/ZHBoxLayout.h>
#include <Tui/ZTextLine.h>
#include <Tui/ZWidget.h>

void exampleTextLine(Tui::ZWidget *dialog, Tui::ZHBoxLayout *layout) {
// snippet-start
    Tui::ZTextLine *text = new Tui::ZTextLine("Heading", dialog);
    layout->addWidget(text);
// snippet-end
}

