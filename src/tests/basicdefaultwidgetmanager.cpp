// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZBasicDefaultWidgetManager.h>

#include "catchwrapper.h"

#include "Testhelper.h"
#include "vcheck_zdefaultwindowmanager.h"

namespace {
class StubEnterAcceptWidget : public Tui::ZWidget {
public:
    using Tui::ZWidget::ZWidget;

    bool event(QEvent *event) override {
        if (event->type() == Tui::ZEventType::queryAcceptsEnter()) {
            if (accepting) {
                event->accept();
            }
        }
        return Tui::ZWidget::event(event);
    }

    bool accepting = false;
};

class BaseWrapper : public Tui::ZDefaultWidgetManager {
public:
    void setDefaultWidget(Tui::ZWidget*) override {};
    Tui::ZWidget *defaultWidget() const override { return nullptr; }
    bool isDefaultWidgetActive() const override { return false; }
};

}

TEST_CASE("dialogdefaultwidgetmananger-no-terminal", "") {
    Tui::ZWidget w;

    Tui::ZBasicDefaultWidgetManager manager{&w};

    SECTION("abi-vcheck") {
        BaseWrapper base;
        checkZDefaultWidgetManagerOverrides(&base, &manager);
    }

    SECTION("empty") {
        CHECK(manager.defaultWidget() == nullptr);
        CHECK(manager.isDefaultWidgetActive() == false);
    }

    SECTION("defaultWidget") {
        Tui::ZWidget *dflt = new Tui::ZWidget(&w);
        manager.setDefaultWidget(dflt);
        CHECK(manager.defaultWidget() == dflt);
        CHECK(manager.isDefaultWidgetActive() == true);
    }
}


TEST_CASE("dialogdefaultwidgetmananger", "") {
    Testhelper t("unused", "unused", 15, 5);
    Tui::ZWidget *w = new Tui::ZWidget(t.root);


    Tui::ZBasicDefaultWidgetManager manager{w};

    SECTION("empty") {
        CHECK(manager.defaultWidget() == nullptr);
        CHECK(manager.isDefaultWidgetActive() == false);
    }

    SECTION("defaultWidget") {
        Tui::ZWidget *dflt = new Tui::ZWidget(w);
        manager.setDefaultWidget(dflt);
        CHECK(manager.defaultWidget() == dflt);
        CHECK(manager.isDefaultWidgetActive() == true);
        w->setFocus();
        CHECK(manager.defaultWidget() == dflt);
        CHECK(manager.isDefaultWidgetActive() == true);
        dflt->setFocus();
        CHECK(manager.defaultWidget() == dflt);
        CHECK(manager.isDefaultWidgetActive() == true);
    }

    SECTION("accepting enter") {
        StubEnterAcceptWidget w1{w};
        StubEnterAcceptWidget w2{&w1};
        StubEnterAcceptWidget w3{&w2};
        StubEnterAcceptWidget s1{w};


        Tui::ZWidget *dflt = new Tui::ZWidget(w);
        manager.setDefaultWidget(dflt);

        SECTION("dialog focused") {
            w->setFocus();
            CHECK(manager.isDefaultWidgetActive() == true);
        }

        SECTION("nested inner none accepts") {
            w3.setFocus();
            CHECK(manager.isDefaultWidgetActive() == true);
        }

        SECTION("nested inner accepts") {
            w3.setFocus();
            w3.accepting = true;
            CHECK(manager.isDefaultWidgetActive() == false);
        }

        SECTION("nested middle accepts") {
            w3.setFocus();
            w2.accepting = true;
            CHECK(manager.isDefaultWidgetActive() == false);
        }

        SECTION("nested outer accepts") {
            w3.setFocus();
            w1.accepting = true;
            CHECK(manager.isDefaultWidgetActive() == false);
        }

        SECTION("nested sibling accepts") {
            w3.setFocus();
            s1.accepting = true;
            CHECK(manager.isDefaultWidgetActive() == true);
        }

        SECTION("non nested widget does not accept") {
            s1.setFocus();
            CHECK(manager.isDefaultWidgetActive() == true);
        }

        SECTION("non nested widget accepts") {
            s1.setFocus();
            s1.accepting = true;
            CHECK(manager.isDefaultWidgetActive() == false);
        }
    }

    SECTION("nested in default widget") {
        Tui::ZWidget *dflt = new Tui::ZWidget(w);
        manager.setDefaultWidget(dflt);

        StubEnterAcceptWidget w1{dflt};

        SECTION("nested does not accept") {
            w1.setFocus();
            CHECK(manager.isDefaultWidgetActive() == true);
        }

        SECTION("nested does accept") {
            w1.setFocus();
            w1.accepting = true;
            CHECK(manager.isDefaultWidgetActive() == true);
        }
    }

    SECTION("focus outside of window") {
        Tui::ZWidget *dflt = new Tui::ZWidget(w);
        manager.setDefaultWidget(dflt);

        Tui::ZWidget otherWindow{t.root};
        StubEnterAcceptWidget w1{&otherWindow};

        w1.setFocus();

        SECTION("accepts") {
            w1.accepting = true;
            CHECK(manager.isDefaultWidgetActive() == true);
        }

        SECTION("does not accept") {
            w1.accepting = false;
            CHECK(manager.isDefaultWidgetActive() == true);
        }
    }

}
