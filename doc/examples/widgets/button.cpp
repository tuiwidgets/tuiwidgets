#include <Tui/ZButton.h>
#include <Tui/ZHBoxLayout.h>
#include <Tui/ZWidget.h>

void exampleButton(Tui::ZWidget *dialog, Tui::ZHBoxLayout *layout) {
// snippet-start
    Tui::ZButton *button = new Tui::ZButton(Tui::withMarkup, "B<m>u</m>tton", dialog);
    layout->addWidget(button);

    QObject::connect(button, &Tui::ZButton::clicked, [] { /* ... */ });
// snippet-end
}

int main() {
}
