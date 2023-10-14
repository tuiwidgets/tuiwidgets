// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZDOCUMENTSNAPSHOT_INCLUDED
#define TUIWIDGETS_ZDOCUMENTSNAPSHOT_INCLUDED

#include <memory>

#include <QVector>

#include <Tui/ZDocument.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZDocumentSnapshotPrivate;
class TUIWIDGETS_EXPORT ZDocumentSnapshot {
public:
    ZDocumentSnapshot();
    ZDocumentSnapshot(const ZDocumentSnapshot &other);
    ~ZDocumentSnapshot();

    ZDocumentSnapshot &operator=(const ZDocumentSnapshot &other);

public:
    int lineCount() const;
    QString line(int line) const;
    int lineCodeUnits(int line) const;
    unsigned lineRevision(int line) const;
    std::shared_ptr<ZDocumentLineUserData> lineUserData(int line) const;

    unsigned revision() const;
    bool isUpToDate() const;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZDocumentSnapshot)
    std::shared_ptr<ZDocumentSnapshotPrivate> tuiwidgets_pimpl_ptr;
};


TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZDOCUMENTSNAPSHOT_INCLUDED
