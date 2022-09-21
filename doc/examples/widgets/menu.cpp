#include <Tui/ZCommandNotifier.h>
#include <Tui/ZHBoxLayout.h>
#include <Tui/ZMenu.h>
#include <Tui/ZWidget.h>

void exampleMenu(Tui::ZWidget *root, Tui::ZHBoxLayout *layout) {
// snippet-start
    QVector<Tui::ZMenuItem> items =  {
        {"Kilo", "", "kilo", {}},
        {"Mega", "", "mega", {}},
        {"<m>G</m>iga", "", "giga", {}},
        {"Tera", "", "tera", {}},
        {},
        {"Peta", "", "peta", {}},
        {"Exa", "", "exa", {}},
        {"Zetta", "", "zetta", {}},
        {"Yotta", "", "yotta", {}}
    };

    Tui::ZMenu *menu = new Tui::ZMenu(root);
    menu->setItems(items);

    QObject::connect(new Tui::ZCommandNotifier("mega", root), 
                     &Tui::ZCommandNotifier::activated, [&] { /* ... */ });

    QObject::connect(new Tui::ZCommandNotifier("giga", root),
                     &Tui::ZCommandNotifier::activated, [&] { /* ... */ });

    QObject::connect(new Tui::ZCommandNotifier("tera", root),
                     &Tui::ZCommandNotifier::activated, [&] { /* ... */ });

    menu->popup({1, 1});
// snippet-end
}
