// SPDX-License-Identifier: BSL-1.0

#include "ZTerminalDiagnosticsDialog.h"
#include "ZTerminalDiagnosticsDialog_p.h"

#include <Tui/Misc/SurrogateEscape.h>

#include <Tui/ZButton.h>
#include <Tui/ZHBoxLayout.h>
#include <Tui/ZInputBox.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZTerminal_p.h>
#include <Tui/ZTextMetrics.h>
#include <Tui/ZVBoxLayout.h>

#include <Tui/ZSymbol.h>

TUIWIDGETS_NS_START

namespace {

    class RawInputCapture : public QObject {
    public:
        RawInputCapture(ZTerminal &terminal_)
            : terminal(terminal_) {
            terminal.installEventFilter(this);
        }

        bool eventFilter(QObject *watched, QEvent *event) {
            (void)watched;
            if (event->type() == ZEventType::rawSequence()) {
                raw += static_cast<ZRawSequenceEvent*>(event)->sequence();
            }
            return false;
        }

        QByteArray raw;
        ZTerminal &terminal;
    };

}


ZTerminalDiagnosticsDialogPrivate::ZTerminalDiagnosticsDialogPrivate(ZWidget *pub) : ZDialogPrivate(pub) {
}

ZTerminalDiagnosticsDialogPrivate::~ZTerminalDiagnosticsDialogPrivate() {
}

ZTerminalDiagnosticsDialog::ZTerminalDiagnosticsDialog(ZWidget *parent) : ZDialog(parent, std::make_unique<ZTerminalDiagnosticsDialogPrivate>(this)) {
    auto *const p = tuiwidgets_impl();

    setOptions(ZWindow::CloseOption | ZWindow::MoveOption | ZWindow::AutomaticOption
                | ZWindow::DeleteOnClose);

    setWindowTitle(QStringLiteral("Terminal Diagnostics"));
    setContentsMargins({ 1, 1, 1, 1});

    ZVBoxLayout *layout = new ZVBoxLayout();
    setLayout(layout);

    ZTextLine *l1 = new ZTextLine(QStringLiteral("Terminal Information"), this);
    layout->addWidget(l1);

    p->terminalInfo = new ZInputBox(this);
    p->terminalInfo->setEnabled(false);
    layout->addWidget(p->terminalInfo);

    p->terminalCaps = new ZInputBox(this);
    p->terminalCaps->setEnabled(false);
    layout->addWidget(p->terminalCaps);

    ZTextLine *l2 = new ZTextLine(QStringLiteral("Terminal Self Id"), this);
    layout->addWidget(l2);

    p->terminalSelfId = new ZInputBox(this);
    p->terminalSelfId->setEnabled(false);
    layout->addWidget(p->terminalSelfId);

    if (terminal()) {
        p->updateInfo();
    }

    layout->addSpacing(1);

    p->keyHeader = new ZTextLine(this);
    layout->addWidget(p->keyHeader);

    p->keyRaw = new ZInputBox(this);
    p->keyRaw->setEnabled(false);

    // hide text line
    ZPalette rawPalette = p->keyRaw->palette();
    rawPalette.addRules({{{}, {
        { ZPalette::Type::Publish, TUISYM_LITERAL("lineedit.disabled.bg"), TUISYM_LITERAL("control.bg")},
        { ZPalette::Type::Publish, TUISYM_LITERAL("lineedit.disabled.fg"), TUISYM_LITERAL("control.fg")},
    }}});
    p->keyRaw->setPalette(rawPalette);

    layout->addWidget(p->keyRaw);

    p->keyParsed = new ZTextLine(this);
    layout->addWidget(p->keyParsed);

    layout->addSpacing(1);

    {
        ZHBoxLayout *hbox = new ZHBoxLayout();
        layout->add(hbox);
        hbox->addStretch();

        ZButton *keyBtn = new ZButton(QStringLiteral("Key Tester"), this);
        hbox->addWidget(keyBtn);
        QObject::connect(keyBtn, &ZButton::clicked, this, [this] {
            auto *const p = tuiwidgets_impl();
            p->keyboardTest();
        });

        ZButton *closeBtn = new ZButton(QStringLiteral("Close"), this);
        hbox->addWidget(closeBtn);
        QObject::connect(closeBtn, &ZButton::clicked, this, &ZDialog::reject);
        closeBtn->setFocus();
    }
}

ZTerminalDiagnosticsDialog::~ZTerminalDiagnosticsDialog() {
}


void ZTerminalDiagnosticsDialogPrivate::updateInfo() {
    ZWidget *const w = pub();

    ZTerminal const *terminal = w->terminal();
    auto *const terminalPriv = ZTerminalPrivate::get(terminal);

    terminalInfo->setText(terminal->terminalDetectionResultText());
    QString caps;
    if (terminalPriv->initState == ZTerminalPrivate::InitState::Ready
            || terminalPriv->initState == ZTerminalPrivate::InitState::Paused) {

        auto addCapability = [&](int id, const char *name) {
            if (termpaint_terminal_capable(terminalPriv->terminal, id)) {
                caps += QString::fromUtf8(name);
            } else {
                caps += QStringLiteral("-");
            }
        };
        addCapability(TERMPAINT_CAPABILITY_CSI_POSTFIX_MOD, "p");
        addCapability(TERMPAINT_CAPABILITY_TITLE_RESTORE, "t");
        addCapability(TERMPAINT_CAPABILITY_MAY_TRY_CURSOR_SHAPE_BAR, "b");
        addCapability(TERMPAINT_CAPABILITY_CURSOR_SHAPE_OSC50, "5");
        addCapability(TERMPAINT_CAPABILITY_EXTENDED_CHARSET, "c");
        addCapability(TERMPAINT_CAPABILITY_TRUECOLOR_MAYBE_SUPPORTED, "t");
        addCapability(TERMPAINT_CAPABILITY_TRUECOLOR_SUPPORTED, "T");
        addCapability(TERMPAINT_CAPABILITY_88_COLOR, "8");
        addCapability(TERMPAINT_CAPABILITY_CLEARED_COLORING, "C");
        addCapability(TERMPAINT_CAPABILITY_7BIT_ST, "7");
        addCapability(TERMPAINT_CAPABILITY_MAY_TRY_TAGGED_PASTE, "P");
    }
    terminalCaps->setText(QStringLiteral("Caps: ") + caps);
    terminalSelfId->setText(terminal->terminalSelfReportedNameAndVersion());

    ZTextMetrics metrics = terminal->textMetrics();

    const int minimumWidth = std::max(metrics.sizeInColumns(terminalInfo->text()),
                                      metrics.sizeInColumns(terminalSelfId->text())) + 6;
    w->setMinimumSize(minimumWidth, 0);

    const QRect geom = w->geometry();
    if (geom.width() && geom.width() < minimumWidth) {
        const int missingWidth = minimumWidth - geom.width();
        w->setGeometry(geom.adjusted(-(missingWidth + 1) / 2, 0, missingWidth / 2, 0));
    }
}

void ZTerminalDiagnosticsDialogPrivate::keyboardTest() {
    ZWidget *const w = pub();

    keyHeader->setText(QStringLiteral("Press key to test"));
    RawInputCapture *capture = new RawInputCapture(*w->terminal());
    w->grabKeyboard([this, capture](QEvent *event) {
        if (event->type() == ZEventType::key()) {
            ZWidget *const w = pub();

            auto *const keyEvent = static_cast<ZKeyEvent*>(event);
            keyHeader->setText(QStringLiteral("Key test result"));

            keyRaw->setText(Misc::SurrogateEscape::decode(capture->raw));

            // escape chars as displays as inverted, but default (disabled) line edit foreground matches window
            // background, switch to dataview palette to avoid this.
            // Also resetting the palette makes the keyRaw widget visible again.
            ZPalette rawPalette = keyRaw->palette();
            rawPalette.addRules({{{}, {
                { ZPalette::Type::Publish, TUISYM_LITERAL("lineedit.disabled.bg"), TUISYM_LITERAL("dataview.bg")},
                { ZPalette::Type::Publish, TUISYM_LITERAL("lineedit.disabled.fg"), TUISYM_LITERAL("dataview.fg")},
            }}});
            keyRaw->setPalette(rawPalette);

            QString parsed;

            if (keyEvent->text().size()) {
                parsed += QStringLiteral("Key:  ");
            } else {
                parsed += QStringLiteral("Text: ");
            }

            auto modifiers = keyEvent->modifiers();
            if (modifiers & ShiftModifier) {
                parsed += QStringLiteral("S");
                modifiers &= ~ShiftModifier;
            } else {
                parsed += QStringLiteral(" ");
            }

            if (modifiers & AltModifier) {
                parsed += QStringLiteral("A");
                modifiers &= ~AltModifier;
            } else {
                parsed += QStringLiteral(" ");
            }

            if (modifiers & ControlModifier) {
                parsed += QStringLiteral("C");
                modifiers &= ~ControlModifier;
            } else {
                parsed += QStringLiteral(" ");
            }

            if (modifiers & KeypadModifier) {
                parsed += QStringLiteral("K");
                modifiers &= ~KeypadModifier;
            } else {
                parsed += QStringLiteral(" ");
            }

            if (modifiers) {
                parsed += QStringLiteral("M") + QString::number(modifiers);
            }

            parsed += QStringLiteral(" ");

            if (keyEvent->text().size()) {
                parsed += keyEvent->text();
            } else {
                switch (keyEvent->key()) {
                case Key_PageUp:
                    parsed += QStringLiteral("PageUp");
                    break;
                case Key_PageDown:
                    parsed += QStringLiteral("PageDown");
                    break;
                case Key_Right:
                    parsed += QStringLiteral("Right");
                    break;
                case Key_Left:
                    parsed += QStringLiteral("Left");
                    break;
                case Key_Down:
                    parsed += QStringLiteral("Down");
                    break;
                case Key_Up:
                    parsed += QStringLiteral("Up");
                    break;
                case Key_Tab:
                    parsed += QStringLiteral("Tab");
                    break;
                case Key_Enter:
                    parsed += QStringLiteral("Enter");
                    break;
                case Key_Backspace:
                    parsed += QStringLiteral("Backspace");
                    break;
                case Key_Menu:
                    parsed += QStringLiteral("Menu");
                    break;
                case Key_Delete:
                    parsed += QStringLiteral("Delete");
                    break;
                case Key_Home:
                    parsed += QStringLiteral("Home");
                    break;
                case Key_Insert:
                    parsed += QStringLiteral("Insert");
                    break;
                case Key_End:
                    parsed += QStringLiteral("End");
                    break;
                case Key_Space:
                    parsed += QStringLiteral("Space");
                    break;
                case Key_Escape:
                    parsed += QStringLiteral("Escape");
                    break;
                case Key_F1:
                    parsed += QStringLiteral("F1");
                    break;
                case Key_F2:
                    parsed += QStringLiteral("F2");
                    break;
                case Key_F3:
                    parsed += QStringLiteral("F3");
                    break;
                case Key_F4:
                    parsed += QStringLiteral("F4");
                    break;
                case Key_F5:
                    parsed += QStringLiteral("F5");
                    break;
                case Key_F6:
                    parsed += QStringLiteral("F6");
                    break;
                case Key_F7:
                    parsed += QStringLiteral("F7");
                    break;
                case Key_F8:
                    parsed += QStringLiteral("F8");
                    break;
                case Key_F9:
                    parsed += QStringLiteral("F9");
                    break;
                case Key_F10:
                    parsed += QStringLiteral("F10");
                    break;
                case Key_F11:
                    parsed += QStringLiteral("F11");
                    break;
                case Key_F12:
                    parsed += QStringLiteral("F12");
                    break;
                case Key_division:
                    parsed += QStringLiteral("division");
                    break;
                case Key_multiply:
                    parsed += QStringLiteral("multiply");
                    break;
                case Key_Minus:
                    parsed += QStringLiteral("Minus");
                    break;
                case Key_Plus:
                    parsed += QStringLiteral("Plus");
                    break;
                case Key_Period:
                    parsed += QStringLiteral("Period");
                    break;
                case Key_0:
                    parsed += QStringLiteral("0");
                    break;
                case Key_1:
                    parsed += QStringLiteral("1");
                    break;
                case Key_2:
                    parsed += QStringLiteral("2");
                    break;
                case Key_3:
                    parsed += QStringLiteral("3");
                    break;
                case Key_4:
                    parsed += QStringLiteral("4");
                    break;
                case Key_5:
                    parsed += QStringLiteral("5");
                    break;
                case Key_6:
                    parsed += QStringLiteral("6");
                    break;
                case Key_7:
                    parsed += QStringLiteral("7");
                    break;
                case Key_8:
                    parsed += QStringLiteral("8");
                    break;
                case Key_9:
                    parsed += QStringLiteral("9");
                    break;
                default:
                    parsed += QStringLiteral("Key") + QString::number(keyEvent->key());
                }
            }

            keyParsed->setText(parsed);
            w->releaseKeyboard();
            capture->deleteLater();
        }
    });
}

bool ZTerminalDiagnosticsDialog::event(QEvent *event) {
    if (event->type() == ZEventType::terminalChange()) {
        if (terminal()) {
            auto *const p = tuiwidgets_impl();
            p->updateInfo();
        }
    }
    return ZDialog::event(event);
}

bool ZTerminalDiagnosticsDialog::eventFilter(QObject *watched, QEvent *event) {
    return ZDialog::eventFilter(watched, event);
}

QSize ZTerminalDiagnosticsDialog::sizeHint() const {
    return ZDialog::sizeHint();
}

QSize ZTerminalDiagnosticsDialog::minimumSizeHint() const {
    return ZDialog::minimumSizeHint();
}

QRect ZTerminalDiagnosticsDialog::layoutArea() const {
    return ZDialog::layoutArea();
}

QObject *ZTerminalDiagnosticsDialog::facet(const QMetaObject &metaObject) const {
    return ZDialog::facet(metaObject);
}

ZWidget *ZTerminalDiagnosticsDialog::resolveSizeHintChain() {
    return ZDialog::resolveSizeHintChain();
}

void ZTerminalDiagnosticsDialog::timerEvent(QTimerEvent *event) {
    return ZDialog::timerEvent(event);
}

void ZTerminalDiagnosticsDialog::childEvent(QChildEvent *event) {
    return ZDialog::childEvent(event);
}

void ZTerminalDiagnosticsDialog::customEvent(QEvent *event) {
    return ZDialog::customEvent(event);
}

void ZTerminalDiagnosticsDialog::connectNotify(const QMetaMethod &signal) {
    return ZDialog::connectNotify(signal);
}

void ZTerminalDiagnosticsDialog::disconnectNotify(const QMetaMethod &signal) {
    return ZDialog::disconnectNotify(signal);
}

void ZTerminalDiagnosticsDialog::paintEvent(ZPaintEvent *event) {
    return ZDialog::paintEvent(event);
}

void ZTerminalDiagnosticsDialog::keyEvent(ZKeyEvent *event) {
    return ZDialog::keyEvent(event);
}

void ZTerminalDiagnosticsDialog::pasteEvent(ZPasteEvent *event) {
    return ZDialog::pasteEvent(event);
}

void ZTerminalDiagnosticsDialog::focusInEvent(ZFocusEvent *event) {
    return ZDialog::focusInEvent(event);
}

void ZTerminalDiagnosticsDialog::focusOutEvent(ZFocusEvent *event) {
    return ZDialog::focusOutEvent(event);
}

void ZTerminalDiagnosticsDialog::resizeEvent(ZResizeEvent *event) {
    return ZDialog::resizeEvent(event);
}

void ZTerminalDiagnosticsDialog::moveEvent(ZMoveEvent *event) {
    return ZDialog::moveEvent(event);
}

QVector<ZMenuItem> ZTerminalDiagnosticsDialog::systemMenu() {
    return ZDialog::systemMenu();
}

void ZTerminalDiagnosticsDialog::closeEvent(ZCloseEvent *event) {
    ZDialog::closeEvent(event);
}



TUIWIDGETS_NS_END
