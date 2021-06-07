#include "Testhelper.h"

#include <termpaint_input.h>

#include <QFileInfo>
#include <QPoint>

#include <Tui/ZPainter.h>
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
    root = new Tui::ZRoot();
    root->setMinimumSize(0, 0);
    terminal->setMainWidget(root);
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


void Testhelper::compare(QString name) {
    QString fileNameOfTest = QString(basePath() + namePrefix + QStringLiteral("-") + name)
            .replace(QStringLiteral(" "), QStringLiteral("-"));
    terminal->update();
    Tui::ZImage actual = Tui::ZTest::waitForNextRenderAndGetContents(terminal.get());
    lastCapture = std::make_unique<Tui::ZImage>(actual);
    QFileInfo fi(fileNameOfTest + QStringLiteral(".tpi"));
    if (!fi.exists()) {
        actual.save(fileNameOfTest + QStringLiteral("-new.tpi"));
        FAIL((QStringLiteral("image missing: ") + fileNameOfTest).toUtf8().toStdString());
    } else {
        Tui::ZImage expected(terminal.get(), fileNameOfTest + QStringLiteral(".tpi"));
        if (actual != expected) {
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
    Tui::ZImage expected(terminal.get(), fileNameOfTest + ".tpi");
    Tui::ZImage actual = *lastCapture;
    Tui::ZPainter expectedPainter = expected.painter();
    Tui::ZPainter actualPainter = actual.painter();
    for (const auto &item : ignore) {
        expectedPainter.clearRect(item.x(), item.y(), 1, 1, Tui::ZColor::defaultColor(), Tui::ZColor::defaultColor());
        actualPainter.clearRect(item.x(), item.y(), 1, 1, Tui::ZColor::defaultColor(), Tui::ZColor::defaultColor());
    }
    if (actual != expected) {
        FAIL(("cross check failed: " + fileNameOfTest).toUtf8().toStdString());
    }
}

void Testhelper::compare() {
    std::string name;
    std::vector<std::string> names = getCurrentTestNames();
    for (size_t i = 1; i < names.size(); i++) {
        if (name.size()) {
            name += "-";
        }
        name += names[i];
    }

    compare(QString::fromStdString(name));
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
