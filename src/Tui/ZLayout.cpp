#include "ZLayout.h"
#include "ZLayout_p.h"

#include <QCoreApplication>
#include <QEvent>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QThread>

#include <Tui/ZSymbol.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZWidget.h>


TUIWIDGETS_NS_START

ZLayout::ZLayout(QObject *parent) : QObject(parent), tuiwidgets_pimpl_ptr(std::make_unique<ZLayoutPrivate>())
{
}

ZLayout::~ZLayout() {
}

void ZLayout::widgetEvent(QEvent *event) {
    if (event->type() == ZEventType::resize()
          || ZOtherChangeEvent::match(event, TUISYM_LITERAL("terminal"))) {
        ZWidget *w = widget();
        if (w) {
            ZTerminal *term = w->terminal();
            if (term) {
                term->requestLayout(w);
            }
        }
    } else if (event->type() == QEvent::LayoutRequest) {
        ZWidget *w = widget();
        ZTerminal *term = w->terminal();
        ZWidget *chainRoot = w->resolveSizeHintChain();

        // ensure that the root of the layout chain gets to layout first
        if (chainRoot != w && !ZLayoutPrivate::alreadyLayoutedInThisGeneration(term, w)) {
            // will be marked as done in the chainRoot's event handler before doing much else,
            // given that that chainRoot->resolveSizeHintChain() == chainRoot
            QEvent request(QEvent::LayoutRequest);
            QCoreApplication::sendEvent(chainRoot, &request);
        }
        // if this layout is still pending now, layout locally by doing an additional layout starting here.
        // This could happen if i.e. if sizeHint does not change but positions need to be updated
        // i.e. one widget was replaced with a widget of the same size
        if (!ZLayoutPrivate::alreadyLayoutedInThisGeneration(term, w)) {
            ZLayoutPrivate::markAsAlreadyLayouted(term, w);
            setGeometry(w->layoutArea());
        }
    }
}

ZWidget *ZLayout::widget() {
    QObject *p = parent();
    while (p) {
        ZWidget *w = qobject_cast<ZWidget*>(p);
        if (w) return w;
        p = p->parent();
    }
    return nullptr;
}

QSize ZLayout::sizeHint() const {
    return QSize();
}

SizePolicy ZLayout::sizePolicyH() const {
    return SizePolicy::Preferred;
}

SizePolicy ZLayout::sizePolicyV() const {
    return SizePolicy::Preferred;
}

bool ZLayout::isVisible() const {
    return true;
}

bool ZLayout::event(QEvent *event) {
    return QObject::event(event);
}

bool ZLayout::eventFilter(QObject *watched, QEvent *event) {
    return QObject::eventFilter(watched, event);
}

void ZLayout::relayout() {
    ZWidget *w = widget();
    if (w) {
        auto *term = w->terminal();
        if (term) {
            term->requestLayout(w);
        }
    }
}

void ZLayout::timerEvent(QTimerEvent *event) {
    QObject::timerEvent(event);
}

void ZLayout::childEvent(QChildEvent *event) {
    QObject::childEvent(event);
}

void ZLayout::customEvent(QEvent *event) {
    QObject::customEvent(event);
}

void ZLayout::connectNotify(const QMetaMethod &signal) {
    // XXX needs to be thread-safe
    QObject::connectNotify(signal);
}

void ZLayout::disconnectNotify(const QMetaMethod &signal) {
    // XXX needs to be thread-safe
    QObject::disconnectNotify(signal);
}


thread_local QHash<ZTerminal*, ZLayoutPrivate::LayoutGenData> ZLayoutPrivate::layoutGenData;

void ZLayoutPrivate::markAsAlreadyLayouted(ZTerminal *term, ZWidget *w) {
    if (!ensureLayoutGenData(term)) {
        return;
    }
    layoutGenData[term].alreadyDone.insert(w);
}

bool ZLayoutPrivate::alreadyLayoutedInThisGeneration(ZTerminal *term, ZWidget *w) {
    if (!ensureLayoutGenData(term)) {
        return true;
    }

    return layoutGenData[term].alreadyDone.contains(w);
}

bool ZLayoutPrivate::ensureLayoutGenData(ZTerminal *term) {
    if (!layoutGenData.contains(term)) {
        if (QThread::currentThread() != term->thread()) {
            qWarning("ZLayout: ZTerminal thread does not match current thread.");
            return false;
        }
        // create entry
        layoutGenData[term];
        QObject::connect(term, &QObject::destroyed, [term]{
            layoutGenData.remove(term);
        });
    }

    auto& lgd = layoutGenData[term];
    if (lgd.lastSeenLayoutGeneration != term->currentLayoutGeneration()) {
        lgd.reset();
    }

    return true;
}

void ZLayoutPrivate::LayoutGenData::reset() {
    alreadyDone.clear();
}


TUIWIDGETS_NS_END
