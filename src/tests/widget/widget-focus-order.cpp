// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZWidget.h>

#include <limits>

#include <Tui/ZLayout.h>
#include <Tui/ZPalette.h>

#include "../catchwrapper.h"
#include "../Testhelper.h"
#include "../eventrecorder.h"


#define OBJNAME(x) x.setObjectName(#x)

TEST_CASE("widget-focus-order") {
    Testhelper t("unused", "unused", 2, 4);
    t.root->setObjectName("root");
    t.root->setFocusPolicy(Tui::FocusPolicy::NoFocus);
    CHECK(t.root->nextFocusable() == t.root);
    CHECK(t.root->prevFocusable() == t.root);
    CHECK(t.root->placeFocus(false) == nullptr);
    CHECK(t.root->placeFocus(true) == nullptr);

    Tui::ZWidget widget1(t.root);
    OBJNAME(widget1);
    Tui::FocusPolicy focusMode = GENERATE(Tui::FocusPolicy::TabFocus,
                                          Tui::FocusPolicy::WheelFocus,
                                          Tui::FocusPolicy::StrongFocus);
    CAPTURE(focusMode);
    widget1.setFocusPolicy(focusMode);
    CHECK(t.root->nextFocusable() == &widget1);
    CHECK(t.root->prevFocusable() == &widget1);
    CHECK(widget1.nextFocusable() == &widget1);
    CHECK(widget1.prevFocusable() == &widget1);
    CHECK(t.root->placeFocus(false) == &widget1);
    CHECK(t.root->placeFocus(true) == &widget1);

    SECTION("single") {
        CHECK(t.root->nextFocusable() == &widget1);
        CHECK(t.root->prevFocusable() == &widget1);
        CHECK(widget1.nextFocusable() == &widget1);
        CHECK(widget1.prevFocusable() == &widget1);
    }
    SECTION("single-invisible") {
        widget1.setVisible(false);
        CHECK(t.root->nextFocusable() == t.root);
        CHECK(t.root->prevFocusable() == t.root);
        CHECK(widget1.nextFocusable() == &widget1);
        CHECK(widget1.prevFocusable() == &widget1);
        CHECK(t.root->placeFocus(false) == nullptr);
        CHECK(t.root->placeFocus(true) == nullptr);
    }
    SECTION("single-disable") {
        widget1.setEnabled(false);
        CHECK(t.root->nextFocusable() == t.root);
        CHECK(t.root->prevFocusable() == t.root);
        CHECK(widget1.nextFocusable() == &widget1);
        CHECK(widget1.prevFocusable() == &widget1);
        CHECK(t.root->placeFocus(false) == nullptr);
        CHECK(t.root->placeFocus(true) == nullptr);
    }

    Tui::ZWidget widget2(t.root);
    OBJNAME(widget2);
    widget2.setFocusPolicy(Tui::FocusPolicy::TabFocus);
    SECTION("dual") {
        CHECK(widget1.nextFocusable() == &widget2);
        CHECK(widget1.prevFocusable() == &widget2);
        CHECK(widget2.nextFocusable() == &widget1);
        CHECK(widget2.prevFocusable() == &widget1);
        CHECK(t.root->placeFocus(false) == &widget1);
        CHECK(t.root->placeFocus(true) == &widget2);
        widget1.setEnabled(false);
        CHECK(widget1.nextFocusable() == &widget2);
        CHECK(widget1.prevFocusable() == &widget2);
        CHECK(widget2.nextFocusable() == &widget2);
        CHECK(widget2.prevFocusable() == &widget2);
        CHECK(t.root->placeFocus(false) == &widget2);
        CHECK(t.root->placeFocus(true) == &widget2);
        widget2.setEnabled(false);
        CHECK(widget1.nextFocusable() == &widget1);
        CHECK(widget1.prevFocusable() == &widget1);
        CHECK(widget2.nextFocusable() == &widget2);
        CHECK(widget2.prevFocusable() == &widget2);
        CHECK(t.root->placeFocus(false) == nullptr);
        CHECK(t.root->placeFocus(true) == nullptr);
    }

    Tui::ZWidget widget3(t.root);
    OBJNAME(widget3);
    widget3.setFocusPolicy(Tui::FocusPolicy::WheelFocus);
    SECTION("triple") {
        CHECK(widget1.nextFocusable() == &widget2);
        CHECK(widget1.prevFocusable() == &widget3);
        CHECK(widget2.nextFocusable() == &widget3);
        CHECK(widget2.prevFocusable() == &widget1);
        CHECK(widget3.nextFocusable() == &widget1);
        CHECK(widget3.prevFocusable() == &widget2);
        CHECK(t.root->placeFocus(false) == &widget1);
        CHECK(t.root->placeFocus(true) == &widget3);
        widget1.setEnabled(false);
        CHECK(widget1.nextFocusable() == &widget2);
        CHECK(widget1.prevFocusable() == &widget3);
        CHECK(widget2.nextFocusable() == &widget3);
        CHECK(widget2.prevFocusable() == &widget3);
        CHECK(widget3.nextFocusable() == &widget2);
        CHECK(widget3.prevFocusable() == &widget2);
        CHECK(t.root->placeFocus(false) == &widget2);
        CHECK(t.root->placeFocus(true) == &widget3);
        widget1.setEnabled(true);
        widget2.setVisible(false);
        CHECK(widget1.nextFocusable() == &widget3);
        CHECK(widget1.prevFocusable() == &widget3);
        CHECK(widget2.nextFocusable() == &widget3);
        CHECK(widget2.prevFocusable() == &widget1);
        CHECK(widget3.nextFocusable() == &widget1);
        CHECK(widget3.prevFocusable() == &widget1);
        CHECK(t.root->placeFocus(false) == &widget1);
        CHECK(t.root->placeFocus(true) == &widget3);
        widget2.setVisible(true);
        widget3.setEnabled(false);
        widget3.setVisible(false);
        CHECK(widget1.nextFocusable() == &widget2);
        CHECK(widget1.prevFocusable() == &widget2);
        CHECK(widget2.nextFocusable() == &widget1);
        CHECK(widget2.prevFocusable() == &widget1);
        CHECK(widget3.nextFocusable() == &widget1);
        CHECK(widget3.prevFocusable() == &widget2);
        CHECK(t.root->placeFocus(false) == &widget1);
        CHECK(t.root->placeFocus(true) == &widget2);
    }
}

TEST_CASE("widget-focus-order-tree") {
    Testhelper t("unused", "unused", 2, 4);
    t.root->setObjectName("root");
    t.root->setFocusPolicy(Tui::FocusPolicy::NoFocus);
    CHECK(t.root->nextFocusable() == t.root);
    CHECK(t.root->prevFocusable() == t.root);
    CHECK(t.root->placeFocus(false) == nullptr);
    CHECK(t.root->placeFocus(true) == nullptr);

    Tui::ZWidget widget1(t.root);
    OBJNAME(widget1);
    Tui::FocusPolicy focusMode = GENERATE(Tui::FocusPolicy::TabFocus,
                                          Tui::FocusPolicy::WheelFocus,
                                          Tui::FocusPolicy::StrongFocus);
    CAPTURE(focusMode);
    widget1.setFocusPolicy(focusMode);

    Tui::ZWidget widget2(t.root);
    OBJNAME(widget2);
    widget2.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    Tui::ZWidget widget1Child(&widget1);
    OBJNAME(widget1Child);
    widget1Child.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    Tui::ZWidget childChild(&widget1Child);
    OBJNAME(childChild);
    childChild.setFocusPolicy(Tui::FocusPolicy::StrongFocus);

    CHECK(t.root->placeFocus(false) == &widget1);
    CHECK(t.root->placeFocus(true) == &widget2);

    CHECK(widget1.nextFocusable() == &widget1Child);
    CHECK(widget1.prevFocusable() == &widget2);
    CHECK(widget1.placeFocus(false) == &widget1);
    CHECK(widget1.placeFocus(true) == &childChild);

    CHECK(widget1Child.nextFocusable() == &childChild);
    CHECK(widget1Child.prevFocusable() == &widget1);
    CHECK(widget1Child.placeFocus(false) == &widget1Child);
    CHECK(widget1Child.placeFocus(true) == &childChild);

    CHECK(childChild.nextFocusable() == &widget2);
    CHECK(childChild.prevFocusable() == &widget1Child);
    CHECK(childChild.placeFocus(false) == &childChild);
    CHECK(childChild.placeFocus(true) == &childChild);

    CHECK(widget2.nextFocusable() == &widget1);
    CHECK(widget2.prevFocusable() == &childChild);
    CHECK(widget2.placeFocus(false) == &widget2);
    CHECK(widget2.placeFocus(true) == &widget2);

    Tui::ZWidget widget2Child(&widget2);
    OBJNAME(widget2Child);
    widget2Child.setFocusPolicy(Tui::FocusPolicy::StrongFocus);

    CHECK(t.root->placeFocus(false) == &widget1);
    CHECK(t.root->placeFocus(true) == &widget2Child);

    CHECK(widget1.nextFocusable() == &widget1Child);
    CHECK(widget1.prevFocusable() == &widget2Child);
    CHECK(widget1.placeFocus(false) == &widget1);
    CHECK(widget1.placeFocus(true) == &childChild);

    CHECK(widget1Child.nextFocusable() == &childChild);
    CHECK(widget1Child.prevFocusable() == &widget1);
    CHECK(widget1Child.placeFocus(false) == &widget1Child);
    CHECK(widget1Child.placeFocus(true) == &childChild);

    CHECK(childChild.nextFocusable() == &widget2);
    CHECK(childChild.prevFocusable() == &widget1Child);
    CHECK(childChild.placeFocus(false) == &childChild);
    CHECK(childChild.placeFocus(true) == &childChild);

    CHECK(widget2.nextFocusable() == &widget2Child);
    CHECK(widget2.prevFocusable() == &childChild);
    CHECK(widget2.placeFocus(false) == &widget2);
    CHECK(widget2.placeFocus(true) == &widget2Child);

    CHECK(widget2Child.nextFocusable() == &widget1);
    CHECK(widget2Child.prevFocusable() == &widget2);
    CHECK(widget2Child.placeFocus(false) == &widget2Child);
    CHECK(widget2Child.placeFocus(true) == &widget2Child);
}

TEST_CASE("widget-focus-order-tree-inner") {
    Testhelper t("unused", "unused", 2, 4);
    t.root->setObjectName("root");
    t.root->setFocusPolicy(Tui::FocusPolicy::NoFocus);
    CHECK(t.root->nextFocusable() == t.root);
    CHECK(t.root->prevFocusable() == t.root);
    CHECK(t.root->placeFocus(false) == nullptr);
    CHECK(t.root->placeFocus(true) == nullptr);

    Tui::ZWidget widget1(t.root);
    OBJNAME(widget1);

    Tui::ZWidget widget2(t.root);
    OBJNAME(widget2);
    Tui::ZWidget widget1Child(&widget1);
    OBJNAME(widget1Child);
    Tui::ZWidget childChild(&widget1Child);
    OBJNAME(childChild);
    childChild.setFocusPolicy(Tui::FocusPolicy::StrongFocus);

    Tui::ZWidget widget2Child(&widget2);
    OBJNAME(widget2Child);
    widget2Child.setFocusPolicy(Tui::FocusPolicy::StrongFocus);

    CHECK(t.root->placeFocus(false) == &childChild);
    CHECK(t.root->placeFocus(true) == &widget2Child);

    CHECK(widget1.nextFocusable() == &childChild);
    CHECK(widget1.prevFocusable() == &widget2Child);
    CHECK(widget1.placeFocus(false) == &childChild);
    CHECK(widget1.placeFocus(true) == &childChild);

    CHECK(widget1Child.nextFocusable() == &childChild);
    CHECK(widget1Child.prevFocusable() == &widget2Child);
    CHECK(widget1Child.placeFocus(false) == &childChild);
    CHECK(widget1Child.placeFocus(true) == &childChild);

    CHECK(childChild.nextFocusable() == &widget2Child);
    CHECK(childChild.prevFocusable() == &widget2Child);
    CHECK(childChild.placeFocus(false) == &childChild);
    CHECK(childChild.placeFocus(true) == &childChild);

    CHECK(widget2.nextFocusable() == &widget2Child);
    CHECK(widget2.prevFocusable() == &childChild);
    CHECK(widget2.placeFocus(false) == &widget2Child);
    CHECK(widget2.placeFocus(true) == &widget2Child);

    CHECK(widget2Child.nextFocusable() == &childChild);
    CHECK(widget2Child.prevFocusable() == &childChild);
    CHECK(widget2Child.placeFocus(false) == &widget2Child);
    CHECK(widget2Child.placeFocus(true) == &widget2Child);
}

TEST_CASE("widget-FocusContainerMode-Cycle") {
    Testhelper t("unused", "unused", 2, 4);
    t.root->setObjectName("root");
    t.root->setFocusPolicy(Tui::FocusPolicy::NoFocus);
    bool bf = GENERATE(true, false);

    Tui::ZWidget top1(t.root);
    OBJNAME(top1);
    CHECK(top1.focusPolicy() == Tui::FocusPolicy::NoFocus);
    top1.setFocusMode(Tui::FocusContainerMode::Cycle);
    CHECK(top1.focusMode() == Tui::FocusContainerMode::Cycle);

    Tui::ZWidget widget1(&top1);
    OBJNAME(widget1);
    widget1.setFocusPolicy(Tui::FocusPolicy::StrongFocus);

    Tui::ZWidget widget2(&top1);
    OBJNAME(widget2);
    widget2.setFocusPolicy(Tui::FocusPolicy::StrongFocus);

    CHECK(widget1.placeFocus(bf) == &widget1);
    CHECK(widget2.placeFocus(bf) == &widget2);

    CHECK(top1.placeFocus() == &widget1);
    CHECK(top1.placeFocus(true) == &widget2);

    Tui::ZWidget top2(t.root);
    OBJNAME(top2);
    top2.setFocusMode(Tui::FocusContainerMode::Cycle);
    CHECK(top2.focusMode() == Tui::FocusContainerMode::Cycle);

    Tui::ZWidget widget3(&top2);
    OBJNAME(widget3);
    widget3.setFocusPolicy(Tui::FocusPolicy::StrongFocus);

    Tui::ZWidget widget4(&top2);
    OBJNAME(widget4);
    widget4.setFocusPolicy(Tui::FocusPolicy::StrongFocus);

    CHECK(widget1.placeFocus(bf) == &widget1);
    CHECK(widget2.placeFocus(bf) == &widget2);
    CHECK(widget3.placeFocus(bf) == &widget3);
    CHECK(widget4.placeFocus(bf) == &widget4);

    CHECK(widget1.nextFocusable() == &widget2);
    CHECK(widget1.prevFocusable() == &widget2);
    CHECK(widget2.nextFocusable() == &widget1);
    CHECK(widget2.prevFocusable() == &widget1);

    CHECK(widget3.nextFocusable() == &widget4);
    CHECK(widget3.prevFocusable() == &widget4);
    CHECK(widget4.nextFocusable() == &widget3);
    CHECK(widget4.prevFocusable() == &widget3);

    SECTION("disable-3") {
        bool ev = GENERATE(true, false);
        CAPTURE(ev);
        if (ev) {
            widget3.setEnabled(false);
        } else {
            widget3.setVisible(false);
        }
        CHECK(widget1.placeFocus(bf) == &widget1);
        CHECK(widget2.placeFocus(bf) == &widget2);
        CHECK(widget3.placeFocus(bf) == nullptr);
        CHECK(widget4.placeFocus(bf) == &widget4);
    }

}

TEST_CASE("widget-FocusContainerMode-Cycle-SubOrdering") {
    Testhelper t("unused", "unused", 2, 4);
    t.root->setObjectName("root");
    t.root->setFocusPolicy(Tui::FocusPolicy::NoFocus);
    bool bf = GENERATE(true, false);

    Tui::ZWidget top1(t.root);
    OBJNAME(top1);
    top1.setFocusPolicy(Tui::FocusPolicy::NoFocus);
    top1.setFocusMode(Tui::FocusContainerMode::Cycle);
    CHECK(top1.focusMode() == Tui::FocusContainerMode::Cycle);

    Tui::ZWidget widget1(&top1);
    OBJNAME(widget1);
    widget1.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    CHECK(widget1.placeFocus(bf) == &widget1);

    Tui::ZWidget widget2(&top1);
    OBJNAME(widget2);
    widget2.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    CHECK(widget1.placeFocus(bf) == &widget1);
    CHECK(widget2.placeFocus(bf) == &widget2);

    Tui::ZWidget top2(t.root);
    OBJNAME(top2);
    top2.setFocusPolicy(Tui::FocusPolicy::NoFocus);
    top2.setFocusMode(Tui::FocusContainerMode::SubOrdering);
    CHECK(top2.focusMode() == Tui::FocusContainerMode::SubOrdering);

    Tui::ZWidget widget3(&top2);
    OBJNAME(widget3);
    widget3.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    CHECK(widget1.placeFocus(bf) == &widget1);
    CHECK(widget2.placeFocus(bf) == &widget2);
    CHECK(widget3.placeFocus(bf) == &widget3);

    Tui::ZWidget widget4(&top2);
    OBJNAME(widget4);
    widget4.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    CHECK(widget1.placeFocus(bf) == &widget1);
    CHECK(widget2.placeFocus(bf) == &widget2);
    CHECK(widget3.placeFocus(bf) == &widget3);
    CHECK(widget4.placeFocus(bf) == &widget4);


    CHECK(widget1.nextFocusable() == &widget2);
    CHECK(widget1.prevFocusable() == &widget2);
    CHECK(widget2.nextFocusable() == &widget1);
    CHECK(widget2.prevFocusable() == &widget1);

    CHECK(widget3.nextFocusable() == &widget4);
    CHECK(widget3.prevFocusable() == &widget2);
    CHECK(widget4.nextFocusable() == &widget1);
    CHECK(widget4.prevFocusable() == &widget3);
}

TEST_CASE("widget-FocusContainerMode-Cycle-none") {
    Testhelper t("unused", "unused", 2, 4);
    t.root->setObjectName("root");
    t.root->setFocusPolicy(Tui::FocusPolicy::NoFocus);
    bool bf = GENERATE(true, false);

    Tui::ZWidget top1(t.root);
    OBJNAME(top1);
    top1.setFocusPolicy(Tui::FocusPolicy::NoFocus);
    top1.setFocusMode(Tui::FocusContainerMode::Cycle);
    CHECK(top1.focusMode() == Tui::FocusContainerMode::Cycle);

    Tui::ZWidget widget1(&top1);
    OBJNAME(widget1);
    widget1.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    CHECK(widget1.placeFocus(bf) == &widget1);

    Tui::ZWidget widget2(&top1);
    OBJNAME(widget2);
    widget2.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    CHECK(widget1.placeFocus(bf) == &widget1);
    CHECK(widget2.placeFocus(bf) == &widget2);

    Tui::ZWidget top2(t.root);
    OBJNAME(top2);
    top2.setFocusPolicy(Tui::FocusPolicy::NoFocus);
    CHECK(top2.focusMode() == Tui::FocusContainerMode::None);

    Tui::ZWidget widget3(&top2);
    OBJNAME(widget3);
    widget3.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    CHECK(widget1.placeFocus(bf) == &widget1);
    CHECK(widget2.placeFocus(bf) == &widget2);
    CHECK(widget3.placeFocus(bf) == &widget3);


    Tui::ZWidget widget4(&top2);
    OBJNAME(widget4);
    widget4.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    CHECK(widget1.placeFocus(bf) == &widget1);
    CHECK(widget2.placeFocus(bf) == &widget2);
    CHECK(widget3.placeFocus(bf) == &widget3);
    CHECK(widget4.placeFocus(bf) == &widget4);

    CHECK(widget1.nextFocusable() == &widget2);
    CHECK(widget1.prevFocusable() == &widget2);
    CHECK(widget2.nextFocusable() == &widget1);
    CHECK(widget2.prevFocusable() == &widget1);

    CHECK(widget3.nextFocusable() == &widget4);
    CHECK(widget3.prevFocusable() == &widget2);
    CHECK(widget4.nextFocusable() == &widget1);
    CHECK(widget4.prevFocusable() == &widget3);
}

TEST_CASE("widget-tab-order") {
    Testhelper t("unused", "unused", 2, 4);
    t.root->setObjectName("root");
    t.root->setFocusPolicy(Tui::FocusPolicy::NoFocus);
    bool bf = GENERATE(true, false);

    Tui::ZWidget top1(t.root);
    OBJNAME(top1);
    top1.setFocusPolicy(Tui::FocusPolicy::NoFocus);
    top1.setFocusMode(Tui::FocusContainerMode::Cycle);
    CHECK(top1.focusMode() == Tui::FocusContainerMode::Cycle);

    Tui::ZWidget widget1(&top1);
    OBJNAME(widget1);
    widget1.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    widget1.setFocusOrder(3);
    CHECK(widget1.placeFocus(bf) == &widget1);

    Tui::ZWidget widget2(&top1);
    OBJNAME(widget2);
    widget2.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    widget2.setFocusOrder(1);
    CHECK(widget1.placeFocus(bf) == &widget1);
    CHECK(widget2.placeFocus(bf) == &widget2);

    Tui::ZWidget top2(t.root);
    OBJNAME(top2);
    top2.setFocusMode(Tui::FocusContainerMode::Cycle);
    CHECK(top2.focusMode() == Tui::FocusContainerMode::Cycle);

    Tui::ZWidget widget3(&top2);
    OBJNAME(widget3);
    widget3.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    widget3.setFocusOrder(4);
    CHECK(widget1.placeFocus(bf) == &widget1);
    CHECK(widget2.placeFocus(bf) == &widget2);
    CHECK(widget3.placeFocus(bf) == &widget3);

    Tui::ZWidget widget4(&top2);
    OBJNAME(widget4);
    widget4.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    widget4.setFocusOrder(2);
    CHECK(widget1.placeFocus(bf) == &widget1);
    CHECK(widget2.placeFocus(bf) == &widget2);
    CHECK(widget3.placeFocus(bf) == &widget3);
    CHECK(widget4.placeFocus(bf) == &widget4);

    CHECK(widget1.nextFocusable() == &widget2);
    CHECK(widget1.prevFocusable() == &widget2);
    CHECK(widget2.nextFocusable() == &widget1);
    CHECK(widget2.prevFocusable() == &widget1);

    CHECK(widget3.nextFocusable() == &widget4);
    CHECK(widget3.prevFocusable() == &widget4);
    CHECK(widget4.nextFocusable() == &widget3);
    CHECK(widget4.prevFocusable() == &widget3);
}

TEST_CASE("widget-tab-SubOrdering") {
    Testhelper t("unused", "unused", 2, 4);
    t.root->setFocusPolicy(Tui::FocusPolicy::NoFocus);
    t.root->setObjectName("root");

    Tui::ZWidget top1(t.root);
    OBJNAME(top1);
    top1.setFocusPolicy(Tui::FocusPolicy::NoFocus);
    top1.setFocusMode(Tui::FocusContainerMode::SubOrdering);
    CHECK(top1.focusMode() == Tui::FocusContainerMode::SubOrdering);

    Tui::ZWidget widget1(&top1);
    OBJNAME(widget1);
    widget1.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    widget1.setFocusOrder(3);

    Tui::ZWidget widget2(&top1);
    OBJNAME(widget2);
    widget2.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    widget2.setFocusOrder(1);

    Tui::ZWidget top2(t.root);
    OBJNAME(top2);
    top2.setFocusMode(Tui::FocusContainerMode::SubOrdering);
    CHECK(top2.focusMode() == Tui::FocusContainerMode::SubOrdering);

    Tui::ZWidget widget3(&top2);
    OBJNAME(widget3);
    widget3.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    widget3.setFocusOrder(4);

    Tui::ZWidget widget4(&top2);
    OBJNAME(widget4);
    widget4.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    widget4.setFocusOrder(2);

    CHECK(t.root->placeFocus() == &widget2);
    CHECK(t.root->placeFocus(true) == &widget3);

    CHECK(t.root->nextFocusable() == &widget2);

    CHECK(widget2.nextFocusable() == &widget1);
    CHECK(widget1.nextFocusable() == &widget4);
    CHECK(widget4.nextFocusable() == &widget3);
    CHECK(widget3.nextFocusable() == &widget2);

    CHECK(widget2.prevFocusable() == &widget3);
    CHECK(widget3.prevFocusable() == &widget4);
    CHECK(widget4.prevFocusable() == &widget1);
    CHECK(widget1.prevFocusable() == &widget2);
}

TEST_CASE("widget-tab-onegroup") {
    Testhelper t("unused", "unused", 2, 4);
    t.root->setFocusPolicy(Tui::FocusPolicy::NoFocus);
    t.root->setObjectName("root");
    bool bf = GENERATE(true, false);

    Tui::ZWidget top1(t.root);
    OBJNAME(top1);
    top1.setFocusPolicy(Tui::FocusPolicy::NoFocus);

    bool subordering = GENERATE(true, false);
    CAPTURE(subordering);
    if (subordering) {
        top1.setFocusMode(Tui::FocusContainerMode::SubOrdering);
        CHECK(top1.focusMode() == Tui::FocusContainerMode::SubOrdering);
    } else {
        CHECK(top1.focusMode() == Tui::FocusContainerMode::None);
    }

    Tui::ZWidget widget1(&top1);
    OBJNAME(widget1);
    widget1.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    widget1.setFocusOrder(3);
    CHECK(widget1.placeFocus(bf) == &widget1);

    Tui::ZWidget widget2(&top1);
    OBJNAME(widget2);
    widget2.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    widget2.setFocusOrder(1);
    CHECK(widget1.placeFocus(bf) == &widget1);
    CHECK(widget2.placeFocus(bf) == &widget2);

    Tui::ZWidget widget3(&top1);
    OBJNAME(widget3);
    widget3.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    widget3.setFocusOrder(4);
    CHECK(widget1.placeFocus(bf) == &widget1);
    CHECK(widget2.placeFocus(bf) == &widget2);
    CHECK(widget3.placeFocus(bf) == &widget3);

    Tui::ZWidget widget4(&top1);
    OBJNAME(widget4);
    widget4.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    widget4.setFocusOrder(2);
    CHECK(widget1.placeFocus(bf) == &widget1);
    CHECK(widget2.placeFocus(bf) == &widget2);
    CHECK(widget3.placeFocus(bf) == &widget3);
    CHECK(widget4.placeFocus(bf) == &widget4);

    CHECK(t.root->nextFocusable() == &widget2);

    CHECK(widget2.nextFocusable() == &widget4);
    CHECK(widget4.nextFocusable() == &widget1);
    CHECK(widget1.nextFocusable() == &widget3);
    CHECK(widget3.nextFocusable() == &widget2);

    CHECK(widget2.prevFocusable() == &widget3);
    CHECK(widget3.prevFocusable() == &widget1);
    CHECK(widget1.prevFocusable() == &widget4);
    CHECK(widget4.prevFocusable() == &widget2);
}

TEST_CASE("widget-tab-subordering-order") {
    Testhelper t("unused", "unused", 2, 4);
    t.root->setObjectName("root");

    Tui::ZWidget top1(t.root);
    OBJNAME(top1);
    top1.setFocusMode(Tui::FocusContainerMode::SubOrdering);
    top1.setFocusOrder(999);

    Tui::ZWidget widget11(&top1);
    OBJNAME(widget11);
    widget11.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    widget11.setFocusOrder(100);

    Tui::ZWidget widget12(&top1);
    OBJNAME(widget12);
    widget12.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    widget12.setFocusOrder(99);

    Tui::ZWidget widget13(&top1);
    OBJNAME(widget13);
    widget13.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    widget13.setFocusOrder(101);

    Tui::ZWidget top2(t.root);
    OBJNAME(top2);
    top2.setFocusMode(Tui::FocusContainerMode::SubOrdering);
    top2.setFocusOrder(999);

    Tui::ZWidget widget21(&top2);
    OBJNAME(widget21);
    widget21.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    widget21.setFocusOrder(101);

    Tui::ZWidget widget22(&top2);
    OBJNAME(widget22);
    widget22.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    widget22.setFocusOrder(100);

    Tui::ZWidget widget23(&top2);
    OBJNAME(widget23);
    widget23.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    widget23.setFocusOrder(99);

    Tui::ZWidget top3(t.root);
    OBJNAME(top3);
    top3.setFocusMode(Tui::FocusContainerMode::SubOrdering);
    top3.setFocusOrder(998);

    Tui::ZWidget widget31(&top3);
    OBJNAME(widget31);
    widget31.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    widget31.setFocusOrder(99);

    Tui::ZWidget widget32(&top3);
    OBJNAME(widget32);
    widget32.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    widget32.setFocusOrder(100);

    Tui::ZWidget widget33(&top3);
    OBJNAME(widget33);
    widget33.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    widget33.setFocusOrder(101);

    CHECK(t.root->placeFocus() == &widget31);

    QSet<Tui::ZWidget*> seen;
    Tui::ZWidget *current = &widget31;
    QStringList focusOrder;
    while (!seen.contains(current)) {
        seen.insert(current);
        focusOrder.append(current->objectName());
        current = current->nextFocusable();
    }
    CHECK(focusOrder == QStringList{"widget31", "widget32", "widget33",
                                    "widget12", "widget11", "widget13",
                                    "widget23", "widget22", "widget21"});

    CHECK(t.root->placeFocus(true) == &widget21);

    seen.clear();
    current = &widget21;
    focusOrder.clear();
    while (!seen.contains(current)) {
        seen.insert(current);
        focusOrder.append(current->objectName());
        current = current->prevFocusable();
    }
    CHECK(focusOrder == QStringList{"widget21", "widget22", "widget23",
                                    "widget13", "widget11", "widget12",
                                    "widget33", "widget32", "widget31"});
}
