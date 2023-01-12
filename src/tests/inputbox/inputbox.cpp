// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZInputBox.h>

#include "../catchwrapper.h"

#include "../Testhelper.h"
#include "../signalrecorder.h"
#include "../vcheck_zwidget.h"

TEST_CASE("inputbox-base", "") {
    bool parent = GENERATE(false, true);
    CAPTURE(parent);
    std::unique_ptr<Tui::ZWidget> w = parent ? std::make_unique<Tui::ZWidget>() : nullptr;

    SECTION("constructor without parent") {
        Tui::ZInputBox();
        Tui::ZInputBox("test");
    }

    SECTION("constructor with parent") {
        // The initialisation must not crash.
        delete new Tui::ZInputBox(w.get());
        delete new Tui::ZInputBox("default", w.get());
    }

    auto checkDefaultState = [] (Tui::ZInputBox *inputbox) {
        CHECK(inputbox->echoMode() == Tui::ZInputBox::Normal);
        CHECK(inputbox->sizeHint() == QSize{10, 1});
        CHECK(inputbox->sizePolicyH() == Tui::SizePolicy::Expanding);
        CHECK(inputbox->sizePolicyV() == Tui::SizePolicy::Fixed);
        CHECK(inputbox->focusPolicy() == Tui::StrongFocus);
        CHECK(inputbox->cursorStyle() == Tui::CursorStyle::Bar);
        CHECK(inputbox->overwriteMode() == false);
        FAIL_CHECK_VEC(checkWidgetsDefaultsExcept(inputbox, DefaultException::SizePolicyV
                                                          | DefaultException::SizePolicyH
                                                          | DefaultException::FocusPolicy
                                                          | DefaultException::CursorStyle));
    };

    SECTION("constructor") {
        std::unique_ptr<Tui::ZInputBox> inputbox = std::make_unique<Tui::ZInputBox>(w.get());
        CHECK(inputbox->text() == "");
        CHECK(inputbox->cursorPosition() == 0);
        checkDefaultState(inputbox.get());
    }

    SECTION("constructor-with-text") {
        std::unique_ptr<Tui::ZInputBox> inputbox = std::make_unique<Tui::ZInputBox>("default\ntext", w.get());
        CHECK(inputbox->text() == "default\ntext");
        CHECK(inputbox->cursorPosition() == 12);
        checkDefaultState(inputbox.get());
    }

    SECTION("abi-vcheck") {
        std::unique_ptr<Tui::ZInputBox> inputbox = std::make_unique<Tui::ZInputBox>(w.get());
        Tui::ZWidget base;
        checkZWidgetOverrides(&base, inputbox.get());
    }

    SECTION("get-set-echomode") {
        std::unique_ptr<Tui::ZInputBox> inputbox = std::make_unique<Tui::ZInputBox>(w.get());
        CHECK(inputbox->echoMode() == Tui::ZInputBox::Normal);
        inputbox->setEchoMode(Tui::ZInputBox::NoEcho);
        CHECK(inputbox->echoMode() == Tui::ZInputBox::NoEcho);
        inputbox->setEchoMode(Tui::ZInputBox::Password);
        CHECK(inputbox->echoMode() == Tui::ZInputBox::Password);
        inputbox->setEchoMode(Tui::ZInputBox::Normal);
        CHECK(inputbox->echoMode() == Tui::ZInputBox::Normal);
    }

    SECTION("get-set-text") {
        std::unique_ptr<Tui::ZInputBox> inputbox = std::make_unique<Tui::ZInputBox>(w.get());
        inputbox->setText("some text");
        CHECK(inputbox->text() == "some text");

        // Trigger "no change" detection code path
        inputbox->setText("some text");
        CHECK(inputbox->text() == "some text");

        // \n is special internally, so test that explicitly too.
        inputbox->setText("\n\ntest\ntext\n");
        CHECK(inputbox->text() == "\n\ntest\ntext\n");
    }

    SECTION("get-set-overwrite") {
        std::unique_ptr<Tui::ZInputBox> inputbox = std::make_unique<Tui::ZInputBox>(w.get());
        inputbox->setOverwriteMode(true);
        CHECK(inputbox->overwriteMode() == true);
        inputbox->setOverwriteMode(false);
        CHECK(inputbox->overwriteMode() == false);
    }

    SECTION("cursorPosition") {
        std::unique_ptr<Tui::ZInputBox> inputbox = std::make_unique<Tui::ZInputBox>(w.get());
        CHECK(inputbox->cursorPosition() == 0);
        inputbox->setCursorPosition(-1);
        CHECK(inputbox->cursorPosition() == 0);
        inputbox->setCursorPosition(1);
        CHECK(inputbox->cursorPosition() == 0);
    }

    SECTION("insertAtCursorPosition") {
        std::unique_ptr<Tui::ZInputBox> inputbox = std::make_unique<Tui::ZInputBox>(w.get());
        bool withSize = GENERATE(true, false);
        if (withSize) {
            inputbox->setGeometry({0, 0, 10, 1});
        }
        inputbox->insertAtCursorPosition("A");
        CHECK(inputbox->cursorPosition() == 1);
        inputbox->setCursorPosition(10);
        inputbox->insertAtCursorPosition("B");
        CHECK(inputbox->text() == "AB");
        CHECK(inputbox->cursorPosition() == 2);
        inputbox->setCursorPosition(1);
        inputbox->insertAtCursorPosition("あ😁");
        CHECK(inputbox->text() == "Aあ😁B");
        CHECK(inputbox->cursorPosition() == 4);

        // \n is special internally, so test that explicitly too.
        inputbox->insertAtCursorPosition("\n");
        CHECK(inputbox->text() == "Aあ😁\nB");
        CHECK(inputbox->cursorPosition() == 5);
    }

    SECTION("insertAtCursorPosition-with-text") {
        std::unique_ptr<Tui::ZInputBox> inputbox = std::make_unique<Tui::ZInputBox>("text", w.get());
        bool withSize = GENERATE(true, false);
        if (withSize) {
            inputbox->setGeometry({0, 0, 10, 1});
        }
        CHECK(inputbox->cursorPosition() == 4);
        inputbox->insertAtCursorPosition(" inserted");
        CHECK(inputbox->cursorPosition() == 13);
        CHECK(inputbox->text() == "text inserted");
    }
}

TEST_CASE("inputbox-cursor-position", "") {
    Testhelper t("inputbox", "unused", 10, 5);
    bool parent = GENERATE(false, true);
    CAPTURE(parent);
    std::unique_ptr<Tui::ZInputBox> inputbox = std::make_unique<Tui::ZInputBox>("text", parent ? t.root : nullptr);

    SECTION("composing") {
        inputbox->setText("ab\xcc\x88\xcc\xa4");
        CAPTURE(inputbox->text());
        inputbox->setCursorPosition(0);
        CHECK(inputbox->cursorPosition() == 0);

        inputbox->setCursorPosition(1);
        CHECK(inputbox->cursorPosition() == 1);

        inputbox->setCursorPosition(2);
        CHECK(inputbox->cursorPosition() == 4);

        inputbox->setCursorPosition(3);
        CHECK(inputbox->cursorPosition() == 4);

        inputbox->setCursorPosition(4);
        CHECK(inputbox->cursorPosition() == 4);

        inputbox->setCursorPosition(-1);
        CHECK(inputbox->cursorPosition() == 0);

        inputbox->setCursorPosition(5);
        CHECK(inputbox->cursorPosition() == 4);
    }

    SECTION("astral") {
        inputbox->setText("a😁b");
        CAPTURE(inputbox->text());
        inputbox->setCursorPosition(0);
        CHECK(inputbox->cursorPosition() == 0);

        inputbox->setCursorPosition(1);
        CHECK(inputbox->cursorPosition() == 1);

        inputbox->setCursorPosition(2);
        CHECK(inputbox->cursorPosition() == 3);

        inputbox->setCursorPosition(3);
        CHECK(inputbox->cursorPosition() == 3);

        inputbox->setCursorPosition(4);
        CHECK(inputbox->cursorPosition() == 4);
    }
}

TEST_CASE("inputbox-behavior", "") {
    Testhelper t("inputbox", "inputbox-behavior", 10, 5);
    // TODO (ff) TestBackground
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 10, 5});
    Tui::ZInputBox *inputbox = new Tui::ZInputBox(w);
    inputbox->setGeometry({2, 2, 5, 1});
    inputbox->setFocus();

    SECTION("char-A-5x1+2+2") {
        t.sendChar("A");
        CHECK(inputbox->cursorPosition() == 1);
        CHECK(inputbox->text() == "A");
        t.compare();
    }

    SECTION("char-B-and-Space") {
        t.sendChar("B");
        t.sendKey(Tui::Key_Space);
        CHECK(inputbox->cursorPosition() == 2);
        CHECK(inputbox->text() == "B ");
    }

    SECTION("char-ABCD-5x1+2+2") {
        t.sendChar("A");
        t.sendChar("B");
        t.sendChar("C");
        t.sendChar("D");
        CHECK(inputbox->cursorPosition() == 4);
        CHECK(inputbox->text() == "ABCD");
        t.compare();
    }

    SECTION("char-ABCDE-5x1+2+2") {
        t.sendChar("A");
        t.sendChar("B");
        t.sendChar("C");
        t.sendChar("D");
        t.sendChar("E");
        CHECK(inputbox->cursorPosition() == 5);
        CHECK(inputbox->text() == "ABCDE");
        t.compare();
    }

    SECTION("char-ABCDEF-5x1+2+2") {
        t.sendChar("A");
        t.sendChar("B");
        t.sendChar("C");
        t.sendChar("D");
        t.sendChar("E");
        t.sendChar("F");
        CHECK(inputbox->cursorPosition() == 6);
        CHECK(inputbox->text() == "ABCDEF");
        t.compare();
    }

    SECTION("paste1-ABCDEF-5x1+2+2") {
        t.sendPaste("ABCDEF");
        CHECK(inputbox->cursorPosition() == 6);
        CHECK(inputbox->text() == "ABCDEF");
        t.compare("char-ABCDEF-5x1+2+2");
    }

    SECTION("paste6-ABCDEF-5x1+2+2") {
        t.sendPaste("A");
        t.sendPaste("B");
        t.sendPaste("C");
        t.sendPaste("D");
        t.sendPaste("E");
        t.sendPaste("F");
        CHECK(inputbox->cursorPosition() == 6);
        CHECK(inputbox->text() == "ABCDEF");
        t.compare("char-ABCDEF-5x1+2+2");
    }

    SECTION("char-left2-ABCDEF-5x1+2+2") {
        inputbox->setGeometry({2, 2, 5, 1});
        t.sendChar("A");
        t.sendChar("B");
        t.sendChar("C");
        t.sendChar("D");
        t.sendChar("E");
        t.sendChar("F");
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        CHECK(inputbox->cursorPosition() == 4);
        CHECK(inputbox->text() == "ABCDEF");
        t.compare("char-ABCDEF-5x1+2+2");
    }

    SECTION("char-left4-ABCDEF-5x1+2+2") {
        inputbox->setGeometry({2, 2, 5, 1});
        t.sendChar("A");
        t.sendChar("B");
        t.sendChar("C");
        t.sendChar("D");
        t.sendChar("E");
        t.sendChar("F");
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        CHECK(inputbox->cursorPosition() == 2);
        CHECK(inputbox->text() == "ABCDEF");
        t.compare();
    }

    SECTION("char-left6-ABCDEF-5x1+2+2") {
        inputbox->setGeometry({2, 2, 5, 1});
        t.sendChar("A");
        t.sendChar("B");
        t.sendChar("C");
        t.sendChar("D");
        t.sendChar("E");
        t.sendChar("F");
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        CHECK(inputbox->cursorPosition() == 0);
        CHECK(inputbox->text() == "ABCDEF");
        t.compare();
    }

    SECTION("char-left7-ABCDEF-5x1+2+2") {
        inputbox->setGeometry({2, 2, 5, 1});
        t.sendChar("A");
        t.sendChar("B");
        t.sendChar("C");
        t.sendChar("D");
        t.sendChar("E");
        t.sendChar("F");
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        CHECK(inputbox->cursorPosition() == 0);
        CHECK(inputbox->text() == "ABCDEF");
        t.compare("char-left6-ABCDEF-5x1+2+2");
    }

    SECTION("char-home-ABCDEF-5x1+2+2") {
        inputbox->setGeometry({2, 2, 5, 1});
        t.sendChar("A");
        t.sendChar("B");
        t.sendChar("C");
        t.sendChar("D");
        t.sendChar("E");
        t.sendChar("F");
        t.sendKey(Tui::Key_Home);
        CHECK(inputbox->cursorPosition() == 0);
        CHECK(inputbox->text() == "ABCDEF");
        t.compare("char-left6-ABCDEF-5x1+2+2");
    }

    SECTION("char-insert-AaBbCDEF-5x1+2+2") {
        inputbox->setGeometry({2, 2, 5, 1});
        t.sendChar("A");
        t.sendChar("B");
        t.sendChar("C");
        t.sendChar("D");
        t.sendChar("E");
        t.sendChar("F");
        t.sendKey(Tui::Key_Home);
        t.sendKey(Tui::Key_Right);
        t.sendChar("a");
        t.sendKey(Tui::Key_Right);
        t.sendChar("b");
        CHECK(inputbox->cursorPosition() == 4);
        CHECK(inputbox->text() == "AaBbCDEF");
        t.compare();
    }

    SECTION("char-left2-end-ABCDEF-5x1+2+2") {
        inputbox->setGeometry({2, 2, 5, 1});
        t.sendChar("A");
        t.sendChar("B");
        t.sendChar("C");
        t.sendChar("D");
        t.sendChar("E");
        t.sendChar("F");
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_End);
        CHECK(inputbox->cursorPosition() == 6);
        CHECK(inputbox->text() == "ABCDEF");
        t.compare("char-ABCDEF-5x1+2+2");
    }

    SECTION("char-ABCDEF-up-5x1+2+2") {
        t.sendChar("A");
        t.sendChar("B");
        t.sendChar("C");
        t.sendChar("D");
        t.sendChar("E");
        t.sendChar("F");
        t.sendKey(Tui::Key_Up);
        CHECK(inputbox->cursorPosition() == 6);
        CHECK(inputbox->text() == "ABCDEF");
        t.compare("char-ABCDEF-5x1+2+2");
    }

    SECTION("char-ABCDEF-down-5x1+2+2") {
        t.sendChar("A");
        t.sendChar("B");
        t.sendChar("C");
        t.sendChar("D");
        t.sendChar("E");
        t.sendChar("F");
        t.sendKey(Tui::Key_Down);
        CHECK(inputbox->cursorPosition() == 6);
        CHECK(inputbox->text() == "ABCDEF");
        t.compare("char-ABCDEF-5x1+2+2");
    }

    SECTION("char-ABCDEF-enter-5x1+2+2") {
        t.sendChar("A");
        t.sendChar("B");
        t.sendChar("C");
        t.sendChar("D");
        t.sendChar("E");
        t.sendChar("F");
        t.sendKey(Tui::Key_Enter);
        CHECK(inputbox->cursorPosition() == 6);
        CHECK(inputbox->text() == "ABCDEF");
        t.compare("char-ABCDEF-5x1+2+2");
    }

    SECTION("char-CDEF-delete-5x1+2+2") {
        t.sendKey(Tui::Key_Delete);
        t.sendChar("A");
        t.sendChar("B");
        t.sendChar("C");
        t.sendChar("D");
        t.sendChar("E");
        t.sendChar("F");
        t.sendKey(Tui::Key_Home);
        t.sendKey(Tui::Key_Delete);
        t.sendKey(Tui::Key_Delete);
        CHECK(inputbox->cursorPosition() == 0);
        CHECK(inputbox->text() == "CDEF");
        t.compare("char-ABCDEF-5x1+2+2");
    }

    SECTION("char-ABCDEF-backspace-5x1+2+2") {
        t.sendChar("A");
        t.sendChar("B");
        t.sendChar("C");
        t.sendChar("D");
        t.sendChar("E");
        t.sendChar("F");
        t.sendKey(Tui::Key_Backspace);
        t.sendKey(Tui::Key_Backspace);
        CHECK(inputbox->cursorPosition() == 4);
        CHECK(inputbox->text() == "ABCD");
        t.compare("char-ABCD-5x1+2+2");
    }

    SECTION("backspace-charABCDX-backspace-5x1+2+2") {
        t.sendKey(Tui::Key_Backspace);
        t.sendChar("A");
        t.sendChar("B");
        t.sendChar("C");
        t.sendChar("D");
        t.sendChar("X");
        t.sendKey(Tui::Key_Backspace);
        CHECK(inputbox->cursorPosition() == 4);
        CHECK(inputbox->text() == "ABCD");
        t.compare("char-ABCD-5x1+2+2");
    }

    SECTION("backspace-surrogate-pair") {
        inputbox->setText("😁hello world😁");
        t.sendKey(Tui::Key_Backspace);
        CHECK(inputbox->text() == "😁hello world");
        t.compare("backspace-wide-char");
    }

    SECTION("delete-surrogate-pair") {
        inputbox->setText("😁hello world😁");
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Delete);
        CHECK(inputbox->text() == "😁hello world");
        t.compare("backspace-wide-char");
    }

    SECTION("backspace-wide-char") {
        inputbox->setText("あhello worldあ");
        t.sendKey(Tui::Key_Backspace);
        CHECK(inputbox->text() == "あhello world");
        t.compare();
    }

    SECTION("delete-wide-char") {
        inputbox->setText("あhello worldあ");
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Delete);
        CHECK(inputbox->text() == "あhello world");
        t.compare("backspace-wide-char");
    }

    SECTION("scroll-wide-surrogate-pair") {
        inputbox->setText("😁he");
        t.sendChar("A");
        t.compare("scroll-wide-char");
    }

    SECTION("scroll-paste-wide-surrogate-pair") {
        inputbox->setText("😁he");
        t.sendPaste("A");
        t.compare("scroll-wide-char");
    }

    SECTION("scroll2-wide-surrogate-pair") {
        inputbox->setText("😁he");
        t.sendChar("A");
        t.sendChar("A");
        t.compare();
    }

    SECTION("scroll-wide-char") {
        inputbox->setText("あhe");
        t.sendChar("A");
        t.compare();
    }

    SECTION("scroll-wide-at-end") {
        inputbox->setText("bcdあ");
        inputbox->setCursorPosition(0);
        t.sendChar("A");
        t.sendKey(Tui::Key_Right);
        t.sendKey(Tui::Key_Right);
        t.sendKey(Tui::Key_Right);
        t.compare();
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({3, 0}));
    }

    SECTION("scroll-wide-at-end") {
        inputbox->setText("bcdあ");
        inputbox->setCursorPosition(0);
        t.sendChar("A");
        t.sendKey(Tui::Key_Right);
        t.sendKey(Tui::Key_Right);
        t.sendKey(Tui::Key_Right);
        t.compare();
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({3, 0}));
    }

    SECTION("disable") {
        inputbox->setText("disable");
        inputbox->setEnabled(false);
        inputbox->setFocus();
        t.sendChar("A");
        CHECK(inputbox->text() == "disable");
        CHECK(t.terminal->focusWidget() != inputbox);
        t.compare();
        CHECK(inputbox->cursorPosition() == 7);
        inputbox->insertAtCursorPosition("d");
        CHECK(inputbox->text() == "disabled");
    }

    SECTION("without-focus") {
        Tui::ZInputBox *inputbox2 = new Tui::ZInputBox(w);
        inputbox2->setGeometry({2, 3, 7, 1});
        inputbox->setGeometry({2, 1, 7, 1});

        inputbox->setText("without focus ");
        CHECK(inputbox->cursorPosition() == 14);
        t.sendChar("A");

        inputbox2->setText("with focus");
        inputbox2->setFocus();

        CHECK(inputbox->cursorPosition() == 15);
        CHECK(inputbox->text() == "without focus A");
        CHECK(inputbox->focus() == false);
        CHECK(inputbox2->focus() == true);
        t.compare();
    }

    SECTION("cursorPosition") {
        Tui::ZInputBox *inputbox = new Tui::ZInputBox(w);
        inputbox->setFocus();
        CHECK(inputbox->cursorPosition() == 0);
        inputbox->setCursorPosition(-1);
        CHECK(inputbox->cursorPosition() == 0);
        t.sendChar("A");
        inputbox->setCursorPosition(10);
        CHECK(inputbox->cursorPosition() == 1);
        t.sendChar("B");
        CHECK(inputbox->text() == "AB");
        CHECK(inputbox->cursorPosition() == 2);
        inputbox->setCursorPosition(1);
        t.sendChar("C");
        CHECK(inputbox->text() == "ACB");
        CHECK(inputbox->cursorPosition() == 2);
    }

    SECTION("scroll-resize-end") {
        inputbox->setFocus();
        inputbox->setGeometry({2, 2, 12, 1});
        inputbox->setText("01234567890");
        inputbox->setCursorPosition(inputbox->text().size());
        t.render();
        inputbox->setGeometry({2, 2, 6, 1});
        t.compare();
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({5, 0}));
    }

    SECTION("scroll-resize-mid") {
        inputbox->setFocus();
        inputbox->setGeometry({2, 2, 12, 1});
        inputbox->setText("01234567890");
        inputbox->setCursorPosition(7);
        t.render();
        inputbox->setGeometry({2, 2, 6, 1});
        t.compare();
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({5, 0}));
    }

    SECTION("scroll-resize-mid-wide") {
        inputbox->setFocus();
        inputbox->setGeometry({2, 2, 12, 1});
        inputbox->setText("0123456あ890");
        inputbox->setCursorPosition(7);
        t.render();
        inputbox->setGeometry({2, 2, 6, 1});
        t.compare();
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({4, 0}));
    }

    SECTION("scroll-resize-bigger") {
        t.terminal->resize(17, 5);
        w->setGeometry({0, 0, 17, 5});
        inputbox->setFocus();
        inputbox->setGeometry({2, 2, 6, 1});
        inputbox->setText("01234567890");
        inputbox->setCursorPosition(inputbox->text().size());
        t.render();
        inputbox->setGeometry({2, 2, 13, 1});
        t.compare();
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({11, 0}));
    }


    SECTION("scroll-resize-bigger11") {
        t.terminal->resize(15, 5);
        w->setGeometry({0, 0, 15, 5});
        inputbox->setFocus();
        inputbox->setGeometry({2, 2, 6, 1});
        inputbox->setText("01234567890");
        inputbox->setCursorPosition(inputbox->text().size());
        t.render();
        inputbox->setGeometry({2, 2, 11, 1});
        t.compare();
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({10, 0}));
    }

    SECTION("tab-bubbles") {
        FAIL_CHECK_VEC(t.checkKeyEventBubblesToParent(Tui::Key_Tab));
    }

}

TEST_CASE("inputbox", "") {
    Testhelper t("inputbox", "inputbox", 25, 5);

    Tui::ZWindow *w = new Tui::ZWindow(t.root);

    QString windowType = GENERATE(as<QString>(), "", "dialog", "cyan");

    if (windowType.size()) {
        t.namePrefix += "-" + windowType;
        w->setPaletteClass({"window", windowType });
    }

    w->setGeometry({0, 0, 25, 5});

    Tui::ZInputBox *inputbox = new Tui::ZInputBox(w);
    inputbox->setGeometry({2, 2, 20, 1});

    bool hasFocus = GENERATE(false, true);

    if (hasFocus) {
        t.namePrefix += "-focus";
        inputbox->setFocus();
    }

    SECTION("bare-20x1+2+2") {
        t.compare();
    }

    SECTION("text-hello-20x1+2+2") {
        inputbox->setText("hello");
        t.compare();
    }

    SECTION("text-len19-20x1+2+2") {
        inputbox->setText("1 2 3 4 5 6 7 8 cba");
        t.compare();
    }

    SECTION("text-len20-20x1+2+2") {
        inputbox->setText("1 2 3 4 5 6 7 8 9cba");
        t.compare();
    }

    SECTION("text-len21-20x1+2+2") {
        inputbox->setText("1 2 3 4 5 6 7 8 9 cba");
        t.compare();
    }


    SECTION("text-hello-0x1+2+2") {
        inputbox->setGeometry({2, 2, 0, 1});
        inputbox->setText("hello world");
        t.compare();
    }

    SECTION("text-hello-1x0+2+2") {
        inputbox->setGeometry({2, 2, 1, 0});
        inputbox->setText("hello world");
        t.compare("text-hello-0x1+2+2");
    }

    SECTION("text-hello-1x1+2+2") {
        inputbox->setGeometry({2, 2, 1, 1});
        inputbox->setText("hello world");
        t.compare();
    }

    SECTION("text-hello-2x1+2+2") {
        inputbox->setGeometry({2, 2, 2, 1});
        inputbox->setText("hello world");
        t.compare();
    }

    SECTION("text-hello-3x1+2+2") {
        inputbox->setGeometry({2, 2, 3, 1});
        inputbox->setText("hello world");
        t.compare();
    }

    SECTION("text-hello-21x1+2+2") {
        inputbox->setGeometry({2, 2, 21, 1});
        inputbox->setText("hello world");
        t.compare();
    }

    SECTION("text-newline") {
        inputbox->setGeometry({2, 2, 21, 1});
        inputbox->setText("hello\nworld");
        t.compare();
        if (hasFocus) {
            inputbox->setText("");
            t.sendPaste("hello\nworld");
            t.compare();
        }
        inputbox->setText("");
        inputbox->insertAtCursorPosition("hello\nworld");
        t.compare();
    }

    SECTION("margins-hello") {
        inputbox->setGeometry({2, 1, 21, 3});
        inputbox->setContentsMargins({2, 1, 3, 1});
        inputbox->setText("hello world");
        t.compare();
    }
}

TEST_CASE("inputbox-echomode", "") {
    Testhelper t("inputbox", "inputbox-echomode", 15, 5);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 15, 5});
    Tui::ZInputBox *inputbox = new Tui::ZInputBox(w);
    inputbox->setGeometry({2, 2, 5, 1});
    inputbox->setFocus();

    SECTION("char-AB-Normal") {
        inputbox->setEchoMode(Tui::ZInputBox::Normal);
        t.sendChar("A");
        t.sendChar("B");

        CHECK(inputbox->cursorPosition() == 2);
        CHECK(inputbox->text() == "AB");
        t.compare();
    }

    SECTION("char-AB-Password") {
        inputbox->setEchoMode(Tui::ZInputBox::Password);
        t.sendChar("A");
        t.sendChar("B");
        CHECK(inputbox->cursorPosition() == 2);
        CHECK(inputbox->text() == "AB");
        t.compare();
        inputbox->setText("ab\xcc\x88\xcc\xa4");
        t.compare();
        inputbox->setText("あb");
        t.compare();
        inputbox->setText("😁b");
    }

    // Scrolling out must not lead to changes.
    SECTION("char-A-F-Password") {
        inputbox->setEchoMode(Tui::ZInputBox::Password);
        t.sendChar("A");
        t.sendChar("B");
        t.sendChar("C");
        t.sendChar("D");
        t.sendChar("E");
        t.sendChar("F");
        CHECK(inputbox->cursorPosition() == 6);
        CHECK(inputbox->text() == "ABCDEF");
        t.compare();
    }

    SECTION("char-AB-NoEcho") {
        inputbox->setEchoMode(Tui::ZInputBox::NoEcho);
        t.sendChar("A");
        t.sendChar("B");
        CHECK(inputbox->cursorPosition() == 2);
        CHECK(inputbox->text() == "AB");
        t.compare("NoEcho");
    }

    // When scrolling out, nothing must be visible and the position must still be correct.
    SECTION("char-A-F-NoEcho") {
        inputbox->setEchoMode(Tui::ZInputBox::NoEcho);
        t.sendChar("A");
        t.sendChar("B");
        t.sendChar("C");
        t.sendChar("D");
        t.sendChar("E");
        t.sendChar("F");
        CHECK(inputbox->cursorPosition() == 6);
        CHECK(inputbox->text() == "ABCDEF");
        t.compare("NoEcho");
    }

    SECTION("Editing") {
        auto echoMode = GENERATE(Tui::ZInputBox::Normal, Tui::ZInputBox::Password, Tui::ZInputBox::NoEcho);
        inputbox->setEchoMode(echoMode);
        t.sendChar("A");
        t.sendChar("B");
        t.sendChar("C");
        CHECK(inputbox->text() == "ABC");
        CHECK(inputbox->cursorPosition() == 3);
        t.sendKey(Tui::Key_Left);
        CHECK(inputbox->cursorPosition() == 2);
        t.sendKey(Tui::Key_Left);
        CHECK(inputbox->cursorPosition() == 1);
        t.sendKey(Tui::Key_Right);
        CHECK(inputbox->cursorPosition() == 2);
        t.sendKey(Tui::Key_Right);
        CHECK(inputbox->cursorPosition() == 3);
        inputbox->setCursorPosition(1);
        CHECK(inputbox->cursorPosition() == 1);
        inputbox->setCursorPosition(2);
        t.sendKey(Tui::Key_Backspace);
        CHECK(inputbox->text() == "AC");
        CHECK(inputbox->cursorPosition() == 1);
        t.sendKey(Tui::Key_Delete);
        CHECK(inputbox->text() == "A");
        CHECK(inputbox->cursorPosition() == 1);
    }
}

TEST_CASE("inputbox-attach-terminal", "") {
    Testhelper t("inputbox", "unused", 15, 5);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 15, 5});

    Tui::ZInputBox *inputbox = new Tui::ZInputBox();
    inputbox->setText("a😁b");
    inputbox->setCursorPosition(2);
    CHECK(inputbox->cursorPosition() == 3);

    // This should trigger readjustment of the cursor position using the terminal's text metrics.
    inputbox->setParent(w);

    CHECK(inputbox->cursorPosition() == 3);
}

TEST_CASE("inputbox-event", "") {
    Testhelper t("inputbox", "inputbox-event", 15, 5);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 15, 5});
    Tui::ZInputBox *inputbox = new Tui::ZInputBox(w);
    inputbox->setGeometry({2, 2, 5, 1});
    inputbox->setFocus();

    SignalRecorder recorder;
    recorder.watchSignal(inputbox, RECORDER_SIGNAL(&Tui::ZInputBox::textChanged));

    inputbox->setText("no change");
    CHECK(recorder.consumeFirst(&Tui::ZInputBox::textChanged, QString("no change")));
    CHECK(recorder.noMoreSignal());
    CHECK(inputbox->cursorPosition() == 9);
    inputbox->insertAtCursorPosition("no change");
    CHECK(recorder.consumeFirst(&Tui::ZInputBox::textChanged, QString("no changeno change")));
    CHECK(recorder.noMoreSignal());
    CHECK(inputbox->cursorPosition() == 18);
    inputbox->setCursorPosition(9);
    t.sendKey(Tui::Key_Space);
    CHECK(recorder.consumeFirst(&Tui::ZInputBox::textChanged, QString("no change no change")));
    CHECK(recorder.noMoreSignal());
    t.sendKey(Tui::Key_Enter);
    t.sendKey(Tui::Key_Home);
    t.sendKey(Tui::Key_End);
    t.sendKey(Tui::Key_Insert);
    t.sendKey(Tui::Key_Insert); // When we turn it on, we change the test behaviour. Therefore, we turn it off again.
    t.sendKey(Tui::Key_Left);
    t.sendKey(Tui::Key_Right);
    t.sendKey(Tui::Key_Up);
    t.sendKey(Tui::Key_Down);
    CHECK(recorder.noMoreSignal());

    // \n is special internally, so test that explicitly too.
    inputbox->setCursorPosition(9);
    inputbox->insertAtCursorPosition("\n");
    CHECK(recorder.consumeFirst(&Tui::ZInputBox::textChanged, QString("no change\n no change")));
    CHECK(recorder.noMoreSignal());

    inputbox->setText("Some\ntext");
    CHECK(recorder.consumeFirst(&Tui::ZInputBox::textChanged, QString("Some\ntext")));
    CHECK(recorder.noMoreSignal());

    t.sendKey(Tui::Key_Backspace);
    CHECK(recorder.consumeFirst(&Tui::ZInputBox::textChanged, QString("Some\ntex")));
    CHECK(recorder.noMoreSignal());

    inputbox->setCursorPosition(0);
    t.sendKey(Tui::Key_Delete);
    CHECK(recorder.consumeFirst(&Tui::ZInputBox::textChanged, QString("ome\ntex")));
    CHECK(recorder.noMoreSignal());

    t.sendPaste("s");
    CHECK(recorder.consumeFirst(&Tui::ZInputBox::textChanged, QString("some\ntex")));
    CHECK(recorder.noMoreSignal());
}


TEST_CASE("inputbox-small1", "") {
    Testhelper t("inputbox", "inputbox-small1", 6, 3);

    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 6, 3});

    Tui::ZInputBox *inputbox = new Tui::ZInputBox(w);
    inputbox->setGeometry({1, 1, 1, 1});
    inputbox->setFocus();

    SECTION("a") {
        inputbox->setText("a");
        t.compare();
        CHECK(t.terminal->grabCursorPosition() == QPoint{-1, -1});
    }

    SECTION("b") {
        inputbox->setText("b");
        t.compare("b");
        CHECK(t.terminal->grabCursorPosition() == QPoint{-1, -1});
    }

    SECTION("ab") {
        inputbox->setText("ab");
        t.compare("b");
        CHECK(t.terminal->grabCursorPosition() == QPoint{-1, -1});
    }

    SECTION("ab-Backspace") {
        inputbox->setText("ab");
        t.sendKey(Tui::Key_Backspace);
        t.compare("a");
        CHECK(t.terminal->grabCursorPosition() == QPoint{-1, -1});
    }

    SECTION("ab-Home") {
        inputbox->setText("ab");
        inputbox->setCursorPosition(2);
        t.sendKey(Tui::Key_Home);
        t.compare("a");
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({0, 0}));
    }

    SECTION("ab-Delete") {
        inputbox->setText("ab");
        t.sendKey(Tui::Key_Delete);
        t.compare("b");
        CHECK(t.terminal->grabCursorPosition() == QPoint{-1, -1});
    }

    SECTION("clear") {
        t.compare("clear");
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({0, 0}));
    }

    SECTION("smiley") {
        inputbox->setText("😁");
        t.compare("empty");
        CHECK(t.terminal->grabCursorPosition() == QPoint{-1, -1});
    }

    SECTION("smiley-right") {
        inputbox->setText("😁");
        t.sendKey(Tui::Key_Right);
        CHECK(inputbox->cursorPosition() == 2);
        t.compare("empty");
        CHECK(t.terminal->grabCursorPosition() == QPoint{-1, -1});
    }

    SECTION("smiley-left") {
        inputbox->setText("😁");
        CHECK(inputbox->cursorPosition() == 2);
        t.sendKey(Tui::Key_Left);
        CHECK(inputbox->cursorPosition() == 0);
        t.compare("empty");
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({0, 0}));
    }

    SECTION("smiley-Backspace") {
        inputbox->setText("😁");
        t.sendKey(Tui::Key_Backspace);
        CHECK(inputbox->text() == "");
        CHECK(inputbox->cursorPosition() == 0);
        t.compare("clear");
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({0, 0}));
    }

    SECTION("smiley-Delete") {
        inputbox->setText("😁");
        t.sendKey(Tui::Key_Home);
        t.sendKey(Tui::Key_Delete);
        CHECK(inputbox->cursorPosition() == 0);
        CHECK(inputbox->text() == "");
        t.compare("clear");
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({0, 0}));
    }

    SECTION("tab") {
        inputbox->setText("\t");
        t.compare("clear");
        CHECK(t.terminal->grabCursorPosition() == QPoint{-1, -1});
    }
}

TEST_CASE("inputbox-small2", "") {
    Testhelper t("inputbox", "inputbox-small2", 6, 3);

    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 6, 3});

    Tui::ZInputBox *inputbox = new Tui::ZInputBox(w);
    inputbox->setGeometry({1, 1, 2, 1});
    inputbox->setFocus();

    SECTION("clear") {
        t.compare();
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({0, 0}));
    }

    SECTION("a") {
        inputbox->setText("a");
        t.compare("a-clear");
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({1, 0}));
    }

    SECTION("b") {
        inputbox->setText("ab");
        t.compare();
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({1, 0}));
    }

    SECTION("ab") {
        inputbox->setText("ab");
        CHECK(inputbox->cursorPosition() == 2);
        t.sendKey(Tui::Key_Left);
        CHECK(inputbox->cursorPosition() == 1);
        t.compare();
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({1, 0}));
    }

    SECTION("ab2") {
        inputbox->setText("ab");
        CHECK(inputbox->cursorPosition() == 2);
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        CHECK(inputbox->cursorPosition() == 0);
        t.compare("ab");
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({0, 0}));
    }

    SECTION("ab-Backspace") {
        inputbox->setText("ab");
        t.sendKey(Tui::Key_Backspace);
        CHECK(inputbox->cursorPosition() == 1);
        t.compare("a-clear");
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({1, 0}));
    }

    SECTION("ab-Delete") {
        inputbox->setText("ab");
        t.sendKey(Tui::Key_Home);
        t.sendKey(Tui::Key_Delete);
        CHECK(inputbox->cursorPosition() == 0);
        t.compare("b");
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({0, 0}));
    }

    SECTION("smiley") {
        inputbox->setText("😁");
        CHECK(inputbox->cursorPosition() == 2);
        t.compare("smiley");
        CHECK(t.terminal->grabCursorPosition() == QPoint{-1, -1});
    }

    SECTION("asmiley") {
        inputbox->setText("a😁");
        CHECK(inputbox->cursorPosition() == 3);
        t.compare("smiley");
        CHECK(t.terminal->grabCursorPosition() == QPoint{-1, -1});
    }

    SECTION("asmiley-left") {
        inputbox->setText("a😁");
        t.sendKey(Tui::Key_Left);
        CHECK(inputbox->cursorPosition() == 1);
        t.compare("a-empty");
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({1, 0}));
    }

    SECTION("asmiley-left-Backspace") {
        inputbox->setText("a😁");
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Backspace);
        CHECK(inputbox->cursorPosition() == 0);
        t.compare("smiley");
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({0, 0}));
    }

    SECTION("asmiley-home") {
        inputbox->setText("a😁");
        t.sendKey(Tui::Key_Home);
        CHECK(inputbox->cursorPosition() == 0);
        t.compare("a-empty");
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({0, 0}));
    }

    SECTION("asmiley-delete") {
        inputbox->setText("a😁");
        t.sendKey(Tui::Key_Home);
        t.sendKey(Tui::Key_Delete);
        CHECK(inputbox->cursorPosition() == 0);
        t.compare("smiley");
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({0, 0}));
    }

    SECTION("asmileya") {
        inputbox->setText("a😁a");
        CHECK(inputbox->cursorPosition() == 4);
        t.compare("a-clear");
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({1, 0}));
    }

    SECTION("asmileya-left") {
        inputbox->setText("a😁a");
        t.sendKey(Tui::Key_Left);
        CHECK(inputbox->cursorPosition() == 3);
        t.compare("smiley");
        CHECK(t.terminal->grabCursorPosition() == QPoint{-1, -1});
    }

    SECTION("asmileya-left2") {
        inputbox->setText("a😁a");
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        CHECK(inputbox->cursorPosition() == 1);
        t.compare("a-empty");
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({1, 0}));
    }

    SECTION("tab") {
        inputbox->setText("\t");
        t.compare("clear");
        CHECK(t.terminal->grabCursorPosition() == QPoint{-1, -1});
    }
}

TEST_CASE("inputbox-small3", "") {
    Testhelper t("inputbox", "inputbox-small3", 6, 3);

    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 6, 3});

    Tui::ZInputBox *inputbox = new Tui::ZInputBox(w);
    inputbox->setGeometry({1, 1, 3, 1});
    inputbox->setFocus();

    SECTION("clear") {
        t.compare();
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({0, 0}));
    }

    SECTION("smiley-clear") {
        inputbox->setText("😇😁");
        CHECK(inputbox->cursorPosition() == 4);
        t.compare("smiley-clear");
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({2, 0}));
    }

    SECTION("smiley-left") {
        inputbox->setText("😁😇");
        t.sendKey(Tui::Key_Left);
        CHECK(inputbox->cursorPosition() == 2);
        t.compare("smiley-empty");
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({2, 0}));
    }

    SECTION("smiley-left2") {
        inputbox->setText("😁😇");
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        CHECK(inputbox->cursorPosition() == 0);
        t.compare("smiley-empty");
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({0, 0}));
    }

    SECTION("smiley-left-a") {
        inputbox->setText("😇😁");
        t.sendKey(Tui::Key_Left);
        CHECK(inputbox->cursorPosition() == 2);
        t.sendChar("a");
        CHECK(inputbox->cursorPosition() == 3);
        t.compare("asmiley");
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({1, 0}));
    }

    SECTION("smiley-left-ba") {
        inputbox->setText("😇😁");
        t.sendKey(Tui::Key_Left);
        t.sendChar("b");
        t.sendChar("a");
        CHECK(inputbox->cursorPosition() == 4);
        t.compare("asmiley");
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({1, 0}));
    }

    SECTION("cde-left") {
        inputbox->setText("abcdefg");
        CHECK(inputbox->cursorPosition() == 7);
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        CHECK(inputbox->cursorPosition() == 3);
        t.compare("cde");
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({1, 0}));
    }

    SECTION("cde-right") {
        inputbox->setText("abcdefg");
        t.sendKey(Tui::Key_Home);
        t.sendKey(Tui::Key_Right);
        t.sendKey(Tui::Key_Right);
        t.sendKey(Tui::Key_Right);
        t.sendKey(Tui::Key_Right);
        CHECK(inputbox->cursorPosition() == 4);
        t.compare("cde");
        CHECK(t.terminal->grabCursorPosition() == inputbox->mapToTerminal({2, 0}));
    }

    SECTION("tab") {
        inputbox->setText("\t");
        t.compare("clear");
        CHECK(t.terminal->grabCursorPosition() == QPoint{-1, -1});
    }
}

TEST_CASE("insert-mode", "") {
    Testhelper t("inputbox", "unused", 10, 3);

    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 10, 3});

    Tui::ZInputBox *inputbox = new Tui::ZInputBox(w);
    inputbox->setGeometry({1, 1, 8, 1});
    inputbox->setFocus();

    SECTION("ABcdefg") {
        inputbox->setText("abcdefg");
        CHECK(inputbox->overwriteMode() == false);
        t.sendKey(Tui::Key_Insert);
        CHECK(inputbox->overwriteMode() == true);
        t.sendKey(Tui::Key_Home);
        t.sendChar("A");
        t.sendChar("B");
        CHECK(inputbox->text() == "ABcdefg");
    }

    SECTION("insert-over-abcdeFGhi") {
        inputbox->setText("abcdefg");
        CHECK(inputbox->overwriteMode() == false);
        t.sendKey(Tui::Key_Insert);
        CHECK(inputbox->overwriteMode() == true);
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        t.sendChar("F");
        t.sendChar("G");
        t.sendChar("h");
        t.sendChar("i");
        CHECK(inputbox->text() == "abcdeFGhi");
    }

    SECTION("paste-over-abcdeFGhi-paste") {
        inputbox->setText("abcdefg");
        CHECK(inputbox->overwriteMode() == false);
        t.sendKey(Tui::Key_Insert);
        CHECK(inputbox->overwriteMode() == true);
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        t.sendPaste("FGhi");
        CHECK(inputbox->text() == "abcdeFGhifg");
    }

    SECTION("toggle-insert-AbCdEfG") {
        inputbox->setText("123");
        CHECK(inputbox->overwriteMode() == false);
        CHECK(inputbox->cursorStyle() == Tui::CursorStyle::Bar);
        t.sendKey(Tui::Key_Insert);
        CHECK(inputbox->cursorStyle() == Tui::CursorStyle::Block);
        CHECK(inputbox->overwriteMode() == true);
        t.sendKey(Tui::Key_Home);
        CHECK(inputbox->cursorPosition() == 0);
        t.sendChar("A");
        inputbox->setOverwriteMode(false);
        CHECK(inputbox->cursorStyle() == Tui::CursorStyle::Bar);
        CHECK(inputbox->cursorPosition() == 1);
        t.sendChar("b");
        inputbox->setOverwriteMode(true);
        CHECK(inputbox->cursorStyle() == Tui::CursorStyle::Block);
        CHECK(inputbox->cursorPosition() == 2);
        CHECK(inputbox->text() == "Ab23");
        t.sendChar("C");
        t.sendKey(Tui::Key_Insert);
        CHECK(inputbox->cursorPosition() == 3);
        t.sendChar("d");
        t.sendKey(Tui::Key_Insert);
        CHECK(inputbox->cursorPosition() == 4);
        CHECK(inputbox->text() == "AbCd3");
        t.sendChar("E");
        t.sendKey(Tui::Key_Insert);
        CHECK(inputbox->cursorPosition() == 5);
        t.sendChar("f");
        t.sendKey(Tui::Key_Insert);
        CHECK(inputbox->cursorPosition() == 6);
        t.sendChar("G");
        CHECK(inputbox->cursorPosition() == 7);
        CHECK(inputbox->text() == "AbCdEfG");
    }

    SECTION("backspace-abCDg") {
        inputbox->setText("abcdefg");
        CHECK(inputbox->overwriteMode() == false);
        t.sendKey(Tui::Key_Insert);
        CHECK(inputbox->overwriteMode() == true);
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        CHECK(inputbox->text().size() == 7);
        t.sendKey(Tui::Key_Backspace);
        t.sendKey(Tui::Key_Backspace);
        CHECK(inputbox->text().size() == 5);
        t.sendChar("C");
        t.sendChar("D");
        CHECK(inputbox->text().size() == 5);
        CHECK(inputbox->text() == "abCDg");
    }

    SECTION("surrogate-pairs-replace-ABCz") {
        inputbox->setText("😁😇😂z");
        CHECK(inputbox->overwriteMode() == false);
        t.sendKey(Tui::Key_Insert);
        CHECK(inputbox->overwriteMode() == true);
        t.sendKey(Tui::Key_Home);
        t.sendChar("A");
        t.sendChar("B");
        t.sendChar("C");
        CHECK(inputbox->text() == "ABCz");
    }
    SECTION("surrogate-pairs-paste-inserts-ABC") {
        inputbox->setText("😁😁😁");
        CHECK(inputbox->overwriteMode() == false);
        t.sendKey(Tui::Key_Insert);
        CHECK(inputbox->overwriteMode() == true);
        t.sendKey(Tui::Key_Home);
        t.sendPaste("ABC");
        CHECK(inputbox->text() == "ABC😁😁😁");
    }
    SECTION("surrogate-pairs-replace-clusters") {
        inputbox->setText("ABCD");
        CHECK(inputbox->overwriteMode() == false);
        t.sendKey(Tui::Key_Insert);
        CHECK(inputbox->overwriteMode() == true);
        t.sendKey(Tui::Key_Home);
        t.sendChar("😁");
        t.sendChar("😁");
        t.sendChar("😁");
        CHECK(inputbox->text() == "😁😁😁D");
    }
    SECTION("surrogate-pairs-replace-clusters") {
        inputbox->setText("ABCD");
        CHECK(inputbox->overwriteMode() == false);
        t.sendKey(Tui::Key_Insert);
        CHECK(inputbox->overwriteMode() == true);
        t.sendKey(Tui::Key_Home);
        t.sendPaste("😁😁😁");
        CHECK(inputbox->text() == "😁😁😁ABCD");
    }
    SECTION("CursorStyle") {
        CHECK(w->cursorStyle() == Tui::CursorStyle::Unset);
        CHECK(inputbox->cursorStyle() == Tui::CursorStyle::Bar);
        t.sendKey(Tui::Key_Insert);
        CHECK(w->cursorStyle() == Tui::CursorStyle::Unset);
        CHECK(inputbox->cursorStyle() == Tui::CursorStyle::Block);
        t.sendKey(Tui::Key_Insert);
        CHECK(w->cursorStyle() == Tui::CursorStyle::Unset);
        CHECK(inputbox->cursorStyle() == Tui::CursorStyle::Bar);

        inputbox->setOverwriteMode(true);
        CHECK(w->cursorStyle() == Tui::CursorStyle::Unset);
        CHECK(inputbox->cursorStyle() == Tui::CursorStyle::Block);
        inputbox->setOverwriteMode(false);
        CHECK(w->cursorStyle() == Tui::CursorStyle::Unset);
        CHECK(inputbox->cursorStyle() == Tui::CursorStyle::Bar);
    }

    SECTION("insertAtCursorPosition") {
        inputbox->setText("abcdefg");
        CHECK(inputbox->overwriteMode() == false);
        t.sendKey(Tui::Key_Insert);
        CHECK(inputbox->overwriteMode() == true);
        t.sendKey(Tui::Key_Home);
        inputbox->insertAtCursorPosition("AB");
        CHECK(inputbox->text() == "ABabcdefg");
    }

}
