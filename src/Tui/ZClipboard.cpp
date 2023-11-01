// SPDX-License-Identifier: BSL-1.0

#include "ZClipboard.h"
#include "ZClipboard_p.h"

#include <QRect>

TUIWIDGETS_NS_START

ZClipboard::ZClipboard() : tuiwidgets_pimpl_ptr(std::make_unique<ZClipboardPrivate>()) {
}

ZClipboard::ZClipboard(std::unique_ptr<ZClipboardPrivate> pimpl)
    : tuiwidgets_pimpl_ptr(move(pimpl))
{
}

ZClipboard::~ZClipboard() {
}

void ZClipboard::clear() {
    auto *const p = tuiwidgets_impl();
    p->contents = QStringLiteral("");
}

QString ZClipboard::contents() const {
    auto *const p = tuiwidgets_impl();
    return p->contents;
}

void ZClipboard::setContents(const QString &text) {
    auto *const p = tuiwidgets_impl();
    if (p->contents != text) {
        p->contents = text;
        contentsChanged();
    }
}


bool ZClipboard::event(QEvent *event) {
    return QObject::event(event);
}

bool ZClipboard::eventFilter(QObject *watched, QEvent *event) {
    return QObject::eventFilter(watched, event);
}

void ZClipboard::timerEvent(QTimerEvent *event) {
    return QObject::timerEvent(event);
}

void ZClipboard::childEvent(QChildEvent *event) {
    return QObject::childEvent(event);
}

void ZClipboard::customEvent(QEvent *event) {
    return QObject::customEvent(event);
}

void ZClipboard::connectNotify(const QMetaMethod &signal) {
    return QObject::connectNotify(signal);
}

void ZClipboard::disconnectNotify(const QMetaMethod &signal) {
    return QObject::disconnectNotify(signal);
}

ZClipboardPrivate::~ZClipboardPrivate() = default;

TUIWIDGETS_NS_END
