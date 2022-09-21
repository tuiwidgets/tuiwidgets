#include <Tui/ZHBoxLayout.h>
#include <Tui/ZInputBox.h>
#include <Tui/ZLabel.h>
#include <Tui/ZWidget.h>

void exampleLabel(Tui::ZWidget *dialog, Tui::ZHBoxLayout *layout) {
// snippet-start
    Tui::ZLabel *label = new Tui::ZLabel(Tui::withMarkup, "<m>F</m>ilename", dialog);
    Tui::ZInputBox *textinput = new Tui::ZInputBox(dialog);
    label->setBuddy(textinput);
    layout->addWidget(label);
    layout->addWidget(textinput);
// snippet-end
}

