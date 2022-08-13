// SPDX-License-Identifier: BSL-1.0

#include "ZInputBox.h"
#include "ZInputBox_p.h"

#include <QTextBoundaryFinder>

#include "ZPainter.h"
#include "ZTerminal.h"
#include "ZTextLayout.h"
#include "ZTextMetrics.h"

TUIWIDGETS_NS_START

static const QChar escapedNewLine = (QChar)(0xdc00 + (unsigned char)'\n');

static QString escapeNewLine(const QString &in) {
    QString out = in;
    out.replace(u'\n', escapedNewLine);
    return out;
}

static QString unescapeNewLine(const QString &in) {
    QString out = in;
    out.replace(escapedNewLine, u'\n');
    return out;
}

ZInputBoxPrivate::ZInputBoxPrivate(ZWidget *pub) : ZWidgetPrivate(pub) {
}

ZInputBoxPrivate::~ZInputBoxPrivate() {
}

ZInputBox::ZInputBox(ZWidget *parent) : ZWidget(parent, std::make_unique<ZInputBoxPrivate>(this)) {
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicyV(SizePolicy::Fixed);
    setSizePolicyH(SizePolicy::Expanding);
}

ZInputBox::ZInputBox(const QString &contents, ZWidget *parent) : ZInputBox(parent) {
    setText(contents);
}

ZInputBox::~ZInputBox() {
}

QString ZInputBox::text() const {
    auto *const p = tuiwidgets_impl();
    return unescapeNewLine(p->text);
}

void ZInputBox::setEchoMode(ZInputBox::EchoMode echoMode) {
    auto *const p = tuiwidgets_impl();
    p->echoMode = echoMode;
    update();
}

ZInputBox::EchoMode ZInputBox::echoMode() const {
    auto *const p = tuiwidgets_impl();
    return p->echoMode;
}

int ZInputBox::cursorPosition() const {
    auto *const p = tuiwidgets_impl();
    return p->cursorPosition;
}

void ZInputBox::setCursorPosition(int pos) {
    auto *const p = tuiwidgets_impl();

    if (pos <= 0) {
        p->cursorPosition = 0;
    } else {
        if (terminal()) {
            ZTextLayout textLayout = p->getTextLayout();
            p->cursorPosition = textLayout.nextCursorPosition(textLayout.previousCursorPosition(pos));
        } else {
            // This of course does not necessarily match what a later connected terminal actually does, but better than
            // nothing.
            QTextBoundaryFinder bnd(QTextBoundaryFinder::Grapheme, p->text);
            bnd.setPosition(pos);
            bnd.toPreviousBoundary();
            bnd.toNextBoundary();
            p->cursorPosition = std::clamp(bnd.position(), 0, p->text.size());
        }
    }
    update();
}

void ZInputBox::insertAtCursorPosition(const QString &text) {
    auto *const p = tuiwidgets_impl();

    const QString internalText = escapeNewLine(text);
    p->text.insert(p->cursorPosition, internalText);
    setCursorPosition(p->cursorPosition + internalText.size());
    p->adjustScrollPosition();
    textChanged(unescapeNewLine(p->text));
    update();
}

void ZInputBox::setText(const QString &text) {
    auto *const p = tuiwidgets_impl();

    const QString internalText = escapeNewLine(text);
    if (p->text == internalText) return;
    p->text = internalText;
    setCursorPosition(p->text.size());
    p->adjustScrollPosition();
    textChanged(unescapeNewLine(p->text));
    update();
}

QSize ZInputBox::sizeHint() const {
    auto cm = contentsMargins();
    return { 10 + cm.left() + cm.right(), 1 + cm.top() + cm.bottom() };
}

void ZInputBox::paintEvent(ZPaintEvent *event) {
    auto *const p = tuiwidgets_impl();

    QRect r = contentsRect();
    ZPainter painter = event->painter()->translateAndClip(r.left(), r.top(), r.width(), r.height());
    ZTextLayout textlayout = p->getTextLayoutForPaint();

    ZColor bg;
    ZColor fg;
    if (focus()) {
        bg = getColor("lineedit.focused.bg");
        fg = getColor("lineedit.focused.fg");
    } else if (!isEnabled()) {
        bg = getColor("lineedit.disabled.bg");
        fg = getColor("lineedit.disabled.fg");
    } else {
        bg = getColor("lineedit.bg");
        fg = getColor("lineedit.fg");
    }

    painter.clear(fg, bg);
    if (focus()) {
        textlayout.draw(painter, {-p->scrollPosition, 0}, {fg, bg});
        textlayout.showCursor(painter, {-p->scrollPosition, 0}, p->cursorPosition);
    } else {
        textlayout.draw(painter, {0, 0}, {fg, bg});
    }
    if (p->echoMode == NoEcho) {
        painter.setCursor(0, 0);
    }
}

void ZInputBox::keyEvent(ZKeyEvent *event) {
    auto *const p = tuiwidgets_impl();

    QString text = event->text();

    ZTextLayout textlayout = p->getTextLayout();

    if (event->key() == Qt::Key_Space && event->modifiers() == 0) {
        insertAtCursorPosition(QStringLiteral(" "));
    } else if (text.size() && event->modifiers() == 0) {
        insertAtCursorPosition(text);
    } else if (event->key() == Qt::Key_Backspace && event->modifiers() == 0) {
        if (p->cursorPosition > 0) {
            int cpleft = textlayout.previousCursorPosition(p->cursorPosition);
            p->text.remove(cpleft, p->cursorPosition - cpleft);
            setCursorPosition(cpleft);
            p->adjustScrollPosition();
            textChanged(unescapeNewLine(p->text));
            update();
        }
    } else if (event->key() == Qt::Key_Delete && event->modifiers() == 0) {
        if(p->text.size() > 0) {
            int cpright = textlayout.nextCursorPosition(p->cursorPosition);
            p->text.remove(p->cursorPosition, cpright - p->cursorPosition);
            p->adjustScrollPosition();
            textChanged(unescapeNewLine(p->text));
            update();
        }
    } else if(event->key() == Qt::Key_Left && event->modifiers() == 0) {
        if (p->cursorPosition > 0) {
            setCursorPosition(textlayout.previousCursorPosition(p->cursorPosition));
            p->adjustScrollPosition();
            update();
        }
    } else if(event->key() == Qt::Key_Right && event->modifiers() == 0) {
        if (p->cursorPosition < p->text.size()) {
            setCursorPosition(textlayout.nextCursorPosition(p->cursorPosition));
            p->adjustScrollPosition();
            update();
        }
    } else if(event->key() == Qt::Key_Home && event->modifiers() == 0) {
        setCursorPosition(0);
        p->adjustScrollPosition();
        update();
    } else if(event->key() == Qt::Key_End && event->modifiers() == 0) {
        setCursorPosition(p->text.size());
        p->adjustScrollPosition();
        update();
    } else {
        ZWidget::keyEvent(event);
    }
}

void ZInputBox::pasteEvent(ZPasteEvent *event) {
    auto *const p = tuiwidgets_impl();

    const QString text = event->text();
    const QString internalText = escapeNewLine(text);
    p->text.insert(p->cursorPosition, internalText);
    setCursorPosition(p->cursorPosition + internalText.size());
    p->adjustScrollPosition();
    textChanged(unescapeNewLine(p->text));
    update();
}

void ZInputBox::resizeEvent(ZResizeEvent *event) {
    auto *const p = tuiwidgets_impl();

    ZWidget::resizeEvent(event);
    p->adjustScrollPosition();
}

bool ZInputBox::event(QEvent *event) {
    auto *const p = tuiwidgets_impl();

    if (!parent()) {
        return ZWidget::event(event);
    }

    if (event->type() == ZEventType::otherChange()) {
        if (!static_cast<ZOtherChangeEvent*>(event)->unchanged().contains(TUISYM_LITERAL("terminal"))) {
            // make sure cursor position is on cluster boundary
            setCursorPosition(cursorPosition());

            p->adjustScrollPosition();
        }
    }

    return ZWidget::event(event);
}

bool ZInputBox::eventFilter(QObject *watched, QEvent *event) {
    return ZWidget::eventFilter(watched, event);
}

QSize ZInputBox::minimumSizeHint() const {
    return ZWidget::minimumSizeHint();
}

QRect ZInputBox::layoutArea() const {
    return ZWidget::layoutArea();
}

QObject *ZInputBox::facet(const QMetaObject &metaObject) const {
    return ZWidget::facet(metaObject);
}

ZWidget *ZInputBox::resolveSizeHintChain() {
    return ZWidget::resolveSizeHintChain();
}

void ZInputBox::timerEvent(QTimerEvent *event) {
    return ZWidget::timerEvent(event);
}

void ZInputBox::childEvent(QChildEvent *event) {
    return ZWidget::childEvent(event);
}

void ZInputBox::customEvent(QEvent *event) {
    return ZWidget::customEvent(event);
}

void ZInputBox::connectNotify(const QMetaMethod &signal) {
    return ZWidget::connectNotify(signal);
}

void ZInputBox::disconnectNotify(const QMetaMethod &signal) {
    return ZWidget::disconnectNotify(signal);
}
void ZInputBox::focusInEvent(ZFocusEvent *event) {
    return ZWidget::focusInEvent(event);
}

void ZInputBox::focusOutEvent(ZFocusEvent *event) {
    return ZWidget::focusOutEvent(event);
}

void ZInputBox::moveEvent(ZMoveEvent *event) {
    return ZWidget::moveEvent(event);
}


void ZInputBoxPrivate::adjustScrollPosition() {
    if (!pub()->terminal()) {
        return;
    }
    if (pub()->geometry().width() == 0) {
        return;
    }
    ZTextLayout textlayout = getTextLayout();
    const int cursor = textlayout.lineAt(0).cursorToX(cursorPosition, ZTextLayout::Leading);
    const int cursorTrailing = textlayout.lineAt(0).cursorToX(cursorPosition, ZTextLayout::Trailing);
    const int cursorWidth = std::max(1, cursorTrailing - cursor);

    if (cursor + cursorWidth - scrollPosition > pub()->geometry().width()) {
         scrollPosition = cursor - pub()->geometry().width() + cursorWidth;
    }
    if (textlayout.lineAt(0).width() - scrollPosition < pub()->geometry().width() - 1) {
         scrollPosition = std::max(0, textlayout.lineAt(0).width() - pub()->geometry().width() + 1);
    }
    if (cursor > 0) {
        const int beforeCursor = textlayout.lineAt(0).cursorToX(textlayout.previousCursorPosition(cursorPosition),
                                                                ZTextLayout::Leading);
        if (cursor - scrollPosition < cursor - beforeCursor) {
            scrollPosition = beforeCursor;
        }
    } else {
        scrollPosition = 0;
    }
}

ZTextLayout ZInputBoxPrivate::getTextLayout() {
    ZTextLayout textlayout = ZTextLayout(pub()->terminal()->textMetrics(), text);
    textlayout.doLayout(65000);
    return textlayout;
}

ZTextLayout ZInputBoxPrivate::getTextLayoutForPaint() {
    QString adjustedText;
    if (echoMode == ZInputBox::Password) {
        adjustedText = QStringLiteral("*").repeated(pub()->terminal()->textMetrics().sizeInClusters(text));
    } else if (echoMode == ZInputBox::NoEcho) {
        adjustedText = QStringLiteral("");
    } else {
        adjustedText = text;
    }
    ZTextLayout textlayout = ZTextLayout(pub()->terminal()->textMetrics(), adjustedText);
    textlayout.doLayout(65000);
    return textlayout;
}

TUIWIDGETS_NS_END
