// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZDOCUMENTSNAPSHOT_P_INCLUDED
#define TUIWIDGETS_ZDOCUMENTSNAPSHOT_P_INCLUDED

#include <Tui/ZDocument_p.h>

TUIWIDGETS_NS_START

class ZDocumentSnapshotPrivate {
public:
    static ZDocumentSnapshotPrivate *get(ZDocumentSnapshot *doc);
    static const ZDocumentSnapshotPrivate *get(const ZDocumentSnapshot *doc);

public:
    unsigned revision = -1;
    QVector<LineData> lines;
    std::shared_ptr<std::atomic<unsigned>> revisionShared;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZDOCUMENTSNAPSHOT_P_INCLUDED
