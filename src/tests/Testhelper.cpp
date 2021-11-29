#include "Testhelper.h"

#include <termpaint_input.h>

#include <QFileInfo>
#include <QPoint>

#include <Tui/ZPainter.h>
#include <Tui/ZPalette.h>
#include <Tui/ZSymbol.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZTest.h>

static Tui::ZSymbol extendedCharset = TUISYM_LITERAL("extendedCharset");

Testhelper::Testhelper(QString dir, QString namePrefix, int width, int height, Options options)
    : namePrefix(dir + QStringLiteral("/") + namePrefix)
{
    static char prgname[] = "test";
    static char *argv[] = {prgname, nullptr};
    int argc = 1;
    app = std::make_unique<QCoreApplication>(argc, argv);

    auto init = Tui::ZTerminal::OffScreen{width, height};
    if (options & ReducedCharset) {
        init = init.withoutCapability(extendedCharset);
    }

    terminal = std::make_unique<Tui::ZTerminal>(init);
    root = new RootStub();
    root->setMinimumSize(0, 0);
    terminal->setMainWidget(root);
}

Testhelper::~Testhelper() {
    delete root;
}

void Testhelper::sendChar(QString ch, Qt::KeyboardModifiers modifiers) {
    Tui::ZTest::sendText(terminal.get(), ch, modifiers);
    render();
}

void Testhelper::sendKey(Qt::Key key, Qt::KeyboardModifiers modifiers) {
    Tui::ZTest::sendKey(terminal.get(), key, modifiers);
    render();
}

void Testhelper::sendKeyToWidget(Tui::ZWidget *w, Qt::Key key, Qt::KeyboardModifiers modifiers) {
    Tui::ZTest::sendKeyToWidget(w, key, modifiers);
    render();
}

static void nativeEvent(void *data, termpaint_event *event) {
    Tui::ZTerminal *terminal = static_cast<Tui::ZTerminal*>(data);
    Tui::ZTerminalNativeEvent tuiEvent{event};
    terminal->event(&tuiEvent);
}

void Testhelper::sendKeyToZTerminal(QString key) {
    std::string action = "";

    if(key.toLower() == QStringLiteral("↑")) {
        action = "\033[A";
    } else if(key.toLower() == QStringLiteral("↓")) {
        action = "\033[B";
    } else if(key.toLower() == QStringLiteral("→")) {
        action = "\033[C";
    } else if(key.toLower() == QStringLiteral("←")) {
        action = "\033[D";
    } else if(key.toLower() == QStringLiteral("f6")) {
        action = "\033[17~";
    } else if(key.toLower() == QStringLiteral("esc")) {
        action = "\033\033[0n";
    }

    if (action != "") {
        termpaint_input *input_ctx = termpaint_input_new();
        termpaint_input_set_event_cb(input_ctx, nativeEvent, terminal.get());
        termpaint_input_add_data(input_ctx, action.c_str(), action.size());
        termpaint_input_free(input_ctx);
    } else {
        FAIL("KEY NOT FOUND");
    }
    render();
}

std::vector<std::string> Testhelper::checkCharEventBubbles(QString ch, Qt::KeyboardModifiers modifiers) {
    std::vector<std::string> problems;
    auto fEventRestore = root->fEvent;

    bool keyBubbled = false;
    root->fEvent = [&keyBubbled, ch, modifiers](QEvent *event) {
        if (event->type() == Tui::ZEventType::key()) {
            auto keyEvent = static_cast<Tui::ZKeyEvent*>(event);
            if (keyEvent->text() == ch && keyEvent->modifiers() == modifiers) {
                CHECK(keyBubbled == false);
                keyBubbled = true;
            }
        }
    };
    sendChar(ch, modifiers);

    if (!keyBubbled) {
        problems.push_back("Key event did not bubble as expected");
    }

    root->fEvent = fEventRestore;

    return problems;
}

std::vector<std::string> Testhelper::checkKeyEventBubbles(Qt::Key key, Qt::KeyboardModifiers modifiers) {
    std::vector<std::string> problems;
    auto fEventRestore = root->fEvent;

    bool keyBubbled = false;
    root->fEvent = [&keyBubbled, key, modifiers](QEvent *event) {
        if (event->type() == Tui::ZEventType::key()) {
            auto keyEvent = static_cast<Tui::ZKeyEvent*>(event);
            if (keyEvent->key() == key && keyEvent->modifiers() == modifiers) {
                CHECK(keyBubbled == false);
                keyBubbled = true;
            }
        }
    };
    sendKey(key, modifiers);

    if (!keyBubbled) {
        problems.push_back("Key event did not bubble as expected");
    }

    root->fEvent = fEventRestore;

    return problems;
}

void Testhelper::compare(QString name) {
    terminal->update();
    Tui::ZImage actual = Tui::ZTest::waitForNextRenderAndGetContents(terminal.get());
    compare(name, actual);
}

void Testhelper::compare(Tui::ZImage actual) {
    std::string name;
    std::vector<std::string> names = getCurrentTestNames();
    for (size_t i = 1; i < names.size(); i++) {
        if (name.size()) {
            name += "-";
        }
        name += names[i];
    }

    compare(QString::fromStdString(name), actual);
}

void Testhelper::compare(QString name, Tui::ZImage actual) {
    QString fileNameOfTest = QString(basePath() + namePrefix + QStringLiteral("-") + name)
            .replace(QStringLiteral(" "), QStringLiteral("-"));

    lastCapture = std::make_unique<Tui::ZImage>(actual);
    QFileInfo fi(fileNameOfTest + QStringLiteral(".tpi"));
    if (!fi.exists()) {
        actual.save(fileNameOfTest + QStringLiteral("-new.tpi"));
        FAIL((QStringLiteral("image missing: ") + fileNameOfTest).toUtf8().toStdString());
    } else {
        std::unique_ptr<Tui::ZImage> expected = Tui::ZImage::fromFile(terminal.get(), fileNameOfTest + QStringLiteral(".tpi"));
        REQUIRE(expected != nullptr);
        if (actual != *expected) {
            actual.save(fileNameOfTest + QStringLiteral("-failed.tpi"));
            FAIL((QStringLiteral("diff image: ") + fileNameOfTest).toUtf8().toStdString());
        }
    }
}

void Testhelper::crossCheckWithMask(std::vector<std::string> overrideNames,
                                      std::vector<QPoint> ignore) {
    if (!lastCapture) {
        FAIL("cross check without available last capture");
    }
    std::string name;
    std::vector<std::string> names = getCurrentTestNames();
    if (names.size() < overrideNames.size()) {
        FAIL("override name list to long");
    }
    for (size_t i = 0; i < overrideNames.size(); i++) {
        names.pop_back();
    }
    names.insert(names.end(), overrideNames.begin(), overrideNames.end());
    for (size_t i = 1; i < names.size(); i++) {
        if (name.size()) {
            name += "-";
        }
        name += names[i];
    }

    QString fileNameOfTest = QString(basePath() + namePrefix + "-" + QString::fromStdString(name)).replace(' ', '-');
    std::unique_ptr<Tui::ZImage> expected = Tui::ZImage::fromFile(terminal.get(), fileNameOfTest + ".tpi");
    REQUIRE(expected != nullptr);
    Tui::ZImage actual = *lastCapture;
    Tui::ZPainter expectedPainter = expected->painter();
    Tui::ZPainter actualPainter = actual.painter();
    for (const auto &item : ignore) {
        expectedPainter.clearRect(item.x(), item.y(), 1, 1, Tui::ZColor::defaultColor(), Tui::ZColor::defaultColor());
        actualPainter.clearRect(item.x(), item.y(), 1, 1, Tui::ZColor::defaultColor(), Tui::ZColor::defaultColor());
    }
    if (actual != *expected) {
        FAIL(("cross check failed: " + fileNameOfTest).toUtf8().toStdString());
    }
}

void Testhelper::compare() {
    terminal->update();
    Tui::ZImage actual = Tui::ZTest::waitForNextRenderAndGetContents(terminal.get());
    compare(actual);
}

void Testhelper::render() {
    terminal->forceRepaint();
}

QString Testhelper::basePath() {
    QString path = QStringLiteral("tests/");
    if (getenv("TUIWIDGETS_TEST_DATA")) {
        path = QString::fromUtf8(getenv("TUIWIDGETS_TEST_DATA"));
    }
    if (!path.endsWith("/")) {
        path += "/";
    }
    return path;
}

thread_local QStringList DiagnosticMessageChecker::messages;
thread_local QtMessageHandler DiagnosticMessageChecker::oldMessageHandler;
thread_local bool DiagnosticMessageChecker::active = false;

DiagnosticMessageChecker::DiagnosticMessageChecker() {
    if (active) {
        FAIL("DiagnosticMessageChecker can only be active once per thread.");
    }
    active = true;
    oldMessageHandler = qInstallMessageHandler(qtMessageOutput);
}

DiagnosticMessageChecker::~DiagnosticMessageChecker() {
    qInstallMessageHandler(oldMessageHandler);
    active = false;
}

void DiagnosticMessageChecker::expectMessage(const QString &msg) {
    messages.append(msg);
}

void DiagnosticMessageChecker::tillHere() {
    while (messages.size()) {
        FAIL_CHECK("Expected diagnostic message not seen: " + messages.front().toStdString());
        messages.pop_front();
    }
}

void DiagnosticMessageChecker::qtMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    (void)type; (void)context;
    if (messages.front() == msg) {
        messages.pop_front();
    } else {
        FAIL_CHECK("Unexpected diagnostic message: " + msg.toStdString());
    }
}

void TestBackground::paintEvent(Tui::ZPaintEvent *event) {
    Tui::ZWindow::paintEvent(event);
    auto *painter = event->painter();
    int w = geometry().width();
    int h = geometry().height();
    int startX = 0;
    int startY = 0;
    auto border = borderEdges();
    if (border & Qt::TopEdge) {
        startY = 1;
    }
    if (border & Qt::BottomEdge) {
        h -= 1;
    }
    if (border & Qt::LeftEdge) {
        startX = 1;
    }
    if (border & Qt::RightEdge) {
        w -= 1;
    }
    for(int i = startY; i < h; i++) {
        painter->writeWithColors(startX, i, QString("␥").repeated(w - startX), getColor("window.frame.focused.fg"), getColor("window.frame.focused.bg"));
    }
}

std::vector<std::string> checkWidgetsDefaultsExcept(const Tui::ZWidget *w, DefaultExceptions exceptions) {
    std::vector<std::string> errors;

    if (!exceptions.testFlag(DefaultException::Enabled) && !w->isLocallyEnabled()) {
        errors.push_back("is not enabled");
    }

    if (!exceptions.testFlag(DefaultException::Visible) && !w->isLocallyVisible()) {
        errors.push_back("is not visible");
    }

    if (!exceptions.testFlag(DefaultException::MinimumSize) && w->minimumSize() != QSize()) {
        errors.push_back("has minimum size");
    }

    if (!exceptions.testFlag(DefaultException::MaximumSize) && w->maximumSize() != QSize{Tui::tuiMaxSize, Tui::tuiMaxSize}) {
        errors.push_back("has maximum size");
    }

    if (!exceptions.testFlag(DefaultException::SizePolicyH) && w->sizePolicyH() != Tui::SizePolicy::Preferred) {
        errors.push_back("sizePolicyH() != Tui::SizePolicy::Preferred");
    }

    if (!exceptions.testFlag(DefaultException::SizePolicyV) && w->sizePolicyV() != Tui::SizePolicy::Preferred) {
        errors.push_back("sizePolicyV() != Tui::SizePolicy::Preferred");
    }

    if (!exceptions.testFlag(DefaultException::Layout) && w->layout()) {
        errors.push_back("has layout");
    }

    if (!exceptions.testFlag(DefaultException::FocusPolicy) && w->focusPolicy() != Qt::NoFocus) {
        errors.push_back("focusPolicy() != NoFocus");
    }

    if (!exceptions.testFlag(DefaultException::FocusMode) && w->focusMode() != Tui::FocusContainerMode::None) {
        errors.push_back("focusMode() != None");
    }

    if (!exceptions.testFlag(DefaultException::FocusOrder) && w->focusOrder() != 0) {
        errors.push_back("focusOrder() != 0");
    }

    if (!exceptions.testFlag(DefaultException::ContentsMargins) && !w->contentsMargins().isNull()) {
        errors.push_back("contentsMargins() set");
    }

    if (!exceptions.testFlag(DefaultException::Palette) && !w->palette().isNull()) {
        errors.push_back("palette() is not null");
    }

    if (!exceptions.testFlag(DefaultException::PaletteClass) && w->paletteClass().size()) {
        errors.push_back("paletteClass() not empty");
    }

    if (!exceptions.testFlag(DefaultException::CursorStyle) && w->cursorStyle() != Tui::CursorStyle::Unset) {
        errors.push_back("cursorStyle() != Unset");
    }

    if (!exceptions.testFlag(DefaultException::Focus) && w->focus()) {
        errors.push_back("has focus");
    }

    if (!exceptions.testFlag(DefaultException::StackingLayer) && w->stackingLayer() != 0) {
        errors.push_back("stackingLayer() != 0");
    }

    return errors;
}
