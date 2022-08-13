// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZTextOption.h>

#include <QList> // should this be needed?

#include <Tui/ZTextStyle.h>
#include <Tui/ZFormatRange.h>

#include "../catchwrapper.h"

TEST_CASE("textoption", "") {

    SECTION("initialisation") {
        delete new Tui::ZTextOption();
        delete new Tui::ZTextOption(Tui::ZTextOption());
    }

    std::unique_ptr<Tui::ZTextOption> to = std::make_unique<Tui::ZTextOption>();
    SECTION("defaults") {
        CHECK(to->flags() == Tui::ZTextOption::Flag{});
        CHECK(to->tabStopDistance() == 8);
        QList<Tui::ZTextOption::Tab> t = to->tabs();
        CHECK(t.size() == 0);
        QList<int> ta = to->tabArray();
        CHECK(ta.size() == 0);

        CHECK(to->wrapMode() == Tui::ZTextOption::WrapMode::WrapAnywhere);

        // mapTrailingWhitespaceColor tested in dedicated section
        // mapTabColor tested in dedicated section
    }

    SECTION("flags") {
        CHECK(to->flags() == Tui::ZTextOption::Flag{});
        to->setFlags(Tui::ZTextOption::Flag::ShowTabsAndSpaces);
        CHECK(to->flags() == Tui::ZTextOption::Flag::ShowTabsAndSpaces);
        to->setFlags(Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors);
        CHECK(to->flags() == Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors);
        to->setFlags(Tui::ZTextOption::Flag{});
        CHECK(to->flags() == Tui::ZTextOption::Flag{});
    }

    SECTION("tabStopDistance") {
        to->setTabStopDistance(65535);
        CHECK(to->tabStopDistance() == 65535);
        to->setTabStopDistance(0);
        CHECK(to->tabStopDistance() == 1);
        to->setTabStopDistance(-1);
        CHECK(to->tabStopDistance() == 1);
    }

    SECTION("tabs") {
        QList<Tui::ZTextOption::Tab> t = { Tui::ZTextOption::Tab(32, Tui::ZTextOption::LeftTab),
                                           Tui::ZTextOption::Tab(42, Tui::ZTextOption::LeftTab)};
        to->setTabs(t);
        REQUIRE(to->tabs().size() == 2);
        CHECK(to->tabs().at(0).position == 32);
        CHECK(to->tabs().at(0).type == Tui::ZTextOption::LeftTab);
        CHECK(to->tabs().at(1).position == 42);
        CHECK(to->tabs().at(1).type == Tui::ZTextOption::LeftTab);
        CHECK(to->tabArray().at(0) == 32);
        CHECK(to->tabArray().at(1) == 42);
    }
    SECTION("tabArray") {
        QList<int> t = { 32, 42 };
        to->setTabArray(t);
        REQUIRE(to->tabArray().size() == 2);
        CHECK(to->tabArray().at(0) == 32);
        CHECK(to->tabArray().at(1) == 42);
        CHECK(to->tabs().at(0).position == 32);
        CHECK(to->tabs().at(0).type == Tui::ZTextOption::LeftTab);
        CHECK(to->tabs().at(1).position == 42);
        CHECK(to->tabs().at(1).type == Tui::ZTextOption::LeftTab);
    }
    SECTION("wrapMode") {
        to->setWrapMode(Tui::ZTextOption::WrapMode::NoWrap);
        CHECK(to->wrapMode() == Tui::ZTextOption::WrapMode::NoWrap);
        to->setWrapMode(Tui::ZTextOption::WrapMode::WrapAnywhere);
        CHECK(to->wrapMode() == Tui::ZTextOption::WrapMode::WrapAnywhere);
        to->setWrapMode(Tui::ZTextOption::WrapMode::WordWrap);
        CHECK(to->wrapMode() == Tui::ZTextOption::WrapMode::WordWrap);
    }

    SECTION("mapTrailingWhitespaceColor") {
        // Default
        Tui::ZTextStyle ts = to->mapTrailingWhitespaceColor(Tui::ZTextStyle({0, 0, 0}, {0, 0, 0}, Tui::ZTextAttribute::Underline),
                                             Tui::ZTextStyle({11, 22, 33}, {44, 55, 66}, Tui::ZTextAttribute::Bold), nullptr);
        CHECK(ts.foregroundColor() == Tui::ZColor{11, 22, 33});
        CHECK(ts.backgroundColor() == Tui::ZColor{44, 55, 66});
        CHECK(ts.attributes() == Tui::ZTextAttributes(Tui::ZTextAttribute::Bold));

        Tui::ZFormatRange fr;
        fr.setFormat(Tui::ZTextStyle({77, 88, 99}, {100, 110, 120}, Tui::ZTextAttribute::Italic));
        ts = to->mapTrailingWhitespaceColor(Tui::ZTextStyle({0, 0, 0}, {0, 0, 0}, Tui::ZTextAttribute::Bold),
                             Tui::ZTextStyle({11, 22, 33}, {44, 55, 66}, Tui::ZTextAttribute::Bold), &fr);
        CHECK(ts.foregroundColor() == Tui::ZColor{77, 88, 99});
        CHECK(ts.backgroundColor() == Tui::ZColor{100, 110, 120});
        CHECK(ts.attributes() == Tui::ZTextAttributes(Tui::ZTextAttribute::Italic));

        // Set
        to->setTrailingWhitespaceColor([&fr](const Tui::ZTextStyle& base, const Tui::ZTextStyle& formatingCharColor, const Tui::ZFormatRange* range) {
            CHECK(base.foregroundColor() == Tui::ZColor{77, 88, 99});
            CHECK(base.backgroundColor() == Tui::ZColor{110, 112, 113});
            CHECK(base.attributes() == Tui::ZTextAttributes(Tui::ZTextAttribute::Inverse));

            CHECK(formatingCharColor.foregroundColor() == Tui::ZColor{11, 22, 33});
            CHECK(formatingCharColor.backgroundColor() == Tui::ZColor{44, 55, 66});
            CHECK(formatingCharColor.attributes() == Tui::ZTextAttributes(Tui::ZTextAttribute::Bold));

            CHECK(&fr == range);
            return Tui::ZTextStyle({140, 150, 160}, {170, 180, 190}, Tui::ZTextAttribute::Overline);
        });

        ts = to->mapTrailingWhitespaceColor(Tui::ZTextStyle({77, 88, 99}, {110, 112, 113}, Tui::ZTextAttribute::Inverse),
                                             Tui::ZTextStyle({11, 22, 33}, {44, 55, 66}, Tui::ZTextAttribute::Bold), &fr);
        CHECK(ts.foregroundColor() == Tui::ZColor{140, 150, 160});
        CHECK(ts.backgroundColor() == Tui::ZColor{170, 180, 190});
        CHECK(ts.attributes() == Tui::ZTextAttributes(Tui::ZTextAttribute::Overline));

        // Reset test
        to->setTrailingWhitespaceColor({});
        ts = to->mapTrailingWhitespaceColor(Tui::ZTextStyle({0, 0, 0}, {0, 0, 0}, Tui::ZTextAttribute::Underline),
                             Tui::ZTextStyle({11, 22, 33}, {44, 55, 66}, Tui::ZTextAttribute::Bold), nullptr);
        CHECK(ts.foregroundColor() == Tui::ZColor{11, 22, 33});
        CHECK(ts.backgroundColor() == Tui::ZColor{44, 55, 66});
        CHECK(ts.attributes() == Tui::ZTextAttributes(Tui::ZTextAttribute::Bold));

        fr.setFormat(Tui::ZTextStyle({77, 88, 99}, {100, 110, 120}, Tui::ZTextAttribute::Italic));
        ts = to->mapTrailingWhitespaceColor(Tui::ZTextStyle({0, 0, 0}, {0, 0, 0}, Tui::ZTextAttribute::Bold),
                             Tui::ZTextStyle({11, 22, 33}, {44, 55, 66}, Tui::ZTextAttribute::Bold), &fr);
        CHECK(ts.foregroundColor() == Tui::ZColor{77, 88, 99});
        CHECK(ts.backgroundColor() == Tui::ZColor{100, 110, 120});
        CHECK(ts.attributes() == Tui::ZTextAttributes(Tui::ZTextAttribute::Italic));
    }

    SECTION("mapTabColor") {
        // Default
        Tui::ZTextStyle ts = to->mapTabColor(0, 0, 0, Tui::ZTextStyle({0, 0, 0}, {0, 0, 0}, Tui::ZTextAttribute::Underline),
                                             Tui::ZTextStyle({11, 22, 33}, {44, 55, 66}, Tui::ZTextAttribute::Bold), nullptr);
        CHECK(ts.foregroundColor() == Tui::ZColor{11, 22, 33});
        CHECK(ts.backgroundColor() == Tui::ZColor{44, 55, 66});
        CHECK(ts.attributes() == Tui::ZTextAttributes(Tui::ZTextAttribute::Bold));

        Tui::ZFormatRange fr;
        fr.setFormat(Tui::ZTextStyle({77, 88, 99}, {100, 110, 120}, Tui::ZTextAttribute::Italic));
        ts = to->mapTabColor(0, 0, 0, Tui::ZTextStyle({0, 0, 0}, {0, 0, 0}, Tui::ZTextAttribute::Bold),
                             Tui::ZTextStyle({11, 22, 33}, {44, 55, 66}, Tui::ZTextAttribute::Bold), &fr);
        CHECK(ts.foregroundColor() == Tui::ZColor{77, 88, 99});
        CHECK(ts.backgroundColor() == Tui::ZColor{100, 110, 120});
        CHECK(ts.attributes() == Tui::ZTextAttributes(Tui::ZTextAttribute::Italic));

        // Set
        to->setTabColor([&fr](int index, int tabWith, int noTabCount, const Tui::ZTextStyle& base, const Tui::ZTextStyle& formatingCharColor, const Tui::ZFormatRange* range) {
            CHECK(index == 200);
            CHECK(tabWith == 210);
            CHECK(noTabCount == 220);

            CHECK(base.foregroundColor() == Tui::ZColor{77, 88, 99});
            CHECK(base.backgroundColor() == Tui::ZColor{110, 112, 113});
            CHECK(base.attributes() == Tui::ZTextAttributes(Tui::ZTextAttribute::Inverse));

            CHECK(formatingCharColor.foregroundColor() == Tui::ZColor{11, 22, 33});
            CHECK(formatingCharColor.backgroundColor() == Tui::ZColor{44, 55, 66});
            CHECK(formatingCharColor.attributes() == Tui::ZTextAttributes(Tui::ZTextAttribute::Bold));

            CHECK(&fr == range);
            return Tui::ZTextStyle({140, 150, 160}, {170, 180, 190}, Tui::ZTextAttribute::Overline);
        });

        ts = to->mapTabColor(200, 210, 220, Tui::ZTextStyle({77, 88, 99}, {110, 112, 113}, Tui::ZTextAttribute::Inverse),
                                             Tui::ZTextStyle({11, 22, 33}, {44, 55, 66}, Tui::ZTextAttribute::Bold), &fr);
        CHECK(ts.foregroundColor() == Tui::ZColor{140, 150, 160});
        CHECK(ts.backgroundColor() == Tui::ZColor{170, 180, 190});
        CHECK(ts.attributes() == Tui::ZTextAttributes(Tui::ZTextAttribute::Overline));

        // Reset test
        to->setTabColor({});
        ts = to->mapTabColor(0, 0, 0, Tui::ZTextStyle({0, 0, 0}, {0, 0, 0}, Tui::ZTextAttribute::Underline),
                             Tui::ZTextStyle({11, 22, 33}, {44, 55, 66}, Tui::ZTextAttribute::Bold), nullptr);
        CHECK(ts.foregroundColor() == Tui::ZColor{11, 22, 33});
        CHECK(ts.backgroundColor() == Tui::ZColor{44, 55, 66});
        CHECK(ts.attributes() == Tui::ZTextAttributes(Tui::ZTextAttribute::Bold));

        fr.setFormat(Tui::ZTextStyle({77, 88, 99}, {100, 110, 120}, Tui::ZTextAttribute::Italic));
        ts = to->mapTabColor(0, 0, 0, Tui::ZTextStyle({0, 0, 0}, {0, 0, 0}, Tui::ZTextAttribute::Bold),
                             Tui::ZTextStyle({11, 22, 33}, {44, 55, 66}, Tui::ZTextAttribute::Bold), &fr);
        CHECK(ts.foregroundColor() == Tui::ZColor{77, 88, 99});
        CHECK(ts.backgroundColor() == Tui::ZColor{100, 110, 120});
        CHECK(ts.attributes() == Tui::ZTextAttributes(Tui::ZTextAttribute::Italic));
    }

    SECTION("operator=") {
        Tui::ZTextOption to1;

        to1.setWrapMode(Tui::ZTextOption::WrapMode::NoWrap);
        to1.setFlags(Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors);
        to1.setTabStopDistance(3);

        Tui::ZTextOption to2;

        to2 = to1;

        CHECK(to2.flags() == Tui::ZTextOption::Flags{Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors});
        CHECK(to2.tabStopDistance() == 3);
        CHECK(to2.wrapMode() == Tui::ZTextOption::WrapMode::NoWrap);

        CHECK(to1.flags() == Tui::ZTextOption::Flags{Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors});
        CHECK(to1.tabStopDistance() == 3);
        CHECK(to1.wrapMode() == Tui::ZTextOption::WrapMode::NoWrap);
    }

    SECTION("operator= move") {
        Tui::ZTextOption to1;

        to1.setWrapMode(Tui::ZTextOption::WrapMode::NoWrap);
        to1.setFlags(Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors);
        to1.setTabStopDistance(3);

        Tui::ZTextOption to2;

        to2 = std::move(to1);

        CHECK(to2.flags() == Tui::ZTextOption::Flags{Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors});
        CHECK(to2.tabStopDistance() == 3);
        CHECK(to2.wrapMode() == Tui::ZTextOption::WrapMode::NoWrap);

        CHECK(to1.flags() == Tui::ZTextOption::Flags{});
        CHECK(to1.tabStopDistance() == 8);
        CHECK(to1.wrapMode() == Tui::ZTextOption::WrapMode::WrapAnywhere);
    }

    SECTION("copy constructor") {
        Tui::ZTextOption to1;

        to1.setWrapMode(Tui::ZTextOption::WrapMode::NoWrap);
        to1.setFlags(Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors);
        to1.setTabStopDistance(3);

        Tui::ZTextOption to2 = to1;

        CHECK(to2.flags() == Tui::ZTextOption::Flags{Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors});
        CHECK(to2.tabStopDistance() == 3);
        CHECK(to2.wrapMode() == Tui::ZTextOption::WrapMode::NoWrap);

        CHECK(to1.flags() == Tui::ZTextOption::Flags{Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors});
        CHECK(to1.tabStopDistance() == 3);
        CHECK(to1.wrapMode() == Tui::ZTextOption::WrapMode::NoWrap);
    }

    SECTION("move constructor") {
        Tui::ZTextOption to1;

        to1.setWrapMode(Tui::ZTextOption::WrapMode::NoWrap);
        to1.setFlags(Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors);
        to1.setTabStopDistance(3);

        Tui::ZTextOption to2 = std::move(to1);

        CHECK(to2.flags() == Tui::ZTextOption::Flags{Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors});
        CHECK(to2.tabStopDistance() == 3);
        CHECK(to2.wrapMode() == Tui::ZTextOption::WrapMode::NoWrap);

        CHECK(to1.flags() == Tui::ZTextOption::Flags{});
        CHECK(to1.tabStopDistance() == 8);
        CHECK(to1.wrapMode() == Tui::ZTextOption::WrapMode::WrapAnywhere);
    }
}
