
#include <Tui/ZVBoxLayout.h>

#include "catchwrapper.h"

#include "Testhelper.h"

namespace {

class StabilityChecker {
public:
    void start(Tui::ZWidget *s) {
        before = capture_state(s);
        forceRelayout(s);
        after = capture_state(s);
    }

    QString before;
    QString after;

    QString capture_state(Tui::ZWidget *s) {
        QString result;
        result += dumpWidget(s, 0);
        return result;
    }

    QString dumpWidget(Tui::ZWidget *w, int level) {
        const QString indent = QString(" ").repeated(level);
        QString result;
        QRect g = w->geometry();
        result += indent + QString("Widget %0 {%1, %2, %3, %4} %5\n").arg(QString::number(level),
                                                                QString::number(g.x()),
                                                                QString::number(g.y()),
                                                                QString::number(g.width()),
                                                                QString::number(g.height()),
                                                                     w->objectName());
        for (Tui::ZWidget *child: w->findChildren<Tui::ZWidget*>(QString{}, Qt::FindDirectChildrenOnly)) {
            result += dumpWidget(child, level + 1);
        }
        return result;
    }

    void forceRelayout(Tui::ZWidget *w) {
        QEvent request(QEvent::LayoutRequest);
        QCoreApplication::sendEvent(w, &request);
        for (Tui::ZWidget *child: w->findChildren<Tui::ZWidget*>()) {
            forceRelayout(child);
        }
    }

};

struct SimpleLayout {
    SimpleLayout(Tui::ZWidget *outer) {
        layout = new Tui::ZVBoxLayout();
        w1 = new StubWidget(outer);
        w1->stubSizeHint = {20, 1};
        layout->addWidget(w1);

        w2 = new StubWidget(outer);
        w2->stubSizeHint = {20, 1};
        layout->addWidget(w2);

        w3 = new StubWidget(outer);
        w3->stubSizeHint = {20, 1};
        layout->addWidget(w3);
    }

    Tui::ZVBoxLayout *layout = nullptr;
    StubWidget *w1 = nullptr;
    StubWidget *w2 = nullptr;
    StubWidget *w3 = nullptr;
};

}

TEST_CASE("layoutinvalidation", "") {
    Testhelper t("unused", "unused", 80, 50);
    TestBackground *w = new TestBackground(t.root);
    w->setGeometry({0,0,80,50});

    t.terminal->doLayout(); // reset pending

    SECTION("geometry after terminal") {
        Tui::ZWidget outer;
        SimpleLayout sl(&outer);
        outer.setLayout(sl.layout);

        CHECK(sl.w1->geometry().width() == 0);

        outer.setParent(w);
        t.terminal->doLayout(); // reset pending

        CHECK(sl.w1->geometry().width() == 0);
        CHECK(t.terminal->isLayoutPending() == false);

        outer.setGeometry({1, 1, 78, 48});
        REQUIRE(t.terminal->isLayoutPending() == true);
        t.terminal->doLayout();
        CHECK(sl.w1->geometry().width() == 78);

        StabilityChecker chk;
        chk.start(&outer);
        CHECK(chk.before == chk.after);
    }

    SECTION("setLayout after terminal") {
        Tui::ZWidget outer;
        SimpleLayout sl(&outer);

        CHECK(sl.w1->geometry().width() == 0);

        outer.setGeometry({1, 1, 78, 48});

        outer.setParent(w);

        CHECK(sl.w1->geometry().width() == 0);
        CHECK(t.terminal->isLayoutPending() == false);

        outer.setLayout(sl.layout);

        REQUIRE(t.terminal->isLayoutPending() == true);
        t.terminal->doLayout();
        CHECK(sl.w1->geometry().width() == 78);

        StabilityChecker chk;
        chk.start(&outer);
        CHECK(chk.before == chk.after);
    }

    SECTION("terminal after size") {
        Tui::ZWidget outer;
        SimpleLayout sl(&outer);

        CHECK(sl.w1->geometry().width() == 0);

        outer.setGeometry({1, 1, 78, 48});
        outer.setLayout(sl.layout);

        CHECK(sl.w1->geometry().width() == 0);
        CHECK(t.terminal->isLayoutPending() == false);

        outer.setParent(w);

        REQUIRE(t.terminal->isLayoutPending() == true);
        t.terminal->doLayout();
        CHECK(sl.w1->geometry().width() == 78);

        StabilityChecker chk;
        chk.start(&outer);
        CHECK(chk.before == chk.after);
    }


    SECTION("setVisible after initial layout") {
        Tui::ZWidget outer(w);
        SimpleLayout sl(&outer);

        CHECK(sl.w1->geometry().width() == 0);

        outer.setGeometry({1, 1, 78, 48});

        CHECK(sl.w1->geometry().width() == 0);
        CHECK(t.terminal->isLayoutPending() == false);

        outer.setLayout(sl.layout);

        CHECK(t.terminal->isLayoutPending() == true);
        t.terminal->doLayout();

        sl.w1->setVisible(false);

        REQUIRE(t.terminal->isLayoutPending() == true);
        t.terminal->doLayout();

        CHECK(sl.w2->geometry().y() == 0);

        StabilityChecker chk;
        chk.start(&outer);
        CHECK(chk.before == chk.after);
    }

    SECTION("setVisible in nested layout after initial layout") {
        Tui::ZWidget outer(w);
        outer.setObjectName("outer");

        auto *layout = new Tui::ZVBoxLayout();
        auto *w1 = new Tui::ZWidget(&outer);
        w1->setObjectName("w1");
        layout->addWidget(w1);


        auto *layoutInner = new Tui::ZVBoxLayout();
        auto *w1Inner = new StubWidget(w1);
        w1Inner->setObjectName("w1Inner");
        w1Inner->stubSizeHint = {20, 1};
        layoutInner->addWidget(w1Inner);
        w1->setLayout(layoutInner);
        w1->setSizePolicyV(Tui::SizePolicy::Fixed);

        auto *w2 = new StubWidget(&outer);
        w2->setObjectName("w2");
        w2->stubSizeHint = {20, 1};
        layout->addWidget(w2);

        auto *w3 = new StubWidget(&outer);
        w3->setObjectName("w3");
        w3->stubSizeHint = {20, 1};
        layout->addWidget(w3);

        outer.setGeometry({1, 1, 78, 48});
        outer.setLayout(layout);

        CHECK(t.terminal->isLayoutPending() == true);
        t.terminal->doLayout();

        CHECK(w2->geometry().y() != 0);

        REQUIRE(t.terminal->isLayoutPending() == false);

        w1Inner->setVisible(false);

        REQUIRE(t.terminal->isLayoutPending() == true);
        t.terminal->doLayout();

        //CHECK(w2->geometry().y() == 0);

        StabilityChecker chk;
        chk.start(&outer);
        CHECK(chk.before == chk.after);
    }

}

namespace {
    class LayoutGenWrap : public Tui::ZWidget {
        using Tui::ZWidget::ZWidget;

    public:
        bool event(QEvent *event) override {
            if (event->type() == QEvent::LayoutRequest) {
                gen = term->currentLayoutGeneration();
            }
            return false;
        }

        Tui::ZTerminal *term;
        int gen = 0;
    };
}

TEST_CASE("layout generation mixed") {
    Testhelper t("unused", "unused", 80, 50);

    LayoutGenWrap w(t.root);
    w.term = t.terminal.get();

    int gen;

    auto &term = *t.terminal.get();

    term.requestLayout(&w);
    term.doLayout();
    gen = w.gen;

    REQUIRE(gen == 2);

    gen = t.terminal->currentLayoutGeneration();

    REQUIRE(gen == 4);

    gen = t.terminal->currentLayoutGeneration();

    REQUIRE(gen == 5);

    term.requestLayout(&w);
    term.doLayout();
    gen = w.gen;

    REQUIRE(gen == 6);
}

TEST_CASE("layout generation wraparound brute force with doLayout", "[!hide][layoutgenwrap]") {
    // Runs about 15 minutes with 32bit int
    Testhelper t("unused", "unused", 80, 50);

    LayoutGenWrap w(t.root);
    w.term = t.terminal.get();

    int gen, nextGen;

    auto &term = *t.terminal.get();

    term.requestLayout(&w);
    term.doLayout();
    gen = w.gen;

    CHECK(gen > 0);

    for (int i = 0; i < 1000; i++) {
        term.requestLayout(&w);
        term.doLayout();
        nextGen = w.gen;
        REQUIRE(nextGen > gen);
        gen = nextGen;
    }

    REQUIRE(gen > 100);

    term.requestLayout(&w);
    term.doLayout();
    nextGen = w.gen;

    while (nextGen > 100) {
        if (nextGen <= gen) { // tight loop, so don't use catch when everything is ok.
            REQUIRE(nextGen > gen);
        }
        gen = nextGen;
        term.requestLayout(&w);
        term.doLayout();
        nextGen = w.gen;
    }
    CHECK(nextGen > 0);
}

TEST_CASE("layout generation wraparound brute force no layout", "[!hide][layoutgenwrap]") {
    // Runs fairly fast < 1min
    Testhelper t("unused", "unused", 80, 50);

    int gen, nextGen;
    gen = t.terminal->currentLayoutGeneration();
    CHECK(gen > 0);

    for (int i = 0; i < 1000; i++) {
        nextGen = t.terminal->currentLayoutGeneration();
        REQUIRE(nextGen > gen);
        gen = nextGen;
    }

    REQUIRE(gen > 100);

    nextGen = t.terminal->currentLayoutGeneration();

    while (nextGen > 100) {
        if (nextGen <= gen) { // tight loop, so don't use catch when everything is ok.
            REQUIRE(nextGen > gen);
        }
        gen = nextGen;
        nextGen = t.terminal->currentLayoutGeneration();
    }
    CHECK(nextGen > 0);
}
