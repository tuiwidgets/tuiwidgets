#include <Tui/ZCommandNotifier.h>
#include <Tui/ZHBoxLayout.h>
#include <Tui/ZMenubar.h>
#include <Tui/ZWidget.h>

void exampleMenubarItems() {
// snippet-initlist-items-start
    QVector<Tui::ZMenuItem> items = {
        { "<m>F</m>ile", "", {}, {
            { "<m>N</m>ew", "", "NewFile", {}},
            { "<m>O</m>pen", "", "OpenFile", {}},
            { "<m>S</m>ave", "", "SaveFile", {}},
            { },
            { "<m>Q</m>uit", "Ctrl-q", "Quit", {}},
        }},
        { "<m>H</m>elp", "", {}, {
            { "<m>A</m>bout", "", "AboutDialog", {}}
        }}
    };
// snippet-initlist-items-end
}

void exampleMenubar(Tui::ZWidget *root, Tui::ZHBoxLayout *layout) {
// snippet-start
    Tui::ZMenubar *menubar = new Tui::ZMenubar(root);

    QVector<Tui::ZMenuItem> items = {
        { "<m>F</m>ile", "", {}, {
            { "<m>N</m>ew", "", "NewFile", {}},
            { "<m>O</m>pen", "", "OpenFile", {}},
            { "<m>S</m>ave", "", "SaveFile", {}},
            { },
            { "<m>Q</m>uit", "Ctrl-q", "Quit", {}},
        }},
        { "<m>H</m>elp", "", {}, {
            { "<m>A</m>bout", "", "AboutDialog", {}}
        }}
    };
    menubar->setItems(items);

    layout->addWidget(menubar);

    QObject::connect(new Tui::ZCommandNotifier("NewFile", root),
                     &Tui::ZCommandNotifier::activated, [&] { /* ... */ });

    QObject::connect(new Tui::ZCommandNotifier("OpenFile", root),
                     &Tui::ZCommandNotifier::activated, [&] { /* ... */ });

    Tui::ZCommandNotifier *saveCommand = new Tui::ZCommandNotifier("SaveFile", root);
    QObject::connect(saveCommand, &Tui::ZCommandNotifier::activated, [&] { /* ... */ });

    saveCommand->setEnabled(false);

    QObject::connect(new Tui::ZCommandNotifier("Quit", root),
                     &Tui::ZCommandNotifier::activated, [&] { /* ... */ });

    QObject::connect(new Tui::ZCommandNotifier("AboutDialog", root),
                     &Tui::ZCommandNotifier::activated, [&] { /* ... */ });
// snippet-end
}
