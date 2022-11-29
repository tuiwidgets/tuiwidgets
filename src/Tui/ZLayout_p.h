// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZLAYOUT_P_INCLUDED
#define TUIWIDGETS_ZLAYOUT_P_INCLUDED

#include <Tui/ZLayout.h>

#include <QHash>
#include <QSet>

TUIWIDGETS_NS_START

class ZLayoutPrivate {
public:
    virtual ~ZLayoutPrivate();

public:
    class LayoutGenData {
    public:
        int lastSeenLayoutGeneration = 0;
        QSet<ZWidget*> alreadyDone;

        void reset();
    };
    static thread_local QHash<ZTerminal*, LayoutGenData> layoutGenData;

    static void markAsAlreadyLayouted(ZTerminal *term, ZWidget *w);
    static bool alreadyLayoutedInThisGeneration(ZTerminal *term, ZWidget *w);

    static bool ensureLayoutGenData(ZTerminal *term);
};

namespace Private {

struct DeleteUnlessLayout {
    void operator()(ZLayoutItem *item) {
        if (!item->layout()) {
            delete item;
        }
    }
};

}

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZLAYOUT_P_INCLUDED
