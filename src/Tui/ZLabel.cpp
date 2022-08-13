// SPDX-License-Identifier: BSL-1.0

#include "ZLabel.h"
#include "ZLabel_p.h"

#include <Tui/ZShortcut.h>
#include <Tui/ZTerminal.h>

TUIWIDGETS_NS_START

ZLabel::ZLabel(ZWidget *parent) : ZWidget(parent, std::make_unique<ZLabelPrivate>(this)) {
    setSizePolicyV(SizePolicy::Fixed);
    setSizePolicyH(SizePolicy::Preferred);
}

ZLabel::ZLabel(const QString &text, ZWidget *parent) : ZLabel(parent) {
    setText(text);
}

ZLabel::ZLabel(WithMarkupTag, const QString &markup, ZWidget *parent) : ZLabel(parent) {
    setMarkup(markup);
}

ZLabel::~ZLabel() = default;

QString ZLabel::text() const {
    auto *const p = tuiwidgets_impl();
    return p->styledText.text();
}

void ZLabel::setText(const QString &text) {
    auto *const p = tuiwidgets_impl();
    p->styledText.setText(text);
    removeShortcut();
    update();
}

QString ZLabel::markup() const {
    auto *const p = tuiwidgets_impl();
    return p->styledText.markup();
}

void ZLabel::removeShortcut() {
    for (ZShortcut *s : findChildren<ZShortcut*>(QString(), Qt::FindDirectChildrenOnly)) {
        delete s;
    }
}

void ZLabel::setMarkup(const QString &markup) {
    auto *const p = tuiwidgets_impl();
    p->styledText.setMarkup(markup);
    if (p->styledText.mnemonic().size()) {
        removeShortcut();
        ZShortcut *s = new ZShortcut(ZKeySequence::forMnemonic(p->styledText.mnemonic()), this);
        connect(s, &ZShortcut::activated, this, [this] {
            auto *p = tuiwidgets_impl();
            if (p->buddy) {
                p->buddy->setFocus(Qt::ShortcutFocusReason);
            }
        });
        s->setEnabledDelegate([this] {
            auto *p = tuiwidgets_impl();
            return isEnabled() && (p->buddy == nullptr || p->buddy->isEnabled());
        });
    } else {
        removeShortcut();
    }
    update();
}

ZWidget *ZLabel::buddy() const {
    auto *const p = tuiwidgets_impl();
    return p->buddy;
}

void ZLabel::setBuddy(ZWidget *buddy) {
    auto *const p = tuiwidgets_impl();
    if (p->buddy == buddy) return;

    auto *term = terminal();
    if (term && (term->focusWidget() == p->buddy || term->focusWidget() == buddy)) {
        update();
    }
    p->buddy = buddy;
}

QSize ZLabel::sizeHint() const {
    auto *const p = tuiwidgets_impl();
    auto *term = terminal();
    if (!term) return {};
    auto cm = contentsMargins();
    QSize sh = { p->styledText.width(term->textMetrics()) + 1 + cm.left() + cm.right(),
                 1 + cm.top() + cm.bottom() };
    return sh;
}

void ZLabel::paintEvent(ZPaintEvent *event) {
    auto *const p = tuiwidgets_impl();
    ZTextStyle baseStyle;
    ZTextStyle shortcut;

    auto *painter = event->painter();
    QRect r = contentsRect();

    auto *term = terminal();
    bool buddyEnabled = true;
    if (p->buddy) {
        buddyEnabled = p->buddy->isEnabled();
    }
    if (isEnabled() && buddyEnabled) {
        if (term && term->focusWidget() == p->buddy && p->buddy) {
            baseStyle = {getColor("control.focused.fg"), getColor("control.focused.bg")};
            painter->writeWithColors(r.left(), r.top(), QStringLiteral("»"), baseStyle.foregroundColor(), baseStyle.backgroundColor());
        } else {
            baseStyle = {getColor("control.fg"), getColor("control.bg")};
        }
        shortcut = {getColor("control.shortcut.fg"), getColor("control.shortcut.bg")};
    } else {
        baseStyle = {getColor("control.disabled.fg"), getColor("control.disabled.bg")};
        shortcut = baseStyle;
    }

    p->styledText.setMnemonicStyle(baseStyle, shortcut);
    p->styledText.write(painter, r.left() + 1, r.top(), r.width() - 1);
}

void ZLabel::keyEvent(ZKeyEvent *event) {
    ZWidget::keyEvent(event);
}

bool ZLabel::event(QEvent *event) {
    return ZWidget::event(event);
}

bool ZLabel::eventFilter(QObject *watched, QEvent *event) {
    return ZWidget::eventFilter(watched, event);
}

QSize ZLabel::minimumSizeHint() const {
    return ZWidget::minimumSizeHint();
}

QRect ZLabel::layoutArea() const {
    return ZWidget::layoutArea();
}

QObject *ZLabel::facet(const QMetaObject &metaObject) const {
    return ZWidget::facet(metaObject);
}

ZWidget *ZLabel::resolveSizeHintChain() {
    return ZWidget::resolveSizeHintChain();
}

void ZLabel::timerEvent(QTimerEvent *event) {
    return ZWidget::timerEvent(event);
}

void ZLabel::childEvent(QChildEvent *event) {
    return ZWidget::childEvent(event);
}

void ZLabel::customEvent(QEvent *event) {
    return ZWidget::customEvent(event);
}

void ZLabel::connectNotify(const QMetaMethod &signal) {
    return ZWidget::connectNotify(signal);
}

void ZLabel::disconnectNotify(const QMetaMethod &signal) {
    return ZWidget::disconnectNotify(signal);
}

void ZLabel::pasteEvent(ZPasteEvent *event) {
    return ZWidget::pasteEvent(event);
}

void ZLabel::focusInEvent(ZFocusEvent *event) {
    return ZWidget::focusInEvent(event);
}

void ZLabel::focusOutEvent(ZFocusEvent *event) {
    return ZWidget::focusOutEvent(event);
}

void ZLabel::resizeEvent(ZResizeEvent *event) {
    return ZWidget::resizeEvent(event);
}

void ZLabel::moveEvent(ZMoveEvent *event) {
    return ZWidget::moveEvent(event);
}

ZLabelPrivate::ZLabelPrivate(ZWidget *pub) : ZWidgetPrivate(pub) {
}

ZLabelPrivate::~ZLabelPrivate() {
}


TUIWIDGETS_NS_END
