// SPDX-License-Identifier: BSL-1.0

#include "ZMenubar.h"
#include "ZMenubar_p.h"
#include "ZStyledTextLine.h"

#include <Tui/ZCommandManager.h>
#include <Tui/ZTerminal.h>

TUIWIDGETS_NS_START

ZMenubar::ZMenubar(ZWidget *parent) : ZWidget(parent, std::make_unique<ZMenubarPrivate>(this)) {
    auto *const p = tuiwidgets_impl();

    setSizePolicyV(SizePolicy::Fixed);
    setSizePolicyH(SizePolicy::Expanding);
    if (parent) {
        updateCacheAndRegrabKeys();
        ZCommandManager *const cmdMgr = parentWidget()->ensureCommandManager();
        p->commandStateChangedConnection = connect(cmdMgr, &ZCommandManager::commandStateChanged, this, &ZMenubar::commandStateChanged);
    }
}

ZMenubar::~ZMenubar() {
    auto *const p = tuiwidgets_impl();
    disconnect(p->commandStateChangedConnection);
}

bool ZMenubar::event(QEvent *event) {
    auto *const p = tuiwidgets_impl();
    if (!parent()) {
        return ZWidget::event(event);
    }

    switch (event->type()) {
        case QEvent::ParentChange: {
            updateCacheAndRegrabKeys();
            ZCommandManager *const cmdMgr = parentWidget()->ensureCommandManager();
            p->commandStateChangedConnection = connect(cmdMgr, &ZCommandManager::commandStateChanged, this, &ZMenubar::commandStateChanged);
            break;
        }
        case QEvent::ParentAboutToChange: {
            disconnect(p->commandStateChangedConnection);
            break;
        }
        default:
            break;
    }

    if (event->type() == ZEventType::otherChange()) {
        if (!static_cast<ZOtherChangeEvent*>(event)->unchanged().contains(TUISYM_LITERAL("terminal"))) {
            updateCacheAndRegrabKeys();
        }
    }

    return ZWidget::event(event);
}

QVector<ZMenuItem> ZMenubar::items() const {
    auto *const p = tuiwidgets_impl();
    return p->items;
}

void ZMenubar::setItems(QVector<ZMenuItem> items) {
    auto *const p = tuiwidgets_impl();
    p->items = items;
    p->selected = 0;
    update();
    p->shortcuts.clear();
    for (int i = 0; i < p->items.size(); i++) {
        const auto& item = qAsConst(p->items)[i];
        if (item.command()) {
            p->shortcuts[item.command()];
        }
    }
    updateCacheAndRegrabKeys();
}

void ZMenubar::updateCacheAndRegrabKeys() {
    auto *const p = tuiwidgets_impl();
    if (!parentWidget()) {
        return;
    }

    auto *term = terminal();
    if (!term) return;

    for (ZShortcut *s : findChildren<ZShortcut*>(QStringLiteral(""), Qt::FindDirectChildrenOnly)) {
        delete s;
    }

    ZShortcut *s = new ZShortcut(ZKeySequence::forKey(Qt::Key_F10), this, Qt::ApplicationShortcut);
    connect(s, &ZShortcut::activated, this, [this] () {
        auto *const p = tuiwidgets_impl();
        grabKeyboard();
        p->active = true;
        respawnMenu();
        update();
    });

    int x = 1;
    p->itemX.resize(p->items.size());
    ZCommandManager *const cmdMgr = parentWidget()->ensureCommandManager();
    for (int i = 0; i < p->items.size(); i++) {
        p->itemX[i] = x;
        const auto& item = qAsConst(p->items)[i];
        ZStyledTextLine st;
        st.setMarkup(QStringLiteral("<body> ") + item.markup() + QStringLiteral(" </body>"));
        x += st.width(term->textMetrics());
        if (st.mnemonic().size()) {
            s = new ZShortcut(ZKeySequence::forMnemonic(st.mnemonic()), this, Qt::ApplicationShortcut);
            if (item.command()) {
                s->setEnabled(cmdMgr->isCommandEnabled(item.command()));
                connect(s, &ZShortcut::activated, this, [this, cmd=item.command()] () {
                    ZCommandManager *const cmdMgr = parentWidget()->ensureCommandManager();
                    if (cmdMgr->isCommandEnabled(cmd)) {
                        cmdMgr->activateCommand(cmd);
                    }
                });
                p->shortcuts[item.command()].append(s);
            } else if (item.hasSubitems()) {
                connect(s, &ZShortcut::activated, this, [this, idx=i] () {
                    auto *const p = tuiwidgets_impl();
                    grabKeyboard();
                    p->active = true;
                    p->selected = idx;
                    respawnMenu();
                });
            }
        }
    }
    p->width = x;
}

void ZMenubar::commandStateChanged(ZSymbol command) {
    auto *const p = tuiwidgets_impl();
    if (p->shortcuts.contains(command)) {
        update();
        ZCommandManager *const cmdMgr = parentWidget()->ensureCommandManager();
        for (ZShortcut *const s: p->shortcuts[command]) {
            s->setEnabled(cmdMgr->isCommandEnabled(command));
        }
    }
}

void ZMenubar::respawnMenu() {
    auto *const p = tuiwidgets_impl();
    delete p->activeMenu;
    if (qAsConst(p->items)[p->selected].hasSubitems()) {
        p->activeMenu = new ZMenu(parentWidget());
        p->activeMenu->setParentMenu(this);
        p->activeMenu->setItems(qAsConst(p->items)[p->selected].subitems());
        QSize submenuSize = p->activeMenu->sizeHint();
        QRect rect = {p->itemX[p->selected] - 1, 1, submenuSize.width(), submenuSize.height()};
        if (rect.right() >= geometry().width()) {
            rect.moveRight(geometry().width() - 1);
        }
        if (rect.left() < 0) {
            rect.moveLeft(0);
        }
        p->activeMenu->setGeometry(rect);
        p->activeMenu->grabKeyboard();
    } else {
        grabKeyboard();
    }
}

void ZMenubar::left() {
    auto *const p = tuiwidgets_impl();
    if (!parentWidget() || p->items.isEmpty()) {
        return;
    }
    if (p->selected > 0) {
        --p->selected;
    } else {
        p->selected = p->items.size() - 1;
    }
    update();
    respawnMenu();
}

void ZMenubar::right() {
    auto *const p = tuiwidgets_impl();
    if (!parentWidget() || p->items.isEmpty()) {
        return;
    }
    if (p->selected < p->items.size() - 1) {
        ++p->selected;
    } else {
        p->selected = 0;
    }
    update();
    respawnMenu();
}

void ZMenubar::close() {
    auto *const p = tuiwidgets_impl();
    if (!parentWidget() || p->items.isEmpty()) {
        return;
    }
    if (p->activeMenu) {
        p->activeMenu->deleteLater();
    }
    grabKeyboard(); // hack to make sure keyboard grab is released
    releaseKeyboard();
    p->active = false;
    update();
}

void ZMenubar::paintEvent(ZPaintEvent *event) {
    auto *const p = tuiwidgets_impl();
    auto *painter = event->painter();
    ZTextStyle baseStyle = {getColor("menu.fg"), getColor("menu.bg")};
    ZTextStyle shortcut = {getColor("menu.shortcut.fg"), getColor("menu.shortcut.bg"), ZPainter::Attribute::Underline};
    // For better support of terminals that don't do color or that don't understand the color format used, apply
    // inverse attribute but also swap background and foreground color. This ensures that these terminals show the
    // selected entry in inverse. By also swapping the colors terminals with full color support will still show the
    // intended colors.
    ZTextStyle selectedStyle = {getColor("menu.selected.bg"), getColor("menu.selected.fg"), ZPainter::Attribute::Inverse};
    ZTextStyle selectedShortcut = {getColor("menu.selected.shortcut.bg"), getColor("menu.selected.shortcut.fg"),
                                  ZPainter::Attribute::Inverse | ZPainter::Attribute::Underline};
    ZTextStyle disabledStyle = {getColor("menu.disabled.fg"), getColor("menu.disabled.bg")};
    ZTextStyle selectedDisabledStyle = {getColor("menu.selected.disabled.fg"), getColor("menu.selected.disabled.bg")};
    painter->clear(baseStyle.foregroundColor(), baseStyle.backgroundColor());
    ZCommandManager *const cmdMgr = parentWidget()->ensureCommandManager();
    int x = 1;
    p->itemX.resize(p->items.size());
    for (int i = 0; i <p->items.size(); i++) {
        p->itemX[i] = x;
        const auto& item = qAsConst(p->items)[i];
        ZStyledTextLine st;
        st.setMarkup(QStringLiteral("<body> ") + item.markup() + QStringLiteral(" </body>"));
        bool isItemEnabled = (item.hasSubitems() || cmdMgr->isCommandEnabled(item.command())) && isEnabled();
        if (i == p->selected && p->active) {
            if (isItemEnabled) {
                st.setMnemonicStyle(selectedStyle, selectedShortcut);
            } else {
                st.setMnemonicStyle(selectedDisabledStyle, selectedDisabledStyle);
            }
        } else {
            if (isItemEnabled) {
                st.setMnemonicStyle(baseStyle, shortcut);
            } else {
                st.setMnemonicStyle(disabledStyle, disabledStyle);
            }
        }
        st.write(painter, x, 0, st.width(painter->textMetrics()));
        x += st.width(painter->textMetrics());
    }
    p->width = x;
}

void ZMenubar::keyActivate(ZKeyEvent *event) {
    auto *const p = tuiwidgets_impl();
    ZCommandManager *const cmdMgr = parentWidget()->ensureCommandManager();
    for (int i = 0; i < p->items.size(); i++) {
        const auto &item = qAsConst(p->items)[i];
        ZStyledTextLine st;
        st.setMarkup(QStringLiteral("<body> ") + item.markup() + QStringLiteral(" </body>"));
        bool isEnabled = item.hasSubitems() || cmdMgr->isCommandEnabled(item.command());
        if (isEnabled && st.mnemonic().toLower() == event->text().toLower()) {
            p->selected = i;
            update();
            if (item.command()) {
                close();
                cmdMgr->activateCommand(item.command());
            } else if (item.hasSubitems()) {
                respawnMenu();
            }
        }
    }
}

QSize ZMenubar::sizeHint() const {
    auto *const p = tuiwidgets_impl();
    return { p->width, 1 };
}

void ZMenubar::keyEvent(ZKeyEvent *event) {
    auto *const p = tuiwidgets_impl();
    if (event->key() == Qt::Key_Left && event->modifiers() == 0) {
        left();
    } else if (event->key() == Qt::Key_Right && event->modifiers() == 0) {
        right();
    } else if ((event->key() == Qt::Key_Escape || event->key() == Qt::Key_F10) && event->modifiers() == 0) {
        close();
    } else if (event->key() == Qt::Key_Enter && event->modifiers() == 0) {
        ZCommandManager *const cmdMgr = parentWidget()->ensureCommandManager();
        const auto &item = qAsConst(p->items)[p->selected];
        bool isEnabled = item.command() && cmdMgr->isCommandEnabled(item.command());
        if (isEnabled) {
            close();
            cmdMgr->activateCommand(item.command());
        }
    } else if (event->modifiers() == Qt::AltModifier && event->text().size()) {
        keyActivate(event);
    } else {
        ZWidget::keyEvent(event);
    }
}

bool ZMenubar::eventFilter(QObject *watched, QEvent *event) {
    return ZWidget::eventFilter(watched, event);
}

QSize ZMenubar::minimumSizeHint() const {
    return ZWidget::minimumSizeHint();
}

QRect ZMenubar::layoutArea() const {
    return ZWidget::layoutArea();
}

QObject *ZMenubar::facet(const QMetaObject &metaObject) const {
    return ZWidget::facet(metaObject);
}

ZWidget *ZMenubar::resolveSizeHintChain() {
    return ZWidget::resolveSizeHintChain();
}

void ZMenubar::timerEvent(QTimerEvent *event) {
    ZWidget::timerEvent(event);
}

void ZMenubar::childEvent(QChildEvent *event) {
    ZWidget::childEvent(event);
}

void ZMenubar::customEvent(QEvent *event) {
    ZWidget::customEvent(event);
}

void ZMenubar::connectNotify(const QMetaMethod &signal) {
    ZWidget::connectNotify(signal);
}

void ZMenubar::disconnectNotify(const QMetaMethod &signal) {
    ZWidget::disconnectNotify(signal);
}

void ZMenubar::pasteEvent(ZPasteEvent *event) {
    ZWidget::pasteEvent(event);
}

void ZMenubar::focusInEvent(ZFocusEvent *event) {
    ZWidget::focusInEvent(event);
}

void ZMenubar::focusOutEvent(ZFocusEvent *event) {
    ZWidget::focusOutEvent(event);
}

void ZMenubar::resizeEvent(ZResizeEvent *event) {
    ZWidget::resizeEvent(event);
}

void ZMenubar::moveEvent(ZMoveEvent *event) {
    ZWidget::moveEvent(event);
}

ZMenubarPrivate::ZMenubarPrivate(ZWidget *pub) : ZWidgetPrivate (pub) {
}

ZMenubarPrivate::~ZMenubarPrivate() {
}


TUIWIDGETS_NS_END
