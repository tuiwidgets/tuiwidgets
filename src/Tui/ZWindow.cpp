#include "ZWindow.h"
#include "ZWindow_p.h"

#include <QCoreApplication>
#include <QSize>

#include <Tui/ZColor.h>
#include <Tui/ZCommandNotifier.h>
#include <Tui/ZLayout.h>
#include <Tui/ZMenu.h>
#include <Tui/ZPainter.h>
#include <Tui/ZSymbol.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZTextMetrics.h>

TUIWIDGETS_NS_START

static ZSymbol extendedCharset = TUISYM_LITERAL("extendedCharset");

ZWindow::ZWindow(ZWidget *parent) : ZWidget(parent, std::make_unique<ZWindowPrivate>(this)) {
    setFocusMode(FocusContainerMode::Cycle);
    addPaletteClass(QStringLiteral("window"));
    setSizePolicyH(SizePolicy::Expanding);
    setSizePolicyV(SizePolicy::Expanding);

    QObject::connect(new ZCommandNotifier("ZWindowInteractiveMove", this, Qt::WindowShortcut), &ZCommandNotifier::activated, this, &ZWindow::startInteractiveMove);
    QObject::connect(new ZCommandNotifier("ZWindowInteractiveResize", this, Qt::WindowShortcut), &ZCommandNotifier::activated, this, &ZWindow::startInteractiveResize);
    QObject::connect(new ZCommandNotifier("ZWindowAutomaticPlacement", this, Qt::WindowShortcut), &ZCommandNotifier::activated, this, &ZWindow::setAutomaticPlacement);
    QObject::connect(new ZCommandNotifier("ZWindowClose", this, Qt::WindowShortcut), &ZCommandNotifier::activated, this, &ZWindow::close);
}

ZWindow::ZWindow(const QString &title, ZWidget *parent)
    : ZWindow(parent)
{
    setWindowTitle(title);
}

ZWindow::~ZWindow() {
}

QString ZWindow::windowTitle() const {
    auto *const p = tuiwidgets_impl();
    return p->windowTitle;
}

void ZWindow::setWindowTitle(const QString &title) {
    auto *const p = tuiwidgets_impl();
    if (p->windowTitle != title) {
        p->windowTitle = title;
        windowTitleChanged(title);
    }
}

bool ZWindow::showSystemMenu() {
    auto menuItems = systemMenu();
    bool nonEmpty = false;
    for (const auto &menuItem: menuItems) {
        if (menuItem.markup().size()) {
            nonEmpty = true;
            break;
        }
    }
    if (nonEmpty) {
        ZMenu *menu = new ZMenu(parentWidget());
        menu->setItems(menuItems);
        QObject::connect(menu, &ZMenu::aboutToHide, menu, &QObject::deleteLater);
        menu->popup({geometry().x(), geometry().y() + 1});
        return true;
    } else {
        return false;
    }
}

void ZWindow::startInteractiveMove() {
    auto *const p = tuiwidgets_impl();
    p->startInteractiveGeometry(this);
    grabKeyboard([this] (QEvent *event) {
        auto *const p = tuiwidgets_impl();
        if (event->type() == ZEventType::key()) {
            auto *keyEvent = static_cast<ZKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Enter) {
                p->finalizeInteractiveGeometry(this);
            } else if (keyEvent->key() == Qt::Key_Escape) {
                p->cancelInteractiveGeometry(this);
            } else if (keyEvent->key() == Qt::Key_Left) {
                QRect g = geometry();
                setGeometry(g.translated(-1, 0));
            } else if (keyEvent->key() == Qt::Key_Right) {
                QRect g = geometry();
                setGeometry(g.translated(1, 0));
            } else if (keyEvent->key() == Qt::Key_Up) {
                QRect g = geometry();
                setGeometry(g.translated(0, -1));
            } else if (keyEvent->key() == Qt::Key_Down) {
                QRect g = geometry();
                setGeometry(g.translated(0, 1));
            }
        }
    });
}

void ZWindow::startInteractiveResize() {
    auto *const p = tuiwidgets_impl();
    p->startInteractiveGeometry(this);
    grabKeyboard([this] (QEvent *event) {
        auto *const p = tuiwidgets_impl();
        if (event->type() == ZEventType::key()) {
            auto *keyEvent = static_cast<ZKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Enter) {
                p->finalizeInteractiveGeometry(this);
            } else if (keyEvent->key() == Qt::Key_Escape) {
                p->cancelInteractiveGeometry(this);
            } else if (keyEvent->key() == Qt::Key_Left) {
                QRect g = geometry();
                g.setWidth(std::max(effectiveMinimumSize().width(), std::max(3, g.width() - 1)));
                setGeometry(g);
            } else if (keyEvent->key() == Qt::Key_Right) {
                QRect g = geometry();
                g.setWidth(std::min(maximumSize().width(), g.width() + 1));
                setGeometry(g);
            } else if (keyEvent->key() == Qt::Key_Up) {
                QRect g = geometry();
                g.setHeight(std::max(effectiveMinimumSize().height(), std::max(3, g.height() - 1)));
                setGeometry(g);
            } else if (keyEvent->key() == Qt::Key_Down) {
                QRect g = geometry();
                g.setHeight(std::min(maximumSize().height(), g.height() + 1));
                setGeometry(g);
            }
        }
    });
}

ZWindow::Options ZWindow::options() const {
    auto *const p = tuiwidgets_impl();
    return p->options;
}

void ZWindow::setOptions(ZWindow::Options options) {
    auto *const p = tuiwidgets_impl();
    p->options = options;
}

Qt::Edges ZWindow::borderEdges() const {
    auto *const p = tuiwidgets_impl();
    return p->borders;
}

void ZWindow::setBorderEdges(Qt::Edges borders) {
    auto *const p = tuiwidgets_impl();
    p->borders = borders;
}

void ZWindow::setDefaultPlacement(Qt::Alignment align, QPoint displace) {
    auto *const p = tuiwidgets_impl();
    QObject *windowFacet = facet(ZWindowFacet::staticMetaObject);
    if (windowFacet == p->windowFacet.get()) { // ensure that the default facet is actually used
        p->windowFacet->setDefaultPlacement(align, displace);
        p->ensureAutoPlacement();
    } else {
        qWarning("ZWindow::setDefaultPlacement calls with overriden WindowFacet do nothing.");
    }
}

void ZWindow::setAutomaticPlacement() {
    auto *windowFacet = qobject_cast<ZWindowFacet*>(facet(ZWindowFacet::staticMetaObject));
    if (windowFacet) {
        windowFacet->setManuallyPlaced(false);
        auto *const p = tuiwidgets_impl();
        p->ensureAutoPlacement();
    }
}

QSize ZWindow::sizeHint() const {
    auto *const p = tuiwidgets_impl();

    QSize res;

    if (layout()) {
        res = layout()->sizeHint();
        QMargins cm = contentsMargins();
        res.rwidth() += cm.left() + cm.right();
        res.rheight() += cm.top() + cm.bottom();

        if (p->borders & Qt::TopEdge) {
            res += QSize(0, 1);
        }
        if (p->borders & Qt::RightEdge) {
            res += QSize(1, 0);
        }
        if (p->borders & Qt::BottomEdge) {
            res += QSize(0, 1);
        }
        if (p->borders & Qt::LeftEdge) {
            res += QSize(1, 0);
        }
    }
    return res;
}

QRect ZWindow::layoutArea() const {
    auto *const p = tuiwidgets_impl();
    QRect r = { QPoint(0, 0), geometry().size() };
    if (p->borders & Qt::TopEdge) {
        r.adjust(0, 1, 0, 0);
    }
    if (p->borders & Qt::RightEdge) {
        r.adjust(0, 0, -1, 0);
    }
    if (p->borders & Qt::BottomEdge) {
        r.adjust(0, 0, 0, -1);
    }
    if (p->borders & Qt::LeftEdge) {
        r.adjust(1, 0, 0, 0);
    }
    r = r.marginsRemoved(contentsMargins());
    return r;
}

QObject *ZWindow::facet(const QMetaObject metaObject) {
    auto *const p = tuiwidgets_impl();
    if (metaObject.className() == ZWindowFacet::staticMetaObject.className()) {
        if (!p->windowFacet) {
            p->windowFacet = std::make_unique<ZBasicWindowFacet>();
        }
        return p->windowFacet.get();
    } else {
        return ZWidget::facet(metaObject);
    }
}

const static struct {
    const char *topLeft;
    char16_t horizontal;
    const char *topRight;
    const char *vertical;
    const char *bottomRight;
    const char *bottomLeft;
    const char *terminatorTop;
    const char *terminatorRight;
    const char *terminatorBottom;
    const char *terminatorLeft;
} windowDecorations[] = {
    // normal inactive
    { "┌", u'─', "┐", "│", "┘", "└", "╷", "╴", "╵", "╶" },
    // normal active
    { "╔", u'═', "╗", "║", "╝", "╚", "╽", "╾", "╿", "╼" },
    // limited inactive
    { "+", u'-', "+", "|", "+", "+", "#", "#", "#", "#" },
    // limited active
    { "*", u'=', "*", "|", "*", "*", "#", "#", "#", "#" },
};

void ZWindow::paintEvent(ZPaintEvent *event) {
    auto *const p = tuiwidgets_impl();
    ZColor frameBg, frameFg;
    ZColor buttonBg, buttonFg;
    bool active = isInFocusPath();

    if (active) {
        if (!p->interactiveMode) {
            frameBg = getColor("window.frame.focused.bg");
            frameFg = getColor("window.frame.focused.fg");
        } else {
            frameBg = getColor("window.frame.focused.control.bg");
            frameFg = getColor("window.frame.focused.control.fg");
        }
        buttonBg = getColor("window.frame.focused.control.bg");
        buttonFg = getColor("window.frame.focused.control.fg");
    } else {
        frameBg = getColor("window.frame.unfocused.bg");
        frameFg = getColor("window.frame.unfocused.fg");
    }

    auto *painter = event->painter();
    ZTextMetrics metrics = painter->textMetrics();
    painter->clear(frameFg, frameBg);
    int w = geometry().width();
    int h = geometry().height();
    bool extendetCharsetAvailable = terminal()->hasCapability(extendedCharset);
    auto decorations = windowDecorations[active + 2 * !extendetCharsetAvailable];

    if (p->borders & Qt::TopEdge && p->borders & Qt::LeftEdge) {
        painter->writeWithColors(0, 0, QString::fromUtf8(decorations.topLeft), frameFg, frameBg);
    } else if (p->borders & Qt::TopEdge) {
        painter->writeWithColors(0, 0, QString::fromUtf8(decorations.terminatorLeft), frameFg, frameBg);
    } else if (p->borders & Qt::LeftEdge) {
        painter->writeWithColors(0, 0, QString::fromUtf8(decorations.terminatorTop), frameFg, frameBg);
    }
    if (p->borders & Qt::TopEdge && p->borders & Qt::RightEdge) {
        painter->writeWithColors(w - 1, 0, QString::fromUtf8(decorations.topRight), frameFg, frameBg);
    } else if (p->borders & Qt::TopEdge) {
        painter->writeWithColors(w - 1, 0, QString::fromUtf8(decorations.terminatorRight), frameFg, frameBg);
    } else if (p->borders & Qt::RightEdge) {
        painter->writeWithColors(w - 1, 0, QString::fromUtf8(decorations.terminatorTop), frameFg, frameBg);
    }
    if (p->borders & Qt::BottomEdge && p->borders & Qt::RightEdge) {
        painter->writeWithColors(w - 1, h - 1, QString::fromUtf8(decorations.bottomRight), frameFg, frameBg);
    } else if (p->borders & Qt::BottomEdge) {
        painter->writeWithColors(w - 1, h - 1, QString::fromUtf8(decorations.terminatorRight), frameFg, frameBg);
    } else if (p->borders & Qt::RightEdge) {
        painter->writeWithColors(w - 1, h - 1, QString::fromUtf8(decorations.terminatorBottom), frameFg, frameBg);
    }
    if (p->borders & Qt::BottomEdge && p->borders & Qt::LeftEdge) {
        painter->writeWithColors(0, h - 1, QString::fromUtf8(decorations.bottomLeft), frameFg, frameBg);
    } else if (p->borders & Qt::BottomEdge) {
        painter->writeWithColors(0, h - 1, QString::fromUtf8(decorations.terminatorLeft), frameFg, frameBg);
    } else if (p->borders & Qt::LeftEdge) {
        painter->writeWithColors(0, h - 1, QString::fromUtf8(decorations.terminatorBottom), frameFg, frameBg);
    }

    QString hline = QString(w - 2, decorations.horizontal);
    if (p->borders & Qt::TopEdge) {
        painter->writeWithColors(1, 0, hline, frameFg, frameBg);
    }
    if (p->borders & Qt::BottomEdge) {
        painter->writeWithColors(1, h - 1, hline, frameFg, frameBg);
    }

    if (p->borders & Qt::TopEdge && p->windowTitle.size()) {
        int titleLength = metrics.sizeInColumns(p->windowTitle);
        int minX = options() & ZWindow::CloseButton ? 6 : 1;
        if (minX + titleLength + 1 > w) {
            --minX;
        }
        int x = std::max(minX, w / 2 - titleLength / 2);
        if (minX < x && x != 1) {
            painter->writeWithColors(x - 1, 0, QStringLiteral(" "), frameFg, frameBg);
        }
        painter->writeWithColors(x, 0, p->windowTitle, frameFg, frameBg);
        if (x + titleLength < w - 1) {
            painter->writeWithColors(x + titleLength, 0, QStringLiteral(" "), frameFg, frameBg);
        }
    }

    for (int i = 1; i < h - 1; i++) {
        if (p->borders & Qt::LeftEdge) {
            painter->writeWithColors(0, i, QString::fromUtf8(decorations.vertical), frameFg, frameBg);
        }
        if (p->borders & Qt::RightEdge) {
            painter->writeWithColors(w - 1, i, QString::fromUtf8(decorations.vertical), frameFg, frameBg);
        }
    }
    if (p->borders & Qt::TopEdge && (p->options & CloseButton) && active) {
        painter->writeWithColors(2, 0, QStringLiteral("["), frameFg, frameBg);
        painter->writeWithColors(3, 0, QStringLiteral("■"), buttonFg, buttonBg);
        painter->writeWithColors(4, 0, QStringLiteral("]"), frameFg, frameBg);
    }
}

QVector<ZMenuItem> ZWindow::systemMenu() {
    auto *const p = tuiwidgets_impl();

    QVector<ZMenuItem> ret;

    if (options() & MoveOption) {
        ret.append(ZMenuItem{QStringLiteral("<m>M</m>ove"), QString(), QStringLiteral("ZWindowInteractiveMove"), {}});
    }

    if (options() & ResizeOption) {
        ret.append(ZMenuItem{QStringLiteral("<m>R</m>esize"), QString(), QStringLiteral("ZWindowInteractiveResize"), {}});
    }

    if (options() & AutomaticOption) {
        ret.append(ZMenuItem{QStringLiteral("<m>A</m>utomatic"), QString(), QStringLiteral("ZWindowAutomaticPlacement"), {}});
    }

    if (options() & CloseOption) {
        if (ret.size()) {
            ret.append(ZMenuItem{});
        }
        ret.append(ZMenuItem{QStringLiteral("<m>C</m>lose"), QString(), QStringLiteral("ZWindowClose"), {}});
    }

    return ret;
}

void ZWindow::closeEvent(ZCloseEvent *event) {
    (void)event;
}

void ZWindow::keyEvent(ZKeyEvent *event) {
    if (event->key() == Qt::Key_Tab && (event->modifiers() == 0 || event->modifiers() == Qt::ShiftModifier)) {
        ZTerminal *term = terminal();
        if (term) {
            ZWidget *f = term->focusWidget();
            if (f && isAncestorOf(f)) {
                if (event->modifiers() == Qt::ShiftModifier) {
                    f->prevFocusable()->setFocus();
                } else {
                    f->nextFocusable()->setFocus();
                }
            }
        }
    } else if (event->text() == QStringLiteral("-") && event->modifiers() == Qt::AltModifier) {
        if (!showSystemMenu()) {
            ZWidget::keyEvent(event);
        }
    } else {
        ZWidget::keyEvent(event);
    }
}

void ZWindowPrivate::ensureAutoPlacement() {
    if (pub()->parentWidget()) {
        ZWindowFacet *windowFacet = static_cast<ZWindowFacet*>(pub()->facet(ZWindowFacet::staticMetaObject));
        if (windowFacet) {
            if (!windowFacet->isManuallyPlaced() && !windowFacet->container()) {
                windowFacet->autoPlace(pub()->parentWidget()->geometry().size(), pub());
            }
        }
    }
}

void ZWindowPrivate::startInteractiveGeometry(ZWindow *pub) {
    interactiveInitialGeometry = pub->geometry();
    interactiveMode = true;
    auto *windowFacet = qobject_cast<ZWindowFacet*>(pub->facet(ZWindowFacet::staticMetaObject));
    if (windowFacet) {
        interactiveInitialManuallyPlaced = windowFacet->isManuallyPlaced();
        windowFacet->setManuallyPlaced(true);
    }
}

void ZWindowPrivate::cancelInteractiveGeometry(ZWindow *pub) {
    interactiveMode = false;
    pub->releaseKeyboard();
    pub->setGeometry(interactiveInitialGeometry);
    auto *windowFacet = qobject_cast<ZWindowFacet*>(pub->facet(ZWindowFacet::staticMetaObject));
    if (windowFacet) {
        windowFacet->setManuallyPlaced(interactiveInitialManuallyPlaced);
    }
    pub->update();
}

void ZWindowPrivate::finalizeInteractiveGeometry(ZWindow *pub) {
    interactiveMode = false;
    pub->releaseKeyboard();
    pub->update();
}

void ZWindow::close() {
    closeSkipCheck({});
}

void ZWindow::closeSkipCheck(QStringList skipChecks) {
    ZCloseEvent event(skipChecks);
    event.setAccepted(true);
    QCoreApplication::sendEvent(this, &event);
    if (event.isAccepted()) {
        setVisible(false);
        if (options() & DeleteOnClose) {
            deleteLater();
        }
    }
}

void ZWindow::resizeEvent(ZResizeEvent *event) {
    auto *const p = tuiwidgets_impl();

    ZWidget::resizeEvent(event);

    p->ensureAutoPlacement();
}

bool ZWindow::event(QEvent *event) {
    auto *const p = tuiwidgets_impl();

    if (event->type() == QEvent::ParentChange) {
        p->ensureAutoPlacement();
    } else if (event->type() == QEvent::ShowToParent) {
        p->ensureAutoPlacement();
    } else if (event->type() == ZEventType::close()) {
        closeEvent(static_cast<ZCloseEvent*>(event));
    }

    return ZWidget::event(event);
}

bool ZWindow::eventFilter(QObject *watched, QEvent *event) {
    return ZWidget::eventFilter(watched, event);
}

void ZWindow::timerEvent(QTimerEvent *event) {
    ZWidget::timerEvent(event);
}

void ZWindow::childEvent(QChildEvent *event) {
    ZWidget::childEvent(event);
}

void ZWindow::customEvent(QEvent *event) {
    ZWidget::customEvent(event);
}

void ZWindow::connectNotify(const QMetaMethod &signal) {
    ZWidget::connectNotify(signal);
}

void ZWindow::disconnectNotify(const QMetaMethod &signal) {
    ZWidget::disconnectNotify(signal);
}

void ZWindow::pasteEvent(ZPasteEvent *event) {
    ZWidget::pasteEvent(event);
}

void ZWindow::focusInEvent(ZFocusEvent *event) {
    ZWidget::focusInEvent(event);
}

void ZWindow::focusOutEvent(ZFocusEvent *event) {
    ZWidget::focusOutEvent(event);
}

void ZWindow::moveEvent(ZMoveEvent *event) {
    ZWidget::moveEvent(event);
}

ZWindowPrivate::ZWindowPrivate(ZWidget *pub) : ZWidgetPrivate(pub) {
}

TUIWIDGETS_NS_END
