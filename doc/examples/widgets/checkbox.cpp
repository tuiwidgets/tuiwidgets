#include <Tui/ZCheckBox.h>
#include <Tui/ZHBoxLayout.h>
#include <Tui/ZWidget.h>

void exampleCheckBox(Tui::ZWidget *dialog, Tui::ZHBoxLayout *layout) {
// snippet-start
    Tui::ZCheckBox *checkbox = new Tui::ZCheckBox(Tui::withMarkup, "C<m>h</m>eckbox", dialog);
    layout->addWidget(checkbox);

    QObject::connect(checkbox, &Tui::ZCheckBox::stateChanged, [](Qt::CheckState state) { /* ... */ });
// snippet-end
}

