// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZDialog.h>

#include "../catchwrapper.h"

#include <QTimer>

#include <Tui/ZHBoxLayout.h>
#include <Tui/ZLabel.h>
#include <Tui/ZTextLine.h>
#include <Tui/ZBasicDefaultWidgetManager.h>

#include "../Testhelper.h"
#include "../signalrecorder.h"

namespace {
class EventCatcher : public Tui::ZWidget {
public:
    using Tui::ZWidget::ZWidget;

public:
    void keyEvent(Tui::ZKeyEvent *event) override {
        queue.append({event->key(), event->modifiers()});
    }

    struct QueueItem {
        int key;
        Tui::KeyboardModifiers modifiers;
    };

    QList<QueueItem> queue;
};
}


TEST_CASE("dialog-base", "") {
    // Older Qt (e.g. 5.9) compains when timers are created without this
    Testhelper t("unused", "unused", 14, 5);

    bool parent = GENERATE(false, true);
    CAPTURE(parent);

    std::unique_ptr<Tui::ZWidget> w = parent ? std::make_unique<Tui::ZWidget>() : nullptr;


    // The initialisation must not crash.
    SECTION("constructor") {
        delete new Tui::ZDialog(w.get());
    }

    std::unique_ptr<Tui::ZDialog> dlg = std::make_unique<Tui::ZDialog>(w.get());

    SECTION("defaults") {
        CHECK(dlg->focusMode() == Tui::FocusContainerMode::Cycle);
        CHECK(dlg->paletteClass() == QStringList({QString("window"), QString("dialog")}));
        CHECK(dlg->sizePolicyH() == Tui::SizePolicy::Expanding);
        CHECK(dlg->sizePolicyV() == Tui::SizePolicy::Expanding);
        CHECK(dlg->stackingLayer() == 1000);

        FAIL_CHECK_VEC(checkWidgetsDefaultsExcept(dlg.get(), DefaultException::SizePolicyV
                                                           | DefaultException::SizePolicyH
                                                           | DefaultException::FocusMode
                                                           | DefaultException::PaletteClass
                                                           | DefaultException::StackingLayer));
    }

    SECTION("other") {
        dlg->reject();
        CHECK(typeid(*dlg->facet(Tui::ZDefaultWidgetManager::staticMetaObject)) == typeid(Tui::ZBasicDefaultWidgetManager));
        CHECK(dlg->facet(Tui::ZWindowFacet::staticMetaObject));
    }

}

// TODO modifiy window tests to also test dialog for visual and key behaviour at least

TEST_CASE("dialog-autoplacement") {
    Testhelper t("unused", "unused", 14, 5);

    auto runInLoop = [](auto init, auto check) {
        QTimer::singleShot(2000, [&] {
            FAIL_CHECK("Test timed out");
            QCoreApplication::instance()->exit(0);
        });

        QTimer::singleShot(0, init);
        std::function<void()> checkWrapper;
        checkWrapper = [&]{
            check();
            QTimer::singleShot(0, checkWrapper);
        };
        QTimer::singleShot(0, checkWrapper);
        QCoreApplication::instance()->exec();
    };

    const int cycleForInitiallyVisible = 2;
    const int cycleForInitiallyHidden = 3;

    SECTION("no parent") {
        std::unique_ptr<Tui::ZDialog> dlg;
        int counter = 0;
        runInLoop([&] {
            dlg = std::make_unique<Tui::ZDialog>(nullptr);
            Tui::ZHBoxLayout *layout = new Tui::ZHBoxLayout();
            dlg->setLayout(layout);
            layout->addWidget(new Tui::ZTextLine("test", dlg.get()));
        }, [&]{
            counter++;
            CHECK(!dlg->geometry().isValid());
            CHECK(dlg->isVisible());
            if (counter == cycleForInitiallyVisible) {
                QCoreApplication::instance()->exit(0);
            }
        });
    }

    SECTION("initially visible - manually placed") {
        Tui::ZDialog *dlg = nullptr;
        int counter = 0;
        runInLoop([&] {
            dlg = new Tui::ZDialog(t.root);
            Tui::ZHBoxLayout *layout = new Tui::ZHBoxLayout();
            dlg->setLayout(layout);
            layout->addWidget(new Tui::ZTextLine("test", dlg));
        }, [&]{
            counter++;
            if (dlg->geometry().isValid()) {
                CHECK(dlg->geometry() == QRect{5, 1, 6, 3});
                CHECK(dlg->isVisible());
                CHECK(counter == cycleForInitiallyVisible);
                QCoreApplication::instance()->exit(0);
            }
        });
    }

    SECTION("initially invisible") {
        Tui::ZDialog *dlg = nullptr;
        int counter = 0;
        runInLoop([&] {
            dlg = new Tui::ZDialog(t.root);
            Tui::ZHBoxLayout *layout = new Tui::ZHBoxLayout();
            dlg->setLayout(layout);
            layout->addWidget(new Tui::ZTextLine("test", dlg));
            dlg->setVisible(false);
        }, [&]{
            counter++;
            if (counter == 2) {
                dlg->setVisible(true);
            }
            if (dlg->geometry().isValid()) {
                CHECK(dlg->geometry() == QRect{5, 1, 6, 3});
                CHECK(dlg->isVisible());
                CHECK(counter == cycleForInitiallyHidden);
                QCoreApplication::instance()->exit(0);
            }
        });
    }

    SECTION("initially visible, with custom geometry") {
        Tui::ZDialog *dlg = nullptr;
        int counter = 0;
        runInLoop([&] {
            dlg = new Tui::ZDialog(t.root);
            Tui::ZHBoxLayout *layout = new Tui::ZHBoxLayout();
            dlg->setLayout(layout);
            layout->addWidget(new Tui::ZTextLine("test", dlg));
            dlg->setGeometry({8, 2, 6, 3});
        }, [&]{
            counter++;
            if (counter == cycleForInitiallyVisible && dlg->geometry().isValid()) {
                CHECK(dlg->geometry() == QRect{8, 2, 6, 3});
                CHECK(dlg->isVisible());
                QCoreApplication::instance()->exit(0);
            }
        });
    }


    SECTION("initially invisible, with custom geometry") {
        Tui::ZDialog *dlg = nullptr;
        int counter = 0;
        runInLoop([&] {
            dlg = new Tui::ZDialog(t.root);
            Tui::ZHBoxLayout *layout = new Tui::ZHBoxLayout();
            dlg->setLayout(layout);
            layout->addWidget(new Tui::ZTextLine("test", dlg));
            dlg->setVisible(false);
            dlg->setGeometry({8, 2, 6, 3});
        }, [&]{
            counter++;
            if (counter == 2) {
                dlg->setVisible(true);
            }
            if (counter == cycleForInitiallyHidden && dlg->geometry().isValid()) {
                CHECK(dlg->geometry() == QRect{8, 2, 6, 3});
                CHECK(dlg->isVisible());
                QCoreApplication::instance()->exit(0);
            }
        });
    }

}

TEST_CASE("dialog-behavior") {
    Testhelper t("unused", "unused", 14, 5);

    Tui::ZDialog *dlg = new Tui::ZDialog(t.root);

    SignalRecorder recorder;
    recorder.watchSignal(dlg, RECORDER_SIGNAL(&Tui::ZDialog::rejected));

    dlg->setFocus();
    EventCatcher catcher;
    auto defaultManager = dynamic_cast<Tui::ZDefaultWidgetManager*>(dlg->facet(Tui::ZDefaultWidgetManager::staticMetaObject));
    defaultManager->setDefaultWidget(&catcher);

    SECTION("escape key") {
        t.sendKey(Tui::Key_Escape);
        CHECK(recorder.consumeFirst(&Tui::ZDialog::rejected));
        CHECK(recorder.noMoreSignal());
        CHECK(dlg->isVisible() == false);
        CHECK(catcher.queue.size() == 0);

        dlg->setVisible(true);
        t.sendKey(Tui::Key_Escape, Tui::ControlModifier);
        CHECK(recorder.noMoreSignal());
        CHECK(dlg->isVisible() == true);
        CHECK(catcher.queue.size() == 0);

        dlg->setVisible(true);
        t.sendKey(Tui::Key_Escape, Tui::AltModifier);
        CHECK(recorder.noMoreSignal());
        CHECK(dlg->isVisible() == true);
        CHECK(catcher.queue.size() == 0);

        dlg->setVisible(true);
        t.sendKey(Tui::Key_Escape, Tui::ShiftModifier);
        CHECK(recorder.noMoreSignal());
        CHECK(dlg->isVisible() == true);
        CHECK(catcher.queue.size() == 0);
    }

    SECTION("enter key") {
        t.sendKey(Tui::Key_Enter);
        REQUIRE(catcher.queue.size() == 1);
        CHECK(catcher.queue[0].key == Tui::Key_Enter);
        CHECK(catcher.queue[0].modifiers == 0);
    }

    SECTION("enter key - no default") {
        defaultManager->setDefaultWidget(nullptr);
        t.sendKey(Tui::Key_Enter);
        CHECK(catcher.queue.size() == 0);
    }

    SECTION("reject") {
        dlg->reject();
        CHECK(recorder.consumeFirst(&Tui::ZDialog::rejected));
        CHECK(recorder.noMoreSignal());
        CHECK(catcher.queue.size() == 0);
    }
}

