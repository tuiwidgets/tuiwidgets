#include <Tui/ZHBoxLayout.h>
#include <Tui/ZRadioButton.h>
#include <Tui/ZWidget.h>

void exampleRadioButton(Tui::ZWidget *dialog, Tui::ZHBoxLayout *layout) {
// snippet-start
    Tui::ZRadioButton *radiobutton1
             = new Tui::ZRadioButton(Tui::withMarkup, "<m>R</m>ed", dialog);
    layout->addWidget(radiobutton1);

    Tui::ZRadioButton *radiobutton2
             = new Tui::ZRadioButton(Tui::withMarkup, "<m>G</m>reen", dialog);
    layout->addWidget(radiobutton2);

    Tui::ZRadioButton *radiobutton3
             = new Tui::ZRadioButton(Tui::withMarkup, "<m>B</m>lue", dialog);
    layout->addWidget(radiobutton3);

    QObject::connect(radiobutton1, &Tui::ZRadioButton::toggled, [](bool state) { /* ... */ });
    QObject::connect(radiobutton2, &Tui::ZRadioButton::toggled, [](bool state) { /* ... */ });
    QObject::connect(radiobutton3, &Tui::ZRadioButton::toggled, [](bool state) { /* ... */ });
// snippet-end
}

void exampleRadioButtonGroups(Tui::ZWidget *parent1, Tui::ZHBoxLayout *layout1, Tui::ZWidget *parent2, Tui::ZHBoxLayout *layout2) {
// snippet-2group-start
    // First group using parent1 widget and a layout in parent1 called layout1
    Tui::ZRadioButton *radiobutton1
             = new Tui::ZRadioButton(Tui::withMarkup, "<m>B</m>lue", parent1);
    layout1->addWidget(radiobutton1);

    Tui::ZRadioButton *radiobutton2
             = new Tui::ZRadioButton(Tui::withMarkup, "<m>G</m>reen", parent1);
    layout1->addWidget(radiobutton2);

    // Second group using parent2 widget and a layout in parent2 called layout2
    Tui::ZRadioButton *radiobutton4
             = new Tui::ZRadioButton(Tui::withMarkup, "<m>R</m>ound", parent2);
    layout2->addWidget(radiobutton4);

    Tui::ZRadioButton *radiobutton3
             = new Tui::ZRadioButton(Tui::withMarkup, "<m>S</m>quare", parent2);
    layout2->addWidget(radiobutton3);
// snippet-2group-end
}
