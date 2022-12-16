// SPDX-License-Identifier: BSL-1.0

#include "ZRadioButton.h"
#include "ZRadioButton_p.h"

#include <Tui/ZShortcut.h>
#include <Tui/ZTerminal.h>

TUIWIDGETS_NS_START

ZRadioButton::ZRadioButton(ZWidget *parent) : ZWidget(parent, std::make_unique<ZRadioButtonPrivate>(this)) {
    setFocusPolicy(StrongFocus);
    setSizePolicyV(SizePolicy::Fixed);
    setSizePolicyH(SizePolicy::Expanding);
}

ZRadioButton::ZRadioButton(const QString &text, ZWidget *parent) : ZRadioButton(parent) {
    setText(text);
}

ZRadioButton::ZRadioButton(WithMarkupTag, const QString &markup, ZWidget *parent) : ZRadioButton(parent) {
    setMarkup(markup);
}

ZRadioButton::~ZRadioButton() = default;

QString ZRadioButton::text() const {
    auto *const p = tuiwidgets_impl();
    return p->styledText.text();
}

void ZRadioButton::setText(const QString &text) {
    auto *const p = tuiwidgets_impl();
    p->styledText.setText(text);
    removeShortcut();
    update();
}

QString ZRadioButton::markup() const {
    auto *const p = tuiwidgets_impl();
    return p->styledText.markup();
}

void ZRadioButton::setMarkup(const QString &markup) {
    auto *const p = tuiwidgets_impl();
    p->styledText.setMarkup(markup);
    if (p->styledText.mnemonic().size()) {
        setShortcut(ZKeySequence::forMnemonic(p->styledText.mnemonic()));
    } else {
        removeShortcut();
    }
    update();
}

bool ZRadioButton::checked() const {
    auto *const p = tuiwidgets_impl();
    return p->checked;
}

void ZRadioButton::setChecked(bool state) {
    auto *const p = tuiwidgets_impl();
    p->checked = state;
    update();
}

void ZRadioButton::removeShortcut() {
    for (ZShortcut *s : findChildren<ZShortcut*>(QString(), Qt::FindDirectChildrenOnly)) {
        delete s;
    }
}

void ZRadioButton::setShortcut(const ZKeySequence &key) {
    removeShortcut();
    ZShortcut *s = new ZShortcut(key, this);
    QObject::connect(s, &ZShortcut::activated, this, &ZRadioButton::click);
}

QSize ZRadioButton::sizeHint() const {
    auto *const p = tuiwidgets_impl();
    auto *term = terminal();
    if (!term) return {};

    auto cm = contentsMargins();
    QSize sh = { p->styledText.width(term->textMetrics()) + 5 + cm.left() + cm.right(),
             1 + cm.top() + cm.bottom() };
    return sh;
}

void ZRadioButton::click() {
    if (!isEnabled()) {
        return;
    }

    setFocus();
    toggle();
}

void ZRadioButton::toggle() {
    auto *const p = tuiwidgets_impl();
    if (!p->checked) {
        if (parent()) {
            for (ZRadioButton *rb : parent()->findChildren<ZRadioButton*>(QString(), Qt::FindDirectChildrenOnly)) {
                if (rb != this) {
                    rb->tuiwidgets_impl()->checked = false;
                    rb->toggled(false);
                }
            }
        }
        p->checked = true;
        toggled(p->checked);
        update();
    }
}

void ZRadioButton::paintEvent(ZPaintEvent *event) {
    auto *const p = tuiwidgets_impl();
    auto cm = contentsMargins();
    QString focusIndicator = QStringLiteral(" ");
    ZTextStyle baseStyle;
    ZTextStyle shortcut;

    if (isEnabled()) {
        if (focus()) {
            focusIndicator = QStringLiteral("»");
            showCursor({cm.left() + 2, cm.top()});
            baseStyle = {getColor("control.focused.fg"), getColor("control.focused.bg")};
        } else {
            baseStyle = {getColor("control.fg"), getColor("control.bg")};
        }
        shortcut = {getColor("control.shortcut.fg"), getColor("control.shortcut.bg")};
    } else {
        baseStyle = {getColor("control.disabled.fg"), getColor("control.disabled.bg")};
        shortcut = baseStyle;
    }
    ZPainter *painter = event->painter();

    if (p->checked) {
        painter->writeWithColors(cm.left(), cm.top(), focusIndicator + QStringLiteral("(•) "), baseStyle.foregroundColor(), baseStyle.backgroundColor());
    } else {
        painter->writeWithColors(cm.left(), cm.top(), focusIndicator + QStringLiteral("( ) "), baseStyle.foregroundColor(), baseStyle.backgroundColor());
    }
    p->styledText.setMnemonicStyle(baseStyle, shortcut);
    p->styledText.write(painter, cm.left() + 5, cm.top(), geometry().width() - cm.left() - 4);
}

void ZRadioButton::keyEvent(ZKeyEvent *event) {
    if (event->key() == Key_Space && event->modifiers() == 0) {
        click();
    } else {
        ZWidget::keyEvent(event);
    }
}

bool ZRadioButton::event(QEvent *event) {
    return ZWidget::event(event);
}

bool ZRadioButton::eventFilter(QObject *watched, QEvent *event) {
    return ZWidget::eventFilter(watched, event);
}

QSize ZRadioButton::minimumSizeHint() const {
    return ZWidget::minimumSizeHint();
}

QRect ZRadioButton::layoutArea() const {
    return ZWidget::layoutArea();
}

QObject *ZRadioButton::facet(const QMetaObject &metaObject) const {
    return ZWidget::facet(metaObject);
}

ZWidget *ZRadioButton::resolveSizeHintChain() {
    return ZWidget::resolveSizeHintChain();
}

void ZRadioButton::timerEvent(QTimerEvent *event) {
    return ZWidget::timerEvent(event);
}

void ZRadioButton::childEvent(QChildEvent *event) {
    return ZWidget::childEvent(event);
}

void ZRadioButton::customEvent(QEvent *event) {
    return ZWidget::customEvent(event);
}

void ZRadioButton::connectNotify(const QMetaMethod &signal) {
    return ZWidget::connectNotify(signal);
}

void ZRadioButton::disconnectNotify(const QMetaMethod &signal) {
    return ZWidget::disconnectNotify(signal);
}

void ZRadioButton::pasteEvent(ZPasteEvent *event) {
    return ZWidget::pasteEvent(event);
}

void ZRadioButton::focusInEvent(ZFocusEvent *event) {
    return ZWidget::focusInEvent(event);
}

void ZRadioButton::focusOutEvent(ZFocusEvent *event) {
    return ZWidget::focusOutEvent(event);
}

void ZRadioButton::resizeEvent(ZResizeEvent *event) {
    return ZWidget::resizeEvent(event);
}

void ZRadioButton::moveEvent(ZMoveEvent *event) {
    return ZWidget::moveEvent(event);
}

ZRadioButtonPrivate::ZRadioButtonPrivate(ZWidget *pub) : ZWidgetPrivate(pub)  {}

ZRadioButtonPrivate::~ZRadioButtonPrivate() {
}

TUIWIDGETS_NS_END
