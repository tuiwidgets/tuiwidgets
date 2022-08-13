// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZLayoutItem.h>

#include "../catchwrapper.h"


TEST_CASE("layoutitem-base") {
    auto w = std::make_unique<Tui::ZWidget>();

    std::unique_ptr<Tui::ZLayoutItem> li = Tui::ZLayoutItem::wrapWidget(w.get());

    SECTION("setGeometry") {
        li->setGeometry({2, 4, 5, 10});
        CHECK(w->geometry() == QRect{2, 4, 5, 10});
        li->setGeometry({6, 3, 8, 12});
        CHECK(w->geometry() == QRect{6, 3, 8, 12});
    }

    SECTION("sizeHint") {
        CHECK(li->sizeHint() == QSize());
        w->setMinimumSize({4, 8});
        CHECK(li->sizeHint() == QSize{4, 8});
    }

    SECTION("sizePolicyH") {
        CHECK(li->sizePolicyH() == Tui::SizePolicy::Preferred);
        w->setSizePolicyH(Tui::SizePolicy::Fixed);
        CHECK(li->sizePolicyH() == Tui::SizePolicy::Fixed);
    }

    SECTION("sizePolicyV") {
        CHECK(li->sizePolicyV() == Tui::SizePolicy::Preferred);
        w->setSizePolicyV(Tui::SizePolicy::Fixed);
        CHECK(li->sizePolicyV() == Tui::SizePolicy::Fixed);
    }

    SECTION("isVisible") {
        CHECK(li->isVisible() == true);
        w->setVisible(false);
        CHECK(li->isVisible() == false);
    }

    SECTION("isSpacer") {
        CHECK(li->isSpacer() == false);
    }

    w = nullptr;

    SECTION("setGeometry-null-widget") {
        // Must not crash
        li->setGeometry({1, 2, 3, 4});
    }

    SECTION("sizeHint-null-widget") {
        CHECK(li->sizeHint() == QSize{});
    }

    SECTION("sizePolicyH-null-widget") {
        CHECK(li->sizePolicyH() == Tui::SizePolicy::Fixed);
    }

    SECTION("sizePolicyV-null-widget") {
        CHECK(li->sizePolicyV() == Tui::SizePolicy::Fixed);
    }

    SECTION("isVisible-null-widget") {
        CHECK(li->isVisible() == false);
    }

    SECTION("isSpacer-null-widget") {
        CHECK(li->isSpacer() == false);
    }

}
