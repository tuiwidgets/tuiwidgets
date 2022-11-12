#include "tpi-image-builder.h"

#include <QObject>
#include <QRect>
#include <QVector>

#include <Tui/ZCommandNotifier.h>
#include <Tui/ZMenu.h>
#include <Tui/ZMenuItem.h>
#include <Tui/ZMenubar.h>
#include <Tui/ZRoot.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZTest.h>
#include <Tui/ZWindow.h>

void menubar() {
    Tui::ZTerminal terminal(Tui::ZTerminal::OffScreen(40, 40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);
    root.setGeometry({0, 0, 80, 24});
    root.setFocus();

    QVector<Tui::ZMenuItem> items =  { { "<m>F</m>ile", "", {}, {
                                           { "<m>N</m>ew", "", "NewFile", {}},
                                           { "<m>O</m>pen", "", "OpenFile", {}},
                                           { "<m>S</m>ave", "", "SaveFile", {}},
                                           { },
                                           { "<m>Q</m>uit", "Ctrl-q", "Quit", {}},
                                           } },
                                       { "<m>H</m>elp", "", {}, {
                                           { "<m>A</m>bout", "", "AboutDialog", {}}
                                      }}};

    // Default
    {
        Tui::ZMenubar menubar(&root);
        menubar.setGeometry({1, 1, 24, 1});
        menubar.setItems(items);
        export_tpi(&terminal, "menu", 0, 0, 26, 10);
    }

    // disabled
    {
        Tui::ZMenubar menubar(&root);
        menubar.setGeometry({1, 1, 24, 1});
        menubar.setItems(items);
        menubar.setEnabled(false);
        export_tpi(&terminal, "menu-disabled", 0, 0, 26, 10);
    }

    // submenu Default
    {
        Tui::ZMenubar menubar(&root);
        menubar.setGeometry({1, 1, 24, 1});
        menubar.setItems(items);

        QObject::connect(new Tui::ZCommandNotifier("NewFile", &root), &Tui::ZCommandNotifier::activated, [&] {
        });

        QObject::connect(new Tui::ZCommandNotifier("OpenFile", &root), &Tui::ZCommandNotifier::activated, [&] {
        });

        QObject::connect(new Tui::ZCommandNotifier("Quit", &root), &Tui::ZCommandNotifier::activated, [&] {
        });

        Tui::ZTest::sendKey(&terminal, Tui::Key_F10, {});

        export_tpi(&terminal, "menu-submenu", 0, 0, 26, 10);
    }

}

void popupmenu() {
    Tui::ZTerminal terminal (Tui::ZTerminal::OffScreen(40, 40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);
    root.setGeometry({0, 0, 40, 40});
    root.setFocus();

    Tui::ZWindow win(&root);
    win.setGeometry({0, 0, 40, 40});
    win.setFocus();
    win.setBorderEdges({});

    QVector<Tui::ZMenuItem> items =  { {"Kilo", "", "kilo", {}},
                                       {"Mega", "", "mega", {}},
                                       {"<m>G</m>iga", "", "giga", {}},
                                       {"Tera", "", "tera", {}},
                                       {},
                                       {"Peta", "", "peta", {}},
                                       {"Exa", "", "exa", {}},
                                       {"Zetta", "", "zetta", {}},
                                       {"Yotta", "", "yotta", {}}
                                     };


    // Default
    {
        root.ensureCommandManager();

        new Tui::ZCommandNotifier("mega", &root);
        new Tui::ZCommandNotifier("giga", &root);
        new Tui::ZCommandNotifier("tera", &root);

        Tui::ZMenu *menu = new Tui::ZMenu(&root);
        menu->setItems(items);
        menu->popup({1, 1});

        export_tpi(&terminal, "menue-popupmenu", 0, 0, 22, 13);
    }
}

void menu() {
    menubar();
    popupmenu();
}

