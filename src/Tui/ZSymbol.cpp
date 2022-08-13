// SPDX-License-Identifier: BSL-1.0

#include "ZSymbol.h"

#include <mutex>

#include <QDebug>
#include <QHash>

TUIWIDGETS_NS_START

static QHash<QString, int> &ZSymbol_table() {
    static QHash<QString, int> data;
    return data;
}
static QList<QString> &ZSymbol_reverse() {
    static QList<QString> data;
    return data;
}


QString ZSymbol::toString() const {
    return ZSymbol_reverse().at(id + 1);
}

int ZSymbol::lookup(QString str, bool create) {
    static int max = 0;
    static std::mutex mut;

    if (str.isEmpty()) {
        return 0;
    }

    std::lock_guard<std::mutex> g(mut);

    auto &table = ZSymbol_table();
    auto it = table.find(str);
    if (it != table.end()) {
        return it.value();
    }

    if (create) {
        ++max;
        table.insert(str, max);
        ZSymbol_reverse().append(str);
        return max;
    }

    return 0;
}

QDebug operator<<(QDebug dbg, const ZSymbol &sym) {
    dbg << sym.toString();
    return dbg;
}

TUIWIDGETS_NS_END
