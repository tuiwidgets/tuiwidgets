#include <Tui/ZHBoxLayout.h>
#include <Tui/ZInputBox.h>
#include <Tui/ZWidget.h>

void exampleInputBox(Tui::ZWidget *dialog, Tui::ZHBoxLayout *layout) {
// snippet-start
    Tui::ZInputBox *inputbox = new Tui::ZInputBox("InputBox", dialog);
    layout->addWidget(inputbox);

    QObject::connect(inputbox, &Tui::ZInputBox::textChanged, [](const QString &text) { /* ... */ });
// snippet-end
}

