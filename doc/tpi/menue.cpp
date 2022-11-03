#include "menue.h"

void menue() {
    menuebar();
    submenu();
    popupmenu();
}

void menuebar() {

    Tui::ZTerminal terminal (Tui::ZTerminal::OffScreen(40,40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);
    root.setGeometry({0,0,20,10});
    root.setFocus();

    Tui::ZWindow win(&root);
    win.setGeometry({0,0,20,10});
    win.setFocus();
    win.setBorderEdges({});

    QVector<Tui::ZMenuItem> items =  { { "<m>F</m>ile", "", {}, {
                                        { "<m>B</m>utton", "", "ButtonDialog", {}},
                                        { "<m>C</m>heckBox", "", "CheckBoxDialog", {}},
                                        { "<m>I</m>nputBox", "", "InputBoxDialog", {}},
                                        { "<m>R</m>adioButton", "", "RadioButtonDialog", {}},
                                        { "<m>L</m>istView", "", "ListViewDialog", {}},
                                        { "L<m>a</m>bel", "", "LabelDialog", {}},
                                        {},
                                        { "Search and Replace", "", "SearchAndReplaceDialog", {}},
                                        {},
                                        { "<m>Q</m>uit", "Ctrl-q", "Quit", {}},
                                        } },
                                    { "<m>H</m>elp", "", {}, {
                                        { "<m>A</m>bout", "", "AboutDialog", {}}
                                      }}};

    // Default
    {

        Tui::ZMenubar *menu = new Tui::ZMenubar(&win);
        menu->setGeometry({1,1,18,1});
        menu->setItems(items);

        export_tpi(&terminal, "menue", 0,0,20,3);
    }

    // disabled
    {
        Tui::ZMenubar *menu = new Tui::ZMenubar(&win);
        menu->setGeometry({1,1,18,1});
        menu->setItems(items);
        menu->setEnabled(false);

        export_tpi(&terminal, "menue-disabled", 0,0,20,3);
    }
}

void submenu() {

    Tui::ZTerminal terminal (Tui::ZTerminal::OffScreen(40,40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);
    root.setGeometry({0,0,20,10});
    root.setFocus();

    Tui::ZWindow win(&root);
    win.setGeometry({0,0,20,10});
    win.setFocus();
    win.setBorderEdges({});

    QVector<Tui::ZMenuItem> items =  { { "<m>F</m>ile", "", {}, {
                                        { "<m>B</m>utton", "", "ButtonDialog", {}},
                                        { "<m>C</m>heckBox", "", "CheckBoxDialog", {}},
                                        { "<m>I</m>nputBox", "", "InputBoxDialog", {}},
                                        { "<m>R</m>adioButton", "", "RadioButtonDialog", {}},
                                        { "<m>L</m>istView", "", "ListViewDialog", {}},
                                        { "L<m>a</m>bel", "", "LabelDialog", {}},
                                        {},
                                        { "Search and Replace", "", "SearchAndReplaceDialog", {}},
                                        {},
                                        { "<m>Q</m>uit", "Ctrl-q", "Quit", {}},
                                        } },
                                    { "<m>H</m>elp", "", {}, {
                                        { "<m>A</m>bout", "", "AboutDialog", {}}
                                      }}};

    // Default
    {
        Tui::ZMenubar *menu = new Tui::ZMenubar(&win);
        menu->setGeometry({1,1,18,1});
        menu->setItems(items);

        //TODO: how to open

        export_tpi(&terminal, "menue-submenu", 0,0,20,3);
    }

    // disabled
    {
        Tui::ZMenubar *menu = new Tui::ZMenubar(&win);
        menu->setGeometry({1,1,18,1});
        menu->setItems(items);
        menu->setEnabled(false);

        export_tpi(&terminal, "menue-submenu-disabled", 0,0,20,3);
    }
}

void popupmenu() {
    Tui::ZTerminal terminal (Tui::ZTerminal::OffScreen(40,40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);
    root.setGeometry({0,0,40,40});
    root.setFocus();

    Tui::ZWindow win(&root);
    win.setGeometry({0,0,40,40});
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

        Tui::ZMenu *menu = new Tui::ZMenu(&win);
        menu->setGeometry({1,1,40,40});
        menu->setItems(items);
        menu->popup({1, 1});

        QObject::connect(new Tui::ZCommandNotifier("mega", menu), &Tui::ZCommandNotifier::activated, [&] {

        });
        new Tui::ZCommandNotifier("giga", menu);
        new Tui::ZCommandNotifier("tera", menu);

        export_tpi(&terminal, "menue-popupmenu", 0,0,22,13);
    }

    // disabled
    {
        Tui::ZMenu *menu = new Tui::ZMenu(&win);
        menu->setGeometry({1,1,40,40});
        menu->setItems(items);
        menu->popup({1, 1});
        menu->setEnabled(false);

        export_tpi(&terminal, "menue-popupmenu-disabled", 0,0,22,13);
    }
}
