// SPDX-License-Identifier: BSL-1.0

#ifndef VCHECK_QOBJECT_H
#define VCHECK_QOBJECT_H

#include <QObject>

#include "catchwrapper.h"

#include "vcheck_base.h"

VIRTUAL_INDEX_PROBE(QObject, QObject, event)
VIRTUAL_INDEX_PROBE(QObject, QObject, eventFilter)
VIRTUAL_INDEX_PROBE(QObject, QObject, timerEvent)
VIRTUAL_INDEX_PROBE(QObject, QObject, childEvent)
VIRTUAL_INDEX_PROBE(QObject, QObject, customEvent)
VIRTUAL_INDEX_PROBE(QObject, QObject, connectNotify)
VIRTUAL_INDEX_PROBE(QObject, QObject, disconnectNotify)

static inline void checkQObjectOverrides(QObject *base, QObject* derived) {
    CHECK_VIRTUAL(QObject, event);
    CHECK_VIRTUAL(QObject, eventFilter);
    CHECK_VIRTUAL(QObject, timerEvent);
    CHECK_VIRTUAL(QObject, childEvent);
    CHECK_VIRTUAL(QObject, customEvent);
    CHECK_VIRTUAL(QObject, connectNotify);
    CHECK_VIRTUAL(QObject, disconnectNotify);
}

#endif // VCHECK_QOBJECT_H
