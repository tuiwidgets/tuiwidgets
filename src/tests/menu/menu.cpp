#include <Tui/ZMenu.h>
#include <Tui/ZMenubar.h>

#include <../../third-party/catch.hpp>

#include <QVector>

#include <Tui/ZCommandNotifier.h>
#include <Tui/ZPalette.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZWindowFacet.h>

#include "../Testhelper.h"

TEST_CASE("menuitem", "") {

    SECTION("default constructor") {
        Tui::ZMenuItem mi;
        CHECK(mi.markup() == "");
        CHECK(mi.fakeShortcut() == "");
        CHECK(mi.command() == Tui::ZSymbol());
        CHECK(mi.hasSubitems() == false);
        CHECK(mi.subitems().size() == 0);
    }

    SECTION("item constructor 1") {
        Tui::ZMenuItem mi{"some", "ctrl-Fake", "blub", {}};
        CHECK(mi.markup() == "some");
        CHECK(mi.fakeShortcut() == "ctrl-Fake");
        CHECK(mi.command() == Tui::ZSymbol("blub"));
        CHECK(mi.hasSubitems() == false);
        CHECK(mi.subitems().size() == 0);
    }

    SECTION("item constructor 2") {
        Tui::ZMenuItem mi{"some", "ctrl-Fake", "blub", {
                        {"inner", "ctrl-U", "blah", {}}
                    }};
        CHECK(mi.markup() == "some");
        CHECK(mi.fakeShortcut() == "ctrl-Fake");
        CHECK(mi.command() == Tui::ZSymbol("blub"));
        CHECK(mi.hasSubitems() == true);
        REQUIRE(mi.subitems().size() == 1);
        auto &subItem = mi.subitems()[0];
        CHECK(subItem.markup() == "inner");
        CHECK(subItem.fakeShortcut() == "ctrl-U");
        CHECK(subItem.command() == Tui::ZSymbol("blah"));
        CHECK(subItem.hasSubitems() == false);
        CHECK(subItem.subitems().size() == 0);
    }

    SECTION("generator constructor, no context") {
        bool called = false;
        auto generator = [&] {
            CHECK(!called);
            called = true;
            return QVector<Tui::ZMenuItem>{{"inner", "ctrl-U", "blah", {}}};
        };

        Tui::ZMenuItem mi{"some", nullptr, generator};
        CHECK(mi.markup() == "some");
        CHECK(mi.fakeShortcut() == "");
        CHECK(mi.command() == Tui::ZSymbol());
        CHECK(mi.hasSubitems() == true);
        CHECK(called == false);
        auto subitems = mi.subitems();
        CHECK(called == true);
        REQUIRE(subitems.size() == 1);
        auto &subItem = subitems[0];
        CHECK(subItem.markup() == "inner");
        CHECK(subItem.fakeShortcut() == "ctrl-U");
        CHECK(subItem.command() == Tui::ZSymbol("blah"));
        CHECK(subItem.hasSubitems() == false);
        CHECK(subItem.subitems().size() == 0);
        SECTION("setSubitems({})") {
            mi.setSubitems({});
            CHECK(mi.hasSubitems() == false);
            CHECK(mi.subitems().size() == 0);
        }
        SECTION("setSubitems({subitem})") {
            mi.setSubitems({{"second", "ctrl-X", "zwei", {}}});
            CHECK(mi.hasSubitems() == true);
            auto subitems2 = mi.subitems();
            REQUIRE(subitems2.size() == 1);
            auto &subItem2 = subitems2[0];
            CHECK(subItem2.markup() == "second");
            CHECK(subItem2.fakeShortcut() == "ctrl-X");
            CHECK(subItem2.command() == Tui::ZSymbol("zwei"));
            CHECK(subItem2.hasSubitems() == false);
            CHECK(subItem2.subitems().size() == 0);
        }
    }

    SECTION("generator constructor, with context") {
        bool called = false;
        auto generator = [&] {
            CHECK(!called);
            called = true;
            return QVector<Tui::ZMenuItem>{{"inner", "ctrl-U", "blah", {}}};
        };

        QObject *context = new QObject();

        Tui::ZMenuItem mi{"some", context, generator};
        CHECK(mi.markup() == "some");
        CHECK(mi.fakeShortcut() == "");
        CHECK(mi.command() == Tui::ZSymbol());
        CHECK(mi.hasSubitems() == true);
        CHECK(called == false);
        auto subitems = mi.subitems();
        CHECK(called == true);
        REQUIRE(subitems.size() == 1);
        auto &subItem = subitems[0];
        CHECK(subItem.markup() == "inner");
        CHECK(subItem.fakeShortcut() == "ctrl-U");
        CHECK(subItem.command() == Tui::ZSymbol("blah"));
        CHECK(subItem.hasSubitems() == false);
        CHECK(subItem.subitems().size() == 0);
        SECTION("setSubitems({})") {
            mi.setSubitems({});
            CHECK(mi.hasSubitems() == false);
            CHECK(mi.subitems().size() == 0);
        }
        SECTION("setSubitems({subitem})") {
            mi.setSubitems({{"second", "ctrl-X", "zwei", {}}});
            CHECK(mi.hasSubitems() == true);
            auto subitems2 = mi.subitems();
            REQUIRE(subitems2.size() == 1);
            auto &subItem2 = subitems2[0];
            CHECK(subItem2.markup() == "second");
            CHECK(subItem2.fakeShortcut() == "ctrl-X");
            CHECK(subItem2.command() == Tui::ZSymbol("zwei"));
            CHECK(subItem2.hasSubitems() == false);
            CHECK(subItem2.subitems().size() == 0);
        }
        SECTION("delete context") {
            delete context;
            context = nullptr;
            CHECK(mi.hasSubitems() == false);
            CHECK(mi.subitems().size() == 0);
        }
        SECTION("setSubitems({subitem}), delete context") {
            mi.setSubitems({{"second", "ctrl-X", "zwei", {}}});
            delete context;
            context = nullptr;
            CHECK(mi.hasSubitems() == true);
            auto subitems2 = mi.subitems();
            REQUIRE(subitems2.size() == 1);
            auto &subItem2 = subitems2[0];
            CHECK(subItem2.markup() == "second");
            CHECK(subItem2.fakeShortcut() == "ctrl-X");
            CHECK(subItem2.command() == Tui::ZSymbol("zwei"));
            CHECK(subItem2.hasSubitems() == false);
            CHECK(subItem2.subitems().size() == 0);
        }
        delete context;
    }

    SECTION("copy/move") {
        Tui::ZMenuItem miOrig{"some", "ctrl-Fake", "blub", {
                        {"inner", "ctrl-U", "blah", {}}
                    }};
        auto test = [](Tui::ZMenuItem &mi) {
            CHECK(mi.markup() == "some");
            CHECK(mi.fakeShortcut() == "ctrl-Fake");
            CHECK(mi.command() == Tui::ZSymbol("blub"));
            CHECK(mi.hasSubitems() == true);
            REQUIRE(mi.subitems().size() == 1);
            auto &subItem = mi.subitems()[0];
            CHECK(subItem.markup() == "inner");
            CHECK(subItem.fakeShortcut() == "ctrl-U");
            CHECK(subItem.command() == Tui::ZSymbol("blah"));
            CHECK(subItem.hasSubitems() == false);
            CHECK(subItem.subitems().size() == 0);
        };

        auto testIndependence = [&](Tui::ZMenuItem &mi) {
            miOrig.setMarkup("other");
            miOrig.setFakeShortcut("None");
            miOrig.setCommand("DifferentCommand");
            miOrig.setSubitems({});
            test(mi);
        };

        auto testDefaultValues = [](Tui::ZMenuItem &mi) {
            CHECK(mi.markup() == "");
            CHECK(mi.fakeShortcut() == "");
            CHECK(mi.command() == Tui::ZSymbol());
            CHECK(mi.hasSubitems() == false);
            CHECK(mi.subitems().size() == 0);
        };

        SECTION("constructor") {
            Tui::ZMenuItem mi{miOrig};
            test(mi);
            testIndependence(mi);
        }

        SECTION("assignment") {
            Tui::ZMenuItem mi;
            mi = miOrig;
            test(mi);
            testIndependence(mi);
        }

        SECTION("move constructor") {
            Tui::ZMenuItem mi{std::move(miOrig)};
            test(mi);
            testDefaultValues(miOrig);
        }

        SECTION("move assignment") {
            Tui::ZMenuItem mi;
            mi = std::move(miOrig);
            test(mi);
            testDefaultValues(miOrig);
        }

    }

    SECTION("markup") {
        Tui::ZMenuItem mi;
        mi.setMarkup("omega");
        CHECK(mi.markup() == "omega");
    }

    SECTION("shortcut") {
        Tui::ZMenuItem mi;
        mi.setFakeShortcut("Ctrl-P");
        CHECK(mi.fakeShortcut() == "Ctrl-P");
    }

    SECTION("command") {
        Tui::ZMenuItem mi;
        mi.setCommand("SomeCommand");
        CHECK(mi.command() == Tui::ZSymbol("SomeCommand"));
    }

    SECTION("generator, no context") {
        bool called = false;
        auto generator = [&] {
            CHECK(!called);
            called = true;
            return QVector<Tui::ZMenuItem>{{"inner", "ctrl-U", "blah", {}}};
        };

        Tui::ZMenuItem mi;
        mi.setSubitemsGenerator(nullptr, generator);
        CHECK(mi.hasSubitems() == true);
        CHECK(called == false);
        auto subitems = mi.subitems();
        CHECK(called == true);
        REQUIRE(subitems.size() == 1);
        auto &subItem = subitems[0];
        CHECK(subItem.markup() == "inner");
        CHECK(subItem.fakeShortcut() == "ctrl-U");
        CHECK(subItem.command() == Tui::ZSymbol("blah"));
        CHECK(subItem.hasSubitems() == false);
        CHECK(subItem.subitems().size() == 0);
        SECTION("setSubitems({})") {
            mi.setSubitems({});
            CHECK(mi.hasSubitems() == false);
            CHECK(mi.subitems().size() == 0);
        }
        SECTION("setSubitems({subitem})") {
            mi.setSubitems({{"second", "ctrl-X", "zwei", {}}});
            CHECK(mi.hasSubitems() == true);
            auto subitems2 = mi.subitems();
            REQUIRE(subitems2.size() == 1);
            auto &subItem2 = subitems2[0];
            CHECK(subItem2.markup() == "second");
            CHECK(subItem2.fakeShortcut() == "ctrl-X");
            CHECK(subItem2.command() == Tui::ZSymbol("zwei"));
            CHECK(subItem2.hasSubitems() == false);
            CHECK(subItem2.subitems().size() == 0);
        }
    }

    SECTION("generator, with context") {
        bool called = false;
        auto generator = [&] {
            CHECK(!called);
            called = true;
            return QVector<Tui::ZMenuItem>{{"inner", "ctrl-U", "blah", {}}};
        };

        QObject *context = new QObject();

        Tui::ZMenuItem mi;
        mi.setSubitemsGenerator(context, generator);
        CHECK(mi.hasSubitems() == true);
        CHECK(called == false);
        auto subitems = mi.subitems();
        CHECK(called == true);
        REQUIRE(subitems.size() == 1);
        auto &subItem = subitems[0];
        CHECK(subItem.markup() == "inner");
        CHECK(subItem.fakeShortcut() == "ctrl-U");
        CHECK(subItem.command() == Tui::ZSymbol("blah"));
        CHECK(subItem.hasSubitems() == false);
        CHECK(subItem.subitems().size() == 0);
        SECTION("setSubitems({})") {
            mi.setSubitems({});
            CHECK(mi.hasSubitems() == false);
            CHECK(mi.subitems().size() == 0);
        }
        SECTION("setSubitems({subitem})") {
            mi.setSubitems({{"second", "ctrl-X", "zwei", {}}});
            CHECK(mi.hasSubitems() == true);
            auto subitems2 = mi.subitems();
            REQUIRE(subitems2.size() == 1);
            auto &subItem2 = subitems2[0];
            CHECK(subItem2.markup() == "second");
            CHECK(subItem2.fakeShortcut() == "ctrl-X");
            CHECK(subItem2.command() == Tui::ZSymbol("zwei"));
            CHECK(subItem2.hasSubitems() == false);
            CHECK(subItem2.subitems().size() == 0);
        }
        SECTION("delete context") {
            delete context;
            context = nullptr;
            CHECK(mi.hasSubitems() == false);
            CHECK(mi.subitems().size() == 0);
        }
        SECTION("setSubitems({subitem}), delete context") {
            mi.setSubitems({{"second", "ctrl-X", "zwei", {}}});
            delete context;
            context = nullptr;
            CHECK(mi.hasSubitems() == true);
            auto subitems2 = mi.subitems();
            REQUIRE(subitems2.size() == 1);
            auto &subItem2 = subitems2[0];
            CHECK(subItem2.markup() == "second");
            CHECK(subItem2.fakeShortcut() == "ctrl-X");
            CHECK(subItem2.command() == Tui::ZSymbol("zwei"));
            CHECK(subItem2.hasSubitems() == false);
            CHECK(subItem2.subitems().size() == 0);
        }
        delete context;
    }

    SECTION("subitems") {
        Tui::ZMenuItem mi;
        mi.setSubitems({{"second", "ctrl-X", "zwei", {}}});
        CHECK(mi.hasSubitems() == true);
        auto subitems2 = mi.subitems();
        REQUIRE(subitems2.size() == 1);
        auto &subItem2 = subitems2[0];
        CHECK(subItem2.markup() == "second");
        CHECK(subItem2.fakeShortcut() == "ctrl-X");
        CHECK(subItem2.command() == Tui::ZSymbol("zwei"));
        CHECK(subItem2.hasSubitems() == false);
        CHECK(subItem2.subitems().size() == 0);
    }

}

TEST_CASE("menubar", "") {

    Testhelper t("menu", "menubar", 15, 2);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 15, 2});

    SECTION("without-parent") {
        // The initialisation must not crash
        delete new Tui::ZMenubar();
    }

    SECTION("default") {
        // The initialisation must not crash.
        new Tui::ZMenubar(w);
        t.render();
    }

    Tui::ZMenubar *m = new Tui::ZMenubar(w);

    SECTION("defaults") {
        CHECK(m->sizePolicyV() == Tui::SizePolicy::Fixed);
        CHECK(m->sizePolicyH() == Tui::SizePolicy::Expanding);
        FAIL_CHECK_VEC(checkWidgetsDefaultsExcept(m, DefaultException::SizePolicyV
                                                   | DefaultException::SizePolicyH));
    }

    SECTION("geometry") {
        m->setGeometry({0, 0, 15, 1});
        t.render();
    }

    SECTION("geometry-with-items") {
        m->setGeometry({0, 0, 15, 2});
        m->setItems({{"File", "", "file", {}}, {"Edit", "", "edit", {}}, {"Options", "", "options", {}}});
        t.compare();
    }

    m->setGeometry({0, 0, 15, 1});

    SECTION("items-setempty") {
        m->setItems({});
        t.render();
    }
    SECTION("items-empty") {
        m->items();
        t.render();
    }

    SECTION("items-one") {
        m->setItems({{"File", "", "file", {}}});
        t.compare();
    }

    SECTION("items-one-utf8") {
        m->setItems({{"😎", "", "cool", {}}});
        t.compare();
    }

    SECTION("items-three") {
        m->setItems({{"File", "", "file", {}}, {"Edit", "", "edit", {}}, {"Options", "", "options", {}}});
        new Tui::ZCommandNotifier("file", m);
        t.compare();
    }

    SECTION("items-three-f10") {
        m->setItems({{"File", "", "file", {}}, {"Edit", "", "edit", {}}, {"Options", "", "options", {}}});
        new Tui::ZCommandNotifier("file", m);
        t.sendKey(Qt::Key_F10);
        t.compare("items-three-file");
    }

    SECTION("items-three-leftkey") {
        m->setItems({{"File", "", "file", {}}, {"Edit", "", "edit", {}}, {"Options", "", "options", {}}});
        new Tui::ZCommandNotifier("file", m);
        t.sendKey(Qt::Key_F10);
        t.sendKey(Qt::Key_Left);
        t.compare("items-three-options");
    }

    SECTION("items-three-leftfunc") {
        m->setItems({{"File", "", "file", {}}, {"Edit", "", "edit", {}}, {"Options", "", "options", {}}});
        new Tui::ZCommandNotifier("file", m);
        t.sendKey(Qt::Key_F10);
        m->left();
        t.compare("items-three-options");
    }

    SECTION("items-three-close") {
        m->setItems({{"File", "", "file", {}}, {"Edit", "", "edit", {}}, {"Options", "", "options", {}}});
        new Tui::ZCommandNotifier("file", m);
        t.sendKey(Qt::Key_F10);
        m->close();
        t.compare("items-three");
    }

    SECTION("items-three-esc") {
        m->setItems({{"File", "", "file", {}}, {"Edit", "", "edit", {}}, {"Options", "", "options", {}}});
        new Tui::ZCommandNotifier("file", m);
        t.sendKey(Qt::Key_F10);
        t.sendKey(Qt::Key_Escape);
        t.compare("items-three");
    }

    SECTION("items-three-rightkey") {
        m->setItems({{"File", "", "file", {}}, {"Edit", "", "edit", {}}, {"Options", "", "options", {}}});
        new Tui::ZCommandNotifier("file", m);
        t.sendKey(Qt::Key_F10);
        t.sendKey(Qt::Key_Right);
        t.compare("items-three-edit");
    }

    SECTION("items-three-rightfunc") {
        m->setItems({{"File", "", "file", {}}, {"Edit", "", "edit", {}}, {"Options", "", "options", {}}});
        new Tui::ZCommandNotifier("file", m);
        t.sendKey(Qt::Key_F10);
        m->right();
        t.compare("items-three-edit");
    }

    SECTION("items-two-markup") {
        m->setItems({{"<m>F</m>ile", "", "file", {}}, {"Edi<m>t</m>", "", {}, {}}});
        new Tui::ZCommandNotifier("file", m);
        t.compare();
    }

    SECTION("items-two-markup") {
        m->setItems({{"<m>F</m>ile", "", "file", {}}, {"Edi<m>t</m>", "", {}, {}}});
        new Tui::ZCommandNotifier("file", m);
        // Menu "Edit" has no ZCommandNotifier. The menu item should not be highlighted.
        t.sendChar("t", Qt::AltModifier);
        t.compare("items-two-markup");
    }

    SECTION("items-three-f10-f10") {
        m->setItems({{"File", "", "file", {}}, {"Edit", "", "edit", {}}, {"Options", "", "options", {}}});
        new Tui::ZCommandNotifier("file", m);
        t.sendKey(Qt::Key_F10);
        m->left();
        t.compare("items-three-options");
        t.sendKey(Qt::Key_F10);
        t.compare("items-three");
    }

    SECTION("items-two-disable") {
        m->setItems({{"<m>F</m>ile", "", "file", {}}, {"Edit", "", "edit", {}}});
        new Tui::ZCommandNotifier("file", m);
        m->setEnabled(false);
        t.compare();
    }

    SECTION("items-one-disable-notifier") {
        m->setItems({{"<m>F</m>ile", "", "file", {}}});
        Tui::ZCommandNotifier *cm = new Tui::ZCommandNotifier("file", m);
        QObject::connect(cm, &Tui::ZCommandNotifier::activated, [] {
            FAIL("May not be called.");
        });
        cm->setEnabled(false);
        t.sendChar("f", Qt::AltModifier);
        // The menu item should not be highlighted.
        t.compare("items-one");
    }

    SECTION("items-one-disable-f10") {
        m->setItems({{"<m>F</m>ile", "", "file", {}}});
        Tui::ZCommandNotifier *cm = new Tui::ZCommandNotifier("file", m);
        cm->setEnabled(false);
        t.sendKey(Qt::Key_F10);
        t.compare();
    }

    SECTION("items-three-out") {
        m->setItems({{"<m>F</m>ile", "", "file", {}}, {"<m>E</m>dit", "", "edit", {}}, {"Option<m>s</m>", "", "options", {}}});
        new Tui::ZCommandNotifier("file", m);
        new Tui::ZCommandNotifier("edit", m);
        new Tui::ZCommandNotifier("options", m);
        t.compare();
    }

    SECTION("items-three-get") {
        CHECK(m->items().size() == 0);
        QVector<Tui::ZMenuItem> mi = {{"File", "", "file", {}}, {"Edit", "", "edit", {}}, {"Options", "", "options", {}}};
        m->setItems(mi);
        CHECK(m->items().size() == 3);
        CHECK(m->items().last().markup() == "Options");
    }

    SECTION("items-three-key") {
        m->setItems({{"<m>F</m>ile", "", "file", {}}, {"<m>E</m>dit", "", "edit", {}}, {"Option<m>s</m>", "", "options", {}}});
        new Tui::ZCommandNotifier("file", m);
        new Tui::ZCommandNotifier("edit", m);
        new Tui::ZCommandNotifier("options", m);
        t.sendKey(Qt::Key_F10);
        t.sendChar("e", Qt::AltModifier);
        t.compare("items-three-out");
    }

    SECTION("emit") {
        m->setItems({{"File", "", "file", {}}, {"<m>e</m>mit", "", "command_name", {}}, {"Options", "", "options", {}}});
        int trigger = -1;
        QObject::connect(new Tui::ZCommandNotifier("file", m), &Tui::ZCommandNotifier::activated, [&trigger] {
            CHECK(trigger == 0);
            trigger = -1;
        });
        QObject::connect(new Tui::ZCommandNotifier("command_name", m), &Tui::ZCommandNotifier::activated, [&trigger] {
            CHECK(trigger == 1);
            trigger = -1;
        });
        QObject::connect(new Tui::ZCommandNotifier("options", m), &Tui::ZCommandNotifier::activated, [&trigger] {
            CHECK(trigger == 2);
            trigger = -1;
        });
        trigger = 1;
        t.sendChar("e", Qt::AltModifier);
        CHECK(trigger == -1);
        t.sendKey(Qt::Key_Down);
        CHECK(trigger == -1);
        t.sendKey(Qt::Key_Up);
        CHECK(trigger == -1);
        t.sendKey(Qt::Key_Right);
        CHECK(trigger == -1);
        t.sendKey(Qt::Key_Left);
        CHECK(trigger == -1);
        t.sendKey(Qt::Key_Escape);
        CHECK(trigger == -1);
    }

    SECTION("emit-f10") {
        m->setItems({{"File", "", "file", {}}, {"<m>e</m>mit", "", "command_name", {}}, {"Options", "", "options", {}}});
        int trigger = -1;
        QObject::connect(new Tui::ZCommandNotifier("file", m), &Tui::ZCommandNotifier::activated, [&trigger] {
            CHECK(trigger == 0);
            trigger = -1;
        });
        QObject::connect(new Tui::ZCommandNotifier("command_name", m), &Tui::ZCommandNotifier::activated, [&trigger] {
            CHECK(trigger == 1);
            trigger = -1;
        });
        QObject::connect(new Tui::ZCommandNotifier("options", m), &Tui::ZCommandNotifier::activated, [&trigger] {
            CHECK(trigger == 2);
            trigger = -1;
        });

        t.sendKey(Qt::Key_F10);
        CHECK(trigger == -1);
        trigger = 0;
        t.sendKey(Qt::Key_Enter);
        CHECK(trigger == -1);

        t.sendKey(Qt::Key_F10);
        t.sendKey(Qt::Key_Down);
        CHECK(trigger == -1);
        t.sendKey(Qt::Key_Up);
        CHECK(trigger == -1);
        t.sendKey(Qt::Key_Escape);

        t.sendKey(Qt::Key_F10);
        t.sendKey(Qt::Key_Right);
        CHECK(trigger == -1);
        t.sendKey(Qt::Key_Left);
        CHECK(trigger == -1);
        t.sendKey(Qt::Key_Escape);
        CHECK(trigger == -1);
    }

    SECTION("emit-notify-before-setItems") {
        int trigger = -1;
        QObject::connect(new Tui::ZCommandNotifier("command_name", m), &Tui::ZCommandNotifier::activated, [&trigger] {
            CHECK(trigger == 0);
            trigger = -1;
        });
        m->setItems({{"File", "", "file", {}}, {"<m>e</m>mit", "", "command_name", {}}, {"Options", "", "options", {}}});
        trigger = 0;
        t.sendChar("e", Qt::AltModifier);
        CHECK(trigger == -1);
    }

    SECTION("emit-disable-notifier") {
        m->setItems({{"<m>F</m>ile", "", "file", {}}, {"<m>e</m>mit", "", "command_name", {}}, {"Options", "", "options", {}}});
        Tui::ZCommandNotifier *cm = new Tui::ZCommandNotifier("file", m);
        QObject::connect(cm, &Tui::ZCommandNotifier::activated, [] {
            FAIL("May not be called.");
        });
        cm->setEnabled(false);
        t.sendChar("f", Qt::AltModifier);
        t.sendKey(Qt::Key_F10);
        t.sendKey(Qt::Key_Enter);
    }
}

TEST_CASE("submenu", "") {
    Testhelper t("menu", "submenu", 30, 11);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 15, 11});

    Tui::ZMenubar *m = new Tui::ZMenubar(w);
    m->setGeometry({0, 0, 15, 1});

    m->setItems({
                    {"<m>o</m>ne", "", {}, {
                         {"Kilo", "", "kilo", {}},
                         {"Mega", "", "mega", {}},
                         {"<m>G</m>iga", "", "giga", {}},
                         {"Tera", "", "tera", {}},
                         {},
                         {"Peta", "", "peta", {}},
                         {"Exa", "", "exa", {}},
                         {"Zetta", "", "zetta", {}},
                         {"Yotta", "", "yotta", {}}
                     }},
                    {"Edit", "", {}, {
                         {"😀", "", "", {}},
                         {"😀😁", "", "", {}},
                         {"😀<m>😁</m>😂", "", "", {}},
                         {"😀😁<m>😂</m>😃", "", "", {}},
                         {"😀😁😂<m>😃</m>😇", "", "", {}},
                         {"😀😁😂😃<m>😇</m>😉", "", "angel", {}},
                         {"😀😁😂😃😇😉😍", "", "", {}},
                         {"😀😁😂😃😇😉😍😎", "", "", {}},
                     }},
                    {"Options", "", {}, {
                         {"8", "", "", {}},
                         {"<m>2</m>56", "", "two", {}},
                         {"1024", "", "", {}},
                         {"65535", "", "", {}},
                         {"16777216", "", "", {}},
                         {"9223372036854775807", "", "", {}}
                    }}});
    int trigger = -1;
    QObject::connect(new Tui::ZCommandNotifier("giga", m), &Tui::ZCommandNotifier::activated, [&trigger] {
        CHECK(trigger == 0);
        trigger = -1;
    });

    QObject::connect(new Tui::ZCommandNotifier("angel", m), &Tui::ZCommandNotifier::activated, [&trigger] {
        CHECK(trigger == 1);
        trigger = -1;
    });

    QObject::connect(new Tui::ZCommandNotifier("two", m), &Tui::ZCommandNotifier::activated, [&trigger] {
        CHECK(trigger == 2);
        trigger = -1;
    });

    new Tui::ZCommandNotifier("mega", m);

    SECTION("menu-window-facet-configuration") {
        t.sendChar("o", Qt::AltModifier);
        REQUIRE(t.terminal->keyboardGrabber());
        QObject *facetObject = t.terminal->keyboardGrabber()->facet(Tui::ZWindowFacet::staticMetaObject);
        REQUIRE(facetObject != nullptr);
        REQUIRE(facetObject->metaObject()->inherits(&Tui::ZWindowFacet::staticMetaObject));
        auto *winFacet = static_cast<Tui::ZWindowFacet*>(facetObject);
        CHECK(winFacet->isExtendViewport() == true);
        CHECK(winFacet->isManuallyPlaced() == true);
    }

    SECTION("geometry-w15-11-left") {
        t.sendChar("o", Qt::AltModifier);
        t.compare();
    }

    SECTION("geometry-w15-11-right") {
        t.sendChar("o", Qt::AltModifier);
        t.sendKey(Qt::Key_Left);
        t.compare();
    }

    SECTION("esc") {
        t.sendChar("o", Qt::AltModifier);
        t.render();
        t.sendKey(Qt::Key_Escape);
        t.compare();
    }

    SECTION("enter") {
        t.sendChar("o", Qt::AltModifier);
        t.sendKey(Qt::Key_Down);
        t.sendKey(Qt::Key_Down);
        t.render();
        trigger = 0;
        t.sendKey(Qt::Key_Enter);
        CHECK(trigger == -1);
        t.compare("esc");
    }

    SECTION("F10") {
        t.sendChar("o", Qt::AltModifier);
        t.render();
        t.sendKey(Qt::Key_F10);
        t.compare("esc");
    }

    SECTION("alt-x") {
        t.sendChar("x", Qt::AltModifier);
        t.compare("esc");
    }

    SECTION("only-o") {
        t.sendChar("o");
        t.compare("esc");
    }

    SECTION("emit") {
        t.sendChar("o", Qt::AltModifier);
        trigger = 0;
        t.sendChar("g");
        CHECK(trigger == -1);
        t.compare("esc");
    }

    SECTION("emit-number") {
        t.sendChar("o", Qt::AltModifier);
        t.sendKey(Qt::Key_Left);
        trigger = 2;
        t.sendChar("2");
        CHECK(trigger == -1);
        t.compare("esc");
    }

    SECTION("alt-utf8") {
        t.sendChar("😇", Qt::AltModifier);
        t.compare("esc");
    }

    SECTION("emit-utf8") {
        t.sendChar("o", Qt::AltModifier);
        t.sendKey(Qt::Key_Right);
        trigger = 1;
        t.sendChar("😇");
        CHECK(trigger == -1);
        t.compare("esc");
    }

    m->setGeometry({0, 0, 30, 1});

    SECTION("geometry-m30-1") {
        t.sendChar("o", Qt::AltModifier);
        t.compare("geometry-w15-11-left");
    }

    w->setGeometry({0, 0, 30, 11});

    SECTION("geometry-w30-11-left") {
        t.sendChar("o", Qt::AltModifier);
        t.compare();
    }

    SECTION("geometry-w30-11-right") {
        t.sendChar("o", Qt::AltModifier);
        t.sendKey(Qt::Key_Left);
        t.compare();
    }

    SECTION("key-down") {
        t.sendChar("o", Qt::AltModifier);
        t.sendKey(Qt::Key_Down);
        t.compare();
    }

    SECTION("key-up") {
        t.sendChar("o", Qt::AltModifier);
        t.sendKey(Qt::Key_Up);
        t.compare();
    }

    SECTION("key-right") {
        t.sendChar("o", Qt::AltModifier);
        t.sendKey(Qt::Key_Right);
        t.compare();
    }

    SECTION("key-left") {
        t.sendChar("o", Qt::AltModifier);
        t.sendKey(Qt::Key_Left);
        t.compare("geometry-w30-11-right");
    }

    SECTION("reopen") {
        t.sendKey(Qt::Key_F10);
        t.sendKey(Qt::Key_Right);
        t.compare("key-right");
        t.sendChar("o", Qt::AltModifier);
        t.compare("geometry-w30-11-left");
    }

    SECTION("fake-shortcut") {
        m->setItems({
                        {"<m>o</m>ne", "", {}, {
                         {"Kilo", "Ctrl+K", "kilo", {}},
                         {"Mega", "", "mega", {}},
                         {"<m>G</m>iga", "", "giga", {}},
                         {"Tera", "", "tera", {}},
                         {},
                         {"Exa", "Ctrl+E", "exa", {}},
                        }}
                    });
        t.sendChar("o", Qt::AltModifier);
        t.compare();
    }

    SECTION("fake-shortcut-long") {
        m->setItems({
                        {"<m>o</m>ne", "", {}, {
                         {"Kilobytes", "〉Ctrl+K", "kilo", {}},
                         {"Megabytes", "", "mega", {}},
                         {"<m>G</m>igabytes", "", "giga", {}},
                         {"Terabytes", "", "tera", {}},
                         {},
                         {"Exabytes", "Ctrl+E", "exa", {}},
                        }}
                    });
        t.sendChar("o", Qt::AltModifier);
        t.compare();
    }

    // black
    t.root->setPalette(Tui::ZPalette::black());
    SECTION("black-key-down") {
        t.sendChar("o", Qt::AltModifier);
        t.sendKey(Qt::Key_Down);
        t.compare();
    }

    SECTION("black-key-right") {
        t.sendChar("o", Qt::AltModifier);
        t.sendKey(Qt::Key_Right);
        t.compare();
    }

    SECTION("black-key-left") {
        t.sendChar("o", Qt::AltModifier);
        t.sendKey(Qt::Key_Left);
        t.compare();
    }

    // cyan
    t.root->setPalette(Tui::ZPalette::classic());
    w->setPaletteClass({"window", "cyan"});
    SECTION("cyan-key-down") {
        t.sendChar("o", Qt::AltModifier);
        t.sendKey(Qt::Key_Down);
        t.compare();
    }

    SECTION("cyan-key-right") {
        t.sendChar("o", Qt::AltModifier);
        t.sendKey(Qt::Key_Right);
        t.compare();
    }

    SECTION("cyan-key-left") {
        t.sendChar("o", Qt::AltModifier);
        t.sendKey(Qt::Key_Left);
        t.compare();
    }
}

TEST_CASE("popupmenu", "") {
    Testhelper t("menu", "popupmenu", 30, 15);
    t.root->ensureCommandManager();
    Tui::ZWindow *w = new TestBackground(t.root);
    w->setGeometry({0, 0, 30, 15});

    QVector<Tui::ZMenuItem> items1 = {
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
    QVector<Tui::ZMenuItem> items2 = {
                                    {"😀", "", "", {}},
                                    {"😀😁", "", "", {}},
                                    {"😀<m>😁</m>😂", "", "", {}},
                                    {"😀😁<m>😂</m>😃", "", "", {}},
                                    {"😀😁😂<m>😃</m>😇", "", "", {}},
                                    {"😀😁😂😃<m>😇</m>😉", "", "angel", {}},
                                    {"😀😁😂😃😇😉😍", "", "", {}},
                                    {"😀😁😂😃😇😉😍😎", "", "", {}},
                                };

    QVector<Tui::ZMenuItem> items3 = {
                                    {"8", "", "", {}},
                                    {"<m>2</m>56", "", "two", {}},
                                    {"1024", "", "", {}},
                                    {"65535", "", "", {}},
                                    {"16777216", "", "", {}},
                                    {"9223372036854775807", "", "", {}}
                               };
    int trigger = -1;
    QObject::connect(new Tui::ZCommandNotifier("giga", t.root), &Tui::ZCommandNotifier::activated, [&trigger] {
        CHECK(trigger == 0);
        trigger = -1;
    });

    QObject::connect(new Tui::ZCommandNotifier("angel", t.root), &Tui::ZCommandNotifier::activated, [&trigger] {
        CHECK(trigger == 1);
        trigger = -1;
    });

    QObject::connect(new Tui::ZCommandNotifier("two", t.root), &Tui::ZCommandNotifier::activated, [&trigger] {
        CHECK(trigger == 2);
        trigger = -1;
    });

    new Tui::ZCommandNotifier("mega", t.root);

    auto *menu = new Tui::ZMenu(t.root);

    SECTION("defaults") {
        FAIL_CHECK_VEC(checkWidgetsDefaultsExcept(menu, {}));
    }

    SECTION("menu-window-facet-configuration") {
        menu->setItems(items1);
        menu->popup({1, 1});
        REQUIRE(t.terminal->keyboardGrabber());
        QObject *facetObject = t.terminal->keyboardGrabber()->facet(Tui::ZWindowFacet::staticMetaObject);
        REQUIRE(facetObject != nullptr);
        REQUIRE(facetObject->metaObject()->inherits(&Tui::ZWindowFacet::staticMetaObject));
        auto *winFacet = static_cast<Tui::ZWindowFacet*>(facetObject);
        CHECK(winFacet->isExtendViewport() == true);
        CHECK(winFacet->isManuallyPlaced() == true);
    }

    SECTION("too-narrow") {
        t.terminal->resize(10, 15);
        menu->setItems(items1);
        menu->popup({5, 1});
        t.compare();
    }

    SECTION("left") {
        menu->setItems(items1);
        menu->popup({1, 1});
        t.compare();
    }

    SECTION("mid") {
        // just another position
        menu->setItems(items1);
        menu->popup({5, 2});
        t.compare();
    }

    SECTION("too-much-left") {
        // menu is placed partially outside of parent, it automatically moves right to keep visible
        menu->setItems(items1);
        menu->popup({-5, 1});
        t.compare();
    }

    SECTION("too-much-right") {
        // menu is placed partially outside of parent, it automatically moves left to keep visible
        menu->setItems(items1);
        menu->popup({25, 1});
        t.compare();
    }

    SECTION("esc") {
        menu->setItems(items1);
        menu->popup({1, 1});
        t.render();
        t.sendKey(Qt::Key_Escape);
        t.compare();
    }

    SECTION("enter") {
        menu->setItems(items1);
        menu->popup({1, 1});
        t.sendKey(Qt::Key_Down);
        t.sendKey(Qt::Key_Down);
        t.render();
        trigger = 0;
        t.sendKey(Qt::Key_Enter);
        CHECK(trigger == -1);
        t.compare("esc");
    }

    SECTION("F10") {
        menu->setItems(items1);
        menu->popup({1, 1});
        t.render();
        t.sendKey(Qt::Key_F10);
        t.compare("esc");
    }
    SECTION("emit") {
        menu->setItems(items1);
        menu->popup({1, 1});
        trigger = 0;
        t.sendChar("g");
        CHECK(trigger == -1);
        t.compare("esc");
    }

    SECTION("emit-utf8") {
        menu->setItems(items2);
        menu->popup({1, 1});
        trigger = 1;
        t.sendChar("😇");
        CHECK(trigger == -1);
        t.compare("esc");
    }

    SECTION("emit-number") {
        menu->setItems(items3);
        menu->popup({1, 1});
        trigger = 2;
        t.sendChar("2");
        CHECK(trigger == -1);
        t.compare("esc");
    }

    SECTION("key-down") {
        menu->setItems(items2);
        menu->popup({1, 1});
        t.sendKey(Qt::Key_Down);
        t.compare();
    }

    SECTION("key-up") {
        menu->setItems(items2);
        menu->popup({1, 1});
        t.sendKey(Qt::Key_Up);
        t.compare();
    }

    SECTION("fake-shortcut") {
        // The menu contents is less than the minimal width. The Shortcut box is right aligned.
        menu->setItems({
                         {"Kilo", "Ctrl+K", "kilo", {}},
                         {"Mega", "", "mega", {}},
                         {"<m>G</m>iga", "", "giga", {}},
                         {"Tera", "", "tera", {}},
                         {},
                         {"Exa", "Ctrl+E", "exa", {}},
                    });
        menu->popup({1, 1});
        t.sendChar("o", Qt::AltModifier);
        t.compare();
    }

    SECTION("fake-shortcut-long") {
        // The menu contents is more than the minimal width. The Shortcuts box is seperated by a single
        // cell from the longest menu entry.
        menu->setItems({
                         {"Kilobytes", "〉Ctrl+K", "kilo", {}},
                         {"Megabytes", "", "mega", {}},
                         {"<m>G</m>igabytes", "", "giga", {}},
                         {"Terabytes", "", "tera", {}},
                         {},
                         {"Exabytes", "Ctrl+E", "exa", {}},
                    });
        menu->popup({1, 1});
        t.sendChar("o", Qt::AltModifier);
        t.compare();
    }

    // black
    t.root->setPalette(Tui::ZPalette::black());
    SECTION("black-key-down") {
        menu->setItems(items1);
        menu->popup({1, 1});
        t.sendKey(Qt::Key_Down);
        t.compare();
    }

    // cyan
    t.root->setPalette(Tui::ZPalette::classic());
    w->setPaletteClass({"window", "cyan"});
    SECTION("cyan-key-down") {
        menu->setItems(items1);
        menu->popup({1, 1});
        t.sendKey(Qt::Key_Down);
        t.compare();
    }
}

TEST_CASE("menubar-margin", "") {
    Testhelper t("menu", "menubar-margin", 15, 7);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 15, 7});

    SECTION("margin-2-2-2-2") {
        Tui::ZMenubar *m = new Tui::ZMenubar(w);
        QVector<Tui::ZMenuItem> mi = {{"File", "", "file", {}}, {"Edit", "", "edit", {}}, {"Options", "", "options", {}}};
        m->setItems(mi);
        m->setGeometry({1, 1, 13, 5});
        t.render();
        CHECK(m->sizeHint().height() == 1);
        CHECK(m->sizeHint().width() == 22);
        m->setContentsMargins({2, 2, 2, 2});
        t.render();
        //TODO: implement margins
        //CHECK(m->sizeHint().height() == 5);
        //CHECK(m->sizeHint().width() == 26);
    }
}

TEST_CASE("without-terminal", "") {
    Tui::ZWindow *w = new Tui::ZWindow(nullptr);
    w->setGeometry({0, 0, 15, 5});
    Tui::ZMenubar *m = new Tui::ZMenubar(w);
    m->sizeHint();
    m->right();
    m->left();
    m->close();
    m->items();
    m->setItems({});
    CHECK(m->sizePolicyH() == Tui::SizePolicy::Expanding);
    CHECK(m->sizePolicyV() == Tui::SizePolicy::Fixed);
}
