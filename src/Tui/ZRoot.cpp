#include "ZRoot.h"

#include <QSize>

#include <Tui/ZPalette.h>
#include <Tui/ZPainter.h>

TUIWIDGETS_NS_START

ZRoot::ZRoot() {

    ZPalette p = palette();
    p.setColors({
        { "root.bg", Colors::black},
        { "root.fg", {0x80, 0x80, 0x80}},

        { "menu.bg", Colors::lightGray},
        { "menu.fg", Colors::black},
        { "menu.disabled.bg", Colors::lightGray},
        { "menu.disabled.fg", Colors::darkGray},
        { "menu.shortcut.bg", Colors::lightGray},
        { "menu.shortcut.fg", Colors::red},
        { "menu.selected.bg", Colors::green},
        { "menu.selected.fg", Colors::black},
        { "menu.selected.disabled.bg", Colors::green},
        { "menu.selected.disabled.fg", Colors::darkGray},
        { "menu.selected.shortcut.bg", Colors::green},
        { "menu.selected.shortcut.fg", Colors::red},

        { "window.default.bg", Colors::blue},
        { "window.default.frame.focused.fg", Colors::brightWhite},
        { "window.default.frame.focused.control.fg", Colors::brightGreen},
        { "window.default.frame.unfocused.fg", Colors::lightGray},
        { "window.default.scrollbar.fg", Colors::blue},
        { "window.default.scrollbar.bg", { 0, 0x55, 0xaa}},
        { "window.default.scrollbar.control.fg", Colors::blue},
        { "window.default.scrollbar.control.bg", Colors::cyan},
        { "window.default.text.fg", Colors::brightYellow},
        { "window.default.text.bg", Colors::blue},
        { "window.default.text.selected.fg", Colors::blue},
        { "window.default.text.selected.bg", Colors::lightGray},
        { "window.default.control.bg", Colors::blue},
        { "window.default.control.fg", Colors::lightGray},
        { "window.default.control.focused.bg", Colors::blue},
        { "window.default.control.focused.fg", Colors::brightWhite},
        { "window.default.control.disabled.bg", Colors::lightGray},
        { "window.default.control.disabled.fg", Colors::darkGray},
        { "window.default.control.shortcut.bg", Colors::blue},
        { "window.default.control.shortcut.fg", Colors::brightYellow},
        { "window.default.button.bg", Colors::green},
        { "window.default.button.fg", Colors::black},
        { "window.default.button.default.bg", Colors::green},
        { "window.default.button.default.fg", Colors::brightCyan},
        { "window.default.button.focused.bg", Colors::green},
        { "window.default.button.focused.fg", Colors::brightWhite},
        { "window.default.button.disabled.bg", Colors::lightGray},
        { "window.default.button.disabled.fg", Colors::darkGray},
        { "window.default.button.shortcut.bg", Colors::green},
        { "window.default.button.shortcut.fg", Colors::brightYellow},
        { "window.default.lineedit.bg", Colors::lightGray},
        { "window.default.lineedit.fg", Colors::black},
        { "window.default.lineedit.focused.bg", Colors::lightGray},
        { "window.default.lineedit.focused.fg", Colors::brightWhite},

        { "window.gray.bg", Colors::lightGray},
        { "window.gray.frame.focused.fg", Colors::brightWhite},
        { "window.gray.frame.focused.control.fg", Colors::brightGreen},
        { "window.gray.frame.unfocused.fg", Colors::black},
        { "window.gray.scrollbar.fg", Colors::cyan},
        { "window.gray.scrollbar.bg", Colors::blue},
        { "window.gray.scrollbar.control.fg", Colors::cyan},
        { "window.gray.scrollbar.control.bg", Colors::blue},
        { "window.gray.text.fg", Colors::lightGray},
        { "window.gray.text.bg", Colors::black},
        { "window.gray.text.selected.fg", Colors::lightGray},
        { "window.gray.text.selected.bg", Colors::brightWhite},
        { "window.gray.control.bg", Colors::lightGray},
        { "window.gray.control.fg", Colors::black},
        { "window.gray.control.focused.bg", Colors::lightGray},
        { "window.gray.control.focused.fg", Colors::brightWhite},
        { "window.gray.control.disabled.bg", Colors::lightGray},
        { "window.gray.control.disabled.fg", Colors::darkGray},
        { "window.gray.control.shortcut.bg", Colors::lightGray},
        { "window.gray.control.shortcut.fg", Colors::brightYellow},
        { "window.gray.button.bg", Colors::green},
        { "window.gray.button.fg", Colors::black},
        { "window.gray.button.default.bg", Colors::green},
        { "window.gray.button.default.fg", Colors::brightCyan},
        { "window.gray.button.focused.bg", Colors::green},
        { "window.gray.button.focused.fg", Colors::brightWhite},
        { "window.gray.button.disabled.bg", Colors::lightGray},
        { "window.gray.button.disabled.fg", Colors::darkGray},
        { "window.gray.button.shortcut.bg", Colors::green},
        { "window.gray.button.shortcut.fg", Colors::brightYellow},
        { "window.gray.lineedit.bg", Colors::blue},
        { "window.gray.lineedit.fg", Colors::brightWhite},
        { "window.gray.lineedit.focused.bg", Colors::green},
        { "window.gray.lineedit.focused.fg", Colors::brightWhite},

        { "window.Colors::cyan.bg", Colors::cyan},
        { "window.Colors::cyan.frame.focused.fg", Colors::brightWhite},
        { "window.Colors::cyan.frame.focused.control.fg", Colors::brightGreen},
        { "window.Colors::cyan.frame.unfocused.fg", Colors::lightGray},
        { "window.Colors::cyan.scrollbar.fg", Colors::cyan},
        { "window.Colors::cyan.scrollbar.bg", Colors::blue},
        { "window.Colors::cyan.scrollbar.control.fg", Colors::cyan},
        { "window.Colors::cyan.scrollbar.control.bg", Colors::blue},
        { "window.Colors::cyan.text.fg", Colors::black},
        { "window.Colors::cyan.text.bg", Colors::cyan},
        { "window.Colors::cyan.text.selected.fg", Colors::brightWhite},
        { "window.Colors::cyan.text.selected.bg", Colors::cyan},
        { "window.Colors::cyan.control.bg", Colors::cyan},
        { "window.Colors::cyan.control.fg", Colors::black},
        { "window.Colors::cyan.control.focused.bg", Colors::cyan},
        { "window.Colors::cyan.control.focused.fg", Colors::brightWhite},
        { "window.Colors::cyan.control.disabled.bg", Colors::lightGray},
        { "window.Colors::cyan.control.disabled.fg", Colors::darkGray},
        { "window.Colors::cyan.control.shortcut.bg", Colors::cyan},
        { "window.Colors::cyan.control.shortcut.fg", Colors::brightYellow},
        { "window.Colors::cyan.button.bg", Colors::green},
        { "window.Colors::cyan.button.fg", Colors::black},
        { "window.Colors::cyan.button.default.bg", Colors::green},
        { "window.Colors::cyan.button.default.fg", Colors::brightCyan},
        { "window.Colors::cyan.button.focused.bg", Colors::green},
        { "window.Colors::cyan.button.focused.fg", Colors::brightWhite},
        { "window.Colors::cyan.button.disabled.bg", Colors::lightGray},
        { "window.Colors::cyan.button.disabled.fg", Colors::darkGray},
        { "window.Colors::cyan.button.shortcut.bg", Colors::green},
        { "window.Colors::cyan.button.shortcut.fg", Colors::brightYellow},
        { "window.Colors::cyan.lineedit.bg", Colors::blue},
        { "window.Colors::cyan.lineedit.fg", Colors::brightWhite},
        { "window.Colors::cyan.lineedit.focused.bg", Colors::green},
        { "window.Colors::cyan.lineedit.focused.fg", Colors::brightWhite},

    });

    p.addRules({
        {{ QStringLiteral("window") }, {
             { p.Publish, "bg", "window.bg" },

             { p.Publish, "window.bg", "window.default.bg" },
             { p.Publish, "window.frame.focused.bg", "window.bg" },
             { p.Publish, "window.frame.focused.fg", "window.default.frame.focused.fg" },
             { p.Publish, "window.frame.focused.control.bg", "window.bg" },
             { p.Publish, "window.frame.focused.control.fg", "window.default.frame.focused.control.fg" },
             { p.Publish, "window.frame.unfocused.bg", "window.bg" },
             { p.Publish, "window.frame.unfocused.fg", "window.default.frame.unfocused.fg" },

             { p.Publish, "scrollbar.bg", "window.default.scrollbar.bg" },
             { p.Publish, "scrollbar.fg", "window.default.scrollbar.fg" },
             { p.Publish, "scrollbar.control.bg", "window.default.scrollbar.control.bg" },
             { p.Publish, "scrollbar.control.fg", "window.default.scrollbar.control.fg" },

             { p.Publish, "text.bg", "window.default.text.bg" },
             { p.Publish, "text.fg", "window.default.text.fg" },
             { p.Publish, "text.selected.bg", "window.default.text.selected.bg" },
             { p.Publish, "text.selected.fg", "window.default.text.selected.fg" },

             { p.Publish, "control.bg", "window.default.control.bg" },
             { p.Publish, "control.fg", "window.default.control.fg" },
             { p.Publish, "control.focused.bg", "window.default.control.focused.bg" },
             { p.Publish, "control.focused.fg", "window.default.control.focused.fg" },
             { p.Publish, "control.disabled.bg", "window.default.control.disabled.bg" },
             { p.Publish, "control.disabled.fg", "window.default.control.disabled.fg" },
             { p.Publish, "control.shortcut.bg", "window.default.control.shortcut.bg" },
             { p.Publish, "control.shortcut.fg", "window.default.control.shortcut.fg" },

             { p.Publish, "button.bg", "window.default.button.bg" },
             { p.Publish, "button.fg", "window.default.button.fg" },
             { p.Publish, "button.default.bg", "window.default.button.default.bg" },
             { p.Publish, "button.default.fg", "window.default.button.default.fg" },
             { p.Publish, "button.focused.bg", "window.default.button.focused.bg" },
             { p.Publish, "button.focused.fg", "window.default.button.focused.fg" },
             { p.Publish, "button.disabled.bg", "window.default.button.disabled.bg" },
             { p.Publish, "button.disabled.fg", "window.default.button.disabled.fg" },
             { p.Publish, "button.shortcut.bg", "window.default.button.shortcut.bg" },
             { p.Publish, "button.shortcut.fg", "window.default.button.shortcut.fg" },

             { p.Publish, "lineedit.bg", "window.default.lineedit.bg" },
             { p.Publish, "lineedit.fg", "window.default.lineedit.fg" },
             { p.Publish, "lineedit.focused.bg", "window.default.lineedit.focused.bg" },
             { p.Publish, "lineedit.focused.fg", "window.default.lineedit.focused.fg" },
        }},

        {{ QStringLiteral("window"), QStringLiteral("dialog") }, {
             { p.Publish, "bg", "window.bg" },

             { p.Publish, "window.bg", "window.gray.bg" },
             { p.Publish, "window.frame.focused.bg", "window.bg" },
             { p.Publish, "window.frame.focused.fg", "window.gray.frame.focused.fg" },
             { p.Publish, "window.frame.focused.control.bg", "window.bg" },
             { p.Publish, "window.frame.focused.control.fg", "window.gray.frame.focused.control.fg" },
             { p.Publish, "window.frame.unfocused.bg", "window.bg" },
             { p.Publish, "window.frame.unfocused.fg", "window.gray.frame.unfocused.fg" },

             { p.Publish, "scrollbar.bg", "window.gray.scrollbar.control.bg" },
             { p.Publish, "scrollbar.fg", "window.gray.scrollbar.control.fg" },
             { p.Publish, "scrollbar.control.bg", "window.gray.scrollbar.control.bg" },
             { p.Publish, "scrollbar.control.fg", "window.gray.scrollbar.control.fg" },

             { p.Publish, "text.bg", "window.gray.text.bg" },
             { p.Publish, "text.fg", "window.gray.text.fg" },
             { p.Publish, "text.selected.bg", "window.gray.text.selected.bg" },
             { p.Publish, "text.selected.fg", "window.gray.text.selected.fg" },

             { p.Publish, "control.bg", "window.gray.control.bg" },
             { p.Publish, "control.fg", "window.gray.control.fg" },
             { p.Publish, "control.focused.bg", "window.gray.control.focused.bg" },
             { p.Publish, "control.focused.fg", "window.gray.control.focused.fg" },
             { p.Publish, "control.disabled.bg", "window.gray.control.disabled.bg" },
             { p.Publish, "control.disabled.fg", "window.gray.control.disabled.fg" },
             { p.Publish, "control.shortcut.bg", "window.gray.control.shortcut.bg" },
             { p.Publish, "control.shortcut.fg", "window.gray.control.shortcut.fg" },

             { p.Publish, "button.bg", "window.gray.button.bg" },
             { p.Publish, "button.fg", "window.gray.button.fg" },
             { p.Publish, "button.default.bg", "window.gray.button.default.bg" },
             { p.Publish, "button.default.fg", "window.gray.button.default.fg" },
             { p.Publish, "button.focused.bg", "window.gray.button.focused.bg" },
             { p.Publish, "button.focused.fg", "window.gray.button.focused.fg" },
             { p.Publish, "button.disabled.bg", "window.gray.button.disabled.bg" },
             { p.Publish, "button.disabled.fg", "window.gray.button.disabled.fg" },
             { p.Publish, "button.shortcut.bg", "window.gray.button.shortcut.bg" },
             { p.Publish, "button.shortcut.fg", "window.gray.button.shortcut.fg" },

             { p.Publish, "lineedit.bg", "window.gray.lineedit.bg" },
             { p.Publish, "lineedit.fg", "window.gray.lineedit.fg" },
             { p.Publish, "lineedit.focused.bg", "window.gray.lineedit.focused.bg" },
             { p.Publish, "lineedit.focused.fg", "window.gray.lineedit.focused.fg" },
       }},

       {{ QStringLiteral("window"), QStringLiteral("cyan") }, {
            { p.Publish, "bg", "window.bg" },

            { p.Publish, "window.bg", "window.cyan.bg" },
            { p.Publish, "window.frame.focused.bg", "window.bg" },
            { p.Publish, "window.frame.focused.fg", "window.cyan.frame.focused.fg" },
            { p.Publish, "window.frame.focused.control.bg", "window.bg" },
            { p.Publish, "window.frame.focused.control.fg", "window.cyan.frame.focused.control.fg" },
            { p.Publish, "window.frame.unfocused.bg", "window.bg" },
            { p.Publish, "window.frame.unfocused.fg", "window.cyan.frame.unfocused.fg" },

            { p.Publish, "scrollbar.bg", "window.cyan.scrollbar.control.bg" },
            { p.Publish, "scrollbar.fg", "window.cyan.scrollbar.control.fg" },
            { p.Publish, "scrollbar.control.bg", "window.cyan.scrollbar.control.bg" },
            { p.Publish, "scrollbar.control.fg", "window.cyan.scrollbar.control.fg" },

            { p.Publish, "text.bg", "window.cyan.text.bg" },
            { p.Publish, "text.fg", "window.cyan.text.fg" },
            { p.Publish, "text.selected.bg", "window.cyan.text.selected.bg" },
            { p.Publish, "text.selected.fg", "window.cyan.text.selected.fg" },

            { p.Publish, "control.bg", "window.cyan.control.bg" },
            { p.Publish, "control.fg", "window.cyan.control.fg" },
            { p.Publish, "control.focused.bg", "window.cyan.control.focused.bg" },
            { p.Publish, "control.focused.fg", "window.cyan.control.focused.fg" },
            { p.Publish, "control.disabled.bg", "window.cyan.control.disabled.bg" },
            { p.Publish, "control.disabled.fg", "window.cyan.control.disabled.fg" },
            { p.Publish, "control.shortcut.bg", "window.cyan.control.shortcut.bg" },
            { p.Publish, "control.shortcut.fg", "window.cyan.control.shortcut.fg" },

            { p.Publish, "button.bg", "window.cyan.button.bg" },
            { p.Publish, "button.fg", "window.cyan.button.fg" },
            { p.Publish, "button.default.bg", "window.cyan.button.default.bg" },
            { p.Publish, "button.default.fg", "window.cyan.button.default.fg" },
            { p.Publish, "button.focused.bg", "window.cyan.button.focused.bg" },
            { p.Publish, "button.focused.fg", "window.cyan.button.focused.fg" },
            { p.Publish, "button.disabled.bg", "window.cyan.button.disabled.bg" },
            { p.Publish, "button.disabled.fg", "window.cyan.button.disabled.fg" },
            { p.Publish, "button.shortcut.bg", "window.cyan.button.shortcut.bg" },
            { p.Publish, "button.shortcut.fg", "window.cyan.button.shortcut.fg" },

            { p.Publish, "lineedit.bg", "window.cyan.lineedit.bg" },
            { p.Publish, "lineedit.fg", "window.cyan.lineedit.fg" },
            { p.Publish, "lineedit.focused.bg", "window.cyan.lineedit.focused.bg" },
            { p.Publish, "lineedit.focused.fg", "window.cyan.lineedit.focused.fg" },
       }},

    });
    setPalette(p);
}

void ZRoot::paintEvent(ZPaintEvent *event) {
    auto *painter = event->painter();
    painter->clear(getColor("root.fg"), getColor("root.bg"));
}

void ZRoot::keyEvent(ZKeyEvent *event) {
    if (event->key() == Qt::Key_F6 && (event->modifiers() == 0 || event->modifiers() == Qt::Modifier::SHIFT)) {
        ZWidget *first = nullptr;
        bool arm = false;
        bool found = false;
        QList<ZWidget*> childWindows = findChildren<ZWidget*>(QStringLiteral(""), Qt::FindDirectChildrenOnly);
        QMutableListIterator<ZWidget*> it(childWindows);
        while (it.hasNext()) {
            ZWidget* w = it.next();
            if (!w->paletteClass().contains(QStringLiteral("window"))) {
                it.remove();
            }
            if (!w->isVisible()) {
                it.remove();
            }
        }
        if (event->modifiers() == Qt::Modifier::SHIFT) {
            std::reverse(childWindows.begin(), childWindows.end());
        }
        for(ZWidget *win : childWindows) {
            if (!first) {
                first = win->placeFocus();
            }
            if (arm) {
                ZWidget *w = win->placeFocus();
                if (w) {
                    w->setFocus(Qt::FocusReason::ActiveWindowFocusReason);
                    found = true;
                    break;
                }
            }
            if (win->isInFocusPath()) {
                arm = true;
            }
        }
        if (!found && first) {
            first->setFocus(Qt::FocusReason::ActiveWindowFocusReason);
        }
    }
}

bool ZRoot::event(QEvent *event) {
    return ZWidget::event(event);
}

bool ZRoot::eventFilter(QObject *watched, QEvent *event) {
    return ZWidget::eventFilter(watched, event);
}

void ZRoot::timerEvent(QTimerEvent *event) {
    ZWidget::timerEvent(event);
}

void ZRoot::childEvent(QChildEvent *event) {
    ZWidget::childEvent(event);
}

void ZRoot::customEvent(QEvent *event) {
    ZWidget::customEvent(event);
}

void ZRoot::connectNotify(const QMetaMethod &signal) {
    // XXX needs to be thread-safe
    ZWidget::connectNotify(signal);
}

void ZRoot::disconnectNotify(const QMetaMethod &signal) {
    // XXX needs to be thread-safe
    ZWidget::disconnectNotify(signal);
}

QSize ZRoot::sizeHint() const {
    return ZWidget::sizeHint();
}

QObject *ZRoot::facet(const QMetaObject metaObject) {
    return ZWidget::facet(metaObject);
}

void ZRoot::focusInEvent(ZFocusEvent *event) {
    ZWidget::focusInEvent(event);
}

void ZRoot::focusOutEvent(ZFocusEvent *event) {
    ZWidget::focusOutEvent(event);
}

void ZRoot::resizeEvent(ZResizeEvent *event) {
    ZWidget::resizeEvent(event);
}

void ZRoot::moveEvent(ZMoveEvent *event) {
    ZWidget::moveEvent(event);
}

TUIWIDGETS_NS_END
