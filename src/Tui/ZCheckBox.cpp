// SPDX-License-Identifier: BSL-1.0

#include "ZCheckBox.h"
#include "ZCheckBox_p.h"

#include <Tui/ZShortcut.h>
#include <Tui/ZTerminal.h>

TUIWIDGETS_NS_START

ZCheckBox::ZCheckBox(ZWidget *parent) : ZWidget(parent, std::make_unique<ZCheckBoxPrivate>(this)) {
    setFocusPolicy(Tui::StrongFocus);
    setSizePolicyV(SizePolicy::Fixed);
    setSizePolicyH(SizePolicy::Expanding);
}

ZCheckBox::ZCheckBox(const QString &text, ZWidget *parent) : ZCheckBox(parent) {
    setText(text);
}

ZCheckBox::ZCheckBox(WithMarkupTag, const QString &markup, ZWidget *parent) : ZCheckBox(parent) {
    setMarkup(markup);
}

ZCheckBox::~ZCheckBox() = default;

QString ZCheckBox::text() const {
    auto *const p = tuiwidgets_impl();
    return p->styledText.text();
}

void ZCheckBox::setText(const QString &text) {
    auto *const p = tuiwidgets_impl();
    p->styledText.setText(text);
    removeShortcut();
    update();
}

QString ZCheckBox::markup() const {
    auto *const p = tuiwidgets_impl();
    return p->styledText.markup();
}

void ZCheckBox::setMarkup(const QString &markup) {
    auto *const p = tuiwidgets_impl();
    p->styledText.setMarkup(markup);
    if (p->styledText.mnemonic().size()) {
        setShortcut(ZKeySequence::forMnemonic(p->styledText.mnemonic()));
    } else {
        removeShortcut();
    }
    update();
}

void ZCheckBox::setCheckState(Tui::CheckState state) {
    auto *const p = tuiwidgets_impl();
    p->state = state;
    update();
}

Tui::CheckState ZCheckBox::checkState() const {
    auto *const p = tuiwidgets_impl();
    return p->state;
}

bool ZCheckBox::isTristate() const {
    auto *const p = tuiwidgets_impl();
    return p->tristate;
}

void ZCheckBox::setTristate(bool tristate) {
    auto *const p = tuiwidgets_impl();
    p->tristate = tristate;
}

void ZCheckBox::paintEvent(ZPaintEvent *event) {
    auto *const p = tuiwidgets_impl();
    auto cm = contentsMargins();
    QString focusIndicator = QStringLiteral(" ");
    ZTextStyle baseStyle;
    ZTextStyle shortcut;

    if (isEnabled()) {
        if (focus()) {
            focusIndicator = QStringLiteral("»");
            showCursor({cm.left() + 2, cm.top() + 0});
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

    if (p->state == Tui::Checked) {
        painter->writeWithColors(cm.left(), cm.top(), focusIndicator + QStringLiteral("[X] "), baseStyle.foregroundColor(), baseStyle.backgroundColor());
    } else if (p->state == Tui::Unchecked) {
        painter->writeWithColors(cm.left(), cm.top(), focusIndicator + QStringLiteral("[ ] "), baseStyle.foregroundColor(), baseStyle.backgroundColor());
    } else {
        painter->writeWithColors(cm.left(), cm.top(), focusIndicator + QStringLiteral("[#] "), baseStyle.foregroundColor(), baseStyle.backgroundColor());
    }
    p->styledText.setMnemonicStyle(baseStyle, shortcut);
    p->styledText.write(painter, cm.left() + 5, cm.top(), geometry().width() - cm.left() - cm.right() - 4);
}

void ZCheckBox::toggle() {
    auto *const p = tuiwidgets_impl();
    if (p->state == Tui::Unchecked) {
        p->state = Tui::Checked;
    } else if (p->state == Tui::Checked) {
        if (p->tristate) {
            p->state = Tui::PartiallyChecked;
        } else {
            p->state = Tui::Unchecked;
        }
    } else {
        p->state = Tui::Unchecked;
    }
    stateChanged(p->state);
    update();
}

void ZCheckBox::click() {
    if (!isEnabled()) {
        return;
    }

    setFocus();
    toggle();
}

void ZCheckBox::keyEvent(ZKeyEvent *event) {
    if (event->key() == Tui::Key_Space && event->modifiers() == 0) {
        click();
    } else {
        ZWidget::keyEvent(event);
    }
}

void ZCheckBox::removeShortcut() {
    for (ZShortcut *s : findChildren<ZShortcut*>(QString(), Qt::FindDirectChildrenOnly)) {
        delete s;
    }
}

void ZCheckBox::setShortcut(const ZKeySequence &key) {
    removeShortcut();
    ZShortcut *s = new ZShortcut(key, this);
    connect(s, &ZShortcut::activated, this, &ZCheckBox::click);
}

QSize ZCheckBox::sizeHint() const {
    auto *const p = tuiwidgets_impl();
    auto *const term = terminal();
    if (!term) return {};
    auto cm = contentsMargins();
    QSize sh = { p->styledText.width(term->textMetrics()) + 5 + cm.left() + cm.right(),
                 1 + cm.top() + cm.bottom() };
    return sh;
}

bool ZCheckBox::event(QEvent *event) {
    return ZWidget::event(event);
}

bool ZCheckBox::eventFilter(QObject *watched, QEvent *event) {
    return ZWidget::eventFilter(watched, event);
}

QSize ZCheckBox::minimumSizeHint() const {
    return ZWidget::minimumSizeHint();
}

QRect ZCheckBox::layoutArea() const {
    return ZWidget::layoutArea();
}

QObject *ZCheckBox::facet(const QMetaObject &metaObject) const {
    return ZWidget::facet(metaObject);
}

ZWidget *ZCheckBox::resolveSizeHintChain() {
    return ZWidget::resolveSizeHintChain();
}

void ZCheckBox::timerEvent(QTimerEvent *event) {
    return ZWidget::timerEvent(event);
}

void ZCheckBox::childEvent(QChildEvent *event) {
    return ZWidget::childEvent(event);
}

void ZCheckBox::customEvent(QEvent *event) {
    return ZWidget::customEvent(event);
}

void ZCheckBox::connectNotify(const QMetaMethod &signal) {
    return ZWidget::connectNotify(signal);
}

void ZCheckBox::disconnectNotify(const QMetaMethod &signal) {
    return ZWidget::disconnectNotify(signal);
}

void ZCheckBox::pasteEvent(ZPasteEvent *event) {
    return ZWidget::pasteEvent(event);
}

void ZCheckBox::focusInEvent(ZFocusEvent *event) {
    return ZWidget::focusInEvent(event);
}

void ZCheckBox::focusOutEvent(ZFocusEvent *event) {
    return ZWidget::focusOutEvent(event);
}

void ZCheckBox::resizeEvent(ZResizeEvent *event) {
    return ZWidget::resizeEvent(event);
}

void ZCheckBox::moveEvent(ZMoveEvent *event) {
    return ZWidget::moveEvent(event);
}

ZCheckBoxPrivate::ZCheckBoxPrivate(ZWidget *pub) : ZWidgetPrivate(pub) {
}

ZCheckBoxPrivate::~ZCheckBoxPrivate() {
}

TUIWIDGETS_NS_END
