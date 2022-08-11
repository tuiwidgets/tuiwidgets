#include "ZTextLine.h"
#include "ZTextLine_p.h"

#include <Tui/ZTerminal.h>

TUIWIDGETS_NS_START


ZTextLine::ZTextLine(ZWidget *parent) : ZWidget(parent, std::make_unique<ZTextLinePrivate>(this)) {
    setSizePolicyV(SizePolicy::Fixed);
    setSizePolicyH(SizePolicy::Expanding);
}

ZTextLine::ZTextLine(const QString &text, ZWidget *parent) : ZTextLine(parent) {
    setText(text);
}

ZTextLine::ZTextLine(WithMarkupTag, const QString &markup, ZWidget *parent) : ZTextLine(parent) {
    setMarkup(markup);
}

ZTextLine::~ZTextLine() = default;

QString ZTextLine::text() const {
    auto *const p = tuiwidgets_impl();
    return p->styledText.text();
}

void ZTextLine::setText(const QString &text) {
    auto *const p = tuiwidgets_impl();
    p->styledText.setText(text);
    update();
}

QString ZTextLine::markup() const {
    auto *const p = tuiwidgets_impl();
    return p->styledText.markup();
}

void ZTextLine::setMarkup(const QString &markup) {
    auto *const p = tuiwidgets_impl();
    p->styledText.setMarkup(markup);
    update();
}

QSize ZTextLine::sizeHint() const {
    auto *const p = tuiwidgets_impl();
    auto *term = terminal();
    if (!term) return {};
    auto cm = contentsMargins();
    QSize sh = { p->styledText.width(term->textMetrics()) + cm.left() + cm.right(),
                 1 + cm.top() + cm.bottom() };
    return sh;
}

void ZTextLine::paintEvent(ZPaintEvent *event) {
    auto *const p = tuiwidgets_impl();
    ZTextStyle style = {getColor("control.fg"), getColor("control.bg")};

    auto *const painter = event->painter();
    painter->clear(style.foregroundColor(), style.backgroundColor());

    QRect r = contentsRect();
    p->styledText.setBaseStyle(style);
    p->styledText.write(painter, r.left(), r.top(), r.width());
}

bool ZTextLine::event(QEvent *event) {
    return ZWidget::event(event);
}

bool ZTextLine::eventFilter(QObject *watched, QEvent *event) {
    return ZWidget::eventFilter(watched, event);
}

QSize ZTextLine::minimumSizeHint() const {
    return ZWidget::minimumSizeHint();
}

QRect ZTextLine::layoutArea() const {
    return ZWidget::layoutArea();
}

QObject *ZTextLine::facet(const QMetaObject &metaObject) const {
    return ZWidget::facet(metaObject);
}

ZWidget *ZTextLine::resolveSizeHintChain() {
    return ZWidget::resolveSizeHintChain();
}

void ZTextLine::timerEvent(QTimerEvent *event) {
    return ZWidget::timerEvent(event);
}

void ZTextLine::childEvent(QChildEvent *event) {
    return ZWidget::childEvent(event);
}

void ZTextLine::customEvent(QEvent *event) {
    return ZWidget::customEvent(event);
}

void ZTextLine::connectNotify(const QMetaMethod &signal) {
    return ZWidget::connectNotify(signal);
}

void ZTextLine::disconnectNotify(const QMetaMethod &signal) {
    return ZWidget::disconnectNotify(signal);
}

void ZTextLine::keyEvent(ZKeyEvent *event) {
    return ZWidget::keyEvent(event);
}

void ZTextLine::pasteEvent(ZPasteEvent *event) {
    return ZWidget::pasteEvent(event);
}

void ZTextLine::focusInEvent(ZFocusEvent *event) {
    return ZWidget::focusInEvent(event);
}

void ZTextLine::focusOutEvent(ZFocusEvent *event) {
    return ZWidget::focusOutEvent(event);
}

void ZTextLine::resizeEvent(ZResizeEvent *event) {
    return ZWidget::resizeEvent(event);
}

void ZTextLine::moveEvent(ZMoveEvent *event) {
    return ZWidget::moveEvent(event);
}

ZTextLinePrivate::ZTextLinePrivate(ZWidget *pub) : ZWidgetPrivate (pub) {
}

ZTextLinePrivate::~ZTextLinePrivate() {
}

TUIWIDGETS_NS_END
