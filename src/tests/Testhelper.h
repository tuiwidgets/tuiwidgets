// SPDX-License-Identifier: BSL-1.0

#ifndef TESTHELPER_H
#define TESTHELPER_H

#include <memory>

#include <QCoreApplication>

#include <Tui/ZColor.h>
#include <Tui/ZImage.h>
#include <Tui/ZLayout.h>
#include <Tui/ZPainter.h>
#include <Tui/ZRoot.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZWindow.h>
#include <Tui/ZWindowFacet.h>

#include "catchwrapper.h"

std::vector<std::string> getCurrentTestNames();

class RootStub;

class Testhelper : public QObject {
public:
    enum Option {
        ReducedCharset = (1 << 0),
    };

    Q_DECLARE_FLAGS(Options, Option)

public:
    Testhelper(QString dir, QString namePrefix, int width, int height, Options options = {});
    ~Testhelper();

    void sendChar(QString ch, Qt::KeyboardModifiers modifiers = {});
    void sendKey(Qt::Key key, Qt::KeyboardModifiers modifiers = {});
    void sendKeyToWidget(Tui::ZWidget *w, Qt::Key key, Qt::KeyboardModifiers modifiers = {});
    void sendKeyToZTerminal(QString key);
    void sendPaste(const QString &text);

    [[nodiscard]]
    std::vector<std::string> checkCharEventBubbles(QString ch, Qt::KeyboardModifiers modifiers = {});
    [[nodiscard]]
    std::vector<std::string> checkKeyEventBubbles(Qt::Key key, Qt::KeyboardModifiers modifiers = {});
    [[nodiscard]]
    std::vector<std::string> checkPasteEventBubbles(QString str);
    [[nodiscard]]
    std::vector<std::string> checkKeyEventBubblesToParent(Qt::Key key, Qt::KeyboardModifiers modifiers = {});

    void compare();
    void compare(QString name);
    void compare(Tui::ZImage actual);
    void compare(QString name, Tui::ZImage actual);
    void crossCheckWithMask(std::vector<std::string> overrideNames, std::vector<QPoint> ignore);
    void render();

    QString namePrefix;

    std::unique_ptr<QCoreApplication> app;
    std::unique_ptr<Tui::ZTerminal> terminal;
    RootStub *root;

    std::unique_ptr<const Tui::ZImage> lastCapture;
    QString basePath();

};

Q_DECLARE_OPERATORS_FOR_FLAGS(Testhelper::Options)

class DiagnosticMessageChecker {
public:
    DiagnosticMessageChecker();
    ~DiagnosticMessageChecker();

public:
    void expectMessage(const QString &msg);
    void tillHere();

private:
    static void qtMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:
    static thread_local QStringList messages;
    static thread_local QtMessageHandler oldMessageHandler;
    static thread_local bool active;
};

class StubLayout : public Tui::ZLayout {
public:
    void setGeometry(QRect g) override { stubGeometry = g; };
    QSize sizeHint() const override { return stubSizeHint; };
    Tui::SizePolicy sizePolicyH() const override { return stubSizePolicyH; };
    Tui::SizePolicy sizePolicyV() const override { return stubSizePolicyV; };
    bool isVisible() const override { return stubIsVisible; };
    bool isSpacer() const override { return stubIsSpacer; };

    QSize stubSizeHint;
    Tui::SizePolicy stubSizePolicyH = Tui::SizePolicy::Expanding;
    Tui::SizePolicy stubSizePolicyV = Tui::SizePolicy::Expanding;
    bool stubIsVisible = true;
    bool stubIsSpacer = false;
    QRect stubGeometry;
};

class StubWidget : public Tui::ZWidget {
public:
    explicit StubWidget(ZWidget *parent = 0) : Tui::ZWidget(parent) {};

public:
    QSize sizeHint() const override {
        return stubSizeHint;
    }

    void paintEvent(Tui::ZPaintEvent *event) override {
        auto painter = event->painter();
        painter->clear(fg, bg);
    }

    QSize stubSizeHint;
    Tui::ZColor fg;
    Tui::ZColor bg;
};

class TestZWindowFacet : public Tui::ZWindowFacet {
public:
    bool isExtendViewport() const override {
        return extendViewport;
    }

    virtual void autoPlace(const QSize &available, Tui::ZWidget *self) override {
        geometry = available;
        autoPlaceSelf = self;
        autoPlaceCount += 1;
    }

    int autoPlaceCount = 0;
    QSize geometry;
    bool extendViewport = false;
    Tui::ZWidget *autoPlaceSelf = nullptr;
};

class TestBackground : public Tui::ZWindow {
public:
    explicit TestBackground(Tui::ZWidget *parent) : Tui::ZWindow(parent) {}

protected:
    void paintEvent(Tui::ZPaintEvent *event) override;
};

class RootStub : public Tui::ZRoot {
public:
    using Tui::ZRoot::ZRoot;

    std::function<void(QEvent*)> fEvent;
    bool terminalChangedTrigger = false;
    int raiseCount = 0;
    ZWidget *raiseWidget = nullptr;
    bool disableRaiseOnFocus = false;

protected:
    void terminalChanged() override {
        terminalChangedTrigger = true;
    }

    bool event(QEvent *event) override {
        if (fEvent) {
            fEvent(event);
        }
        return Tui::ZRoot::event(event);
    }

    void raiseOnActivate(ZWidget *w) override {
        raiseCount += 1;
        raiseWidget = w;
        if (!disableRaiseOnFocus) {
            Tui::ZRoot::raiseOnActivate(w);
        }
    }
};

enum class DefaultException {
    Enabled = 1 << 0,
    Visible = 1 << 1,
    MinimumSize = 1 << 2,
    MaximumSize = 1 << 3,
    SizePolicyH = 1 << 4,
    SizePolicyV = 1 << 5,
    Layout = 1 << 6,
    FocusPolicy = 1 << 7,
    FocusMode = 1 << 8,
    FocusOrder = 1 << 9,
    ContentsMargins = 1 << 10,
    Palette = 1 << 11,
    PaletteClass = 1 << 12,
    CursorStyle = 1 << 13,
    Focus = 1 << 14,
    StackingLayer = 1 << 15
};

Q_DECLARE_FLAGS(DefaultExceptions, DefaultException)
Q_DECLARE_OPERATORS_FOR_FLAGS(DefaultExceptions)

[[nodiscard]]
std::vector<std::string> checkWidgetsDefaultsExcept(const Tui::ZWidget *w, DefaultExceptions exceptions = {});

#define FAIL_CHECK_VEC(x) do { for (const auto& f : x) { FAIL_CHECK(f);} } while (0)

#endif // TESTHELPER_H
