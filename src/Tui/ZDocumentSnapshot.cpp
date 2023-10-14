// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZDocumentSnapshot.h>
#include <Tui/ZDocumentSnapshot_p.h>

TUIWIDGETS_NS_START

ZDocumentSnapshotPrivate *ZDocumentSnapshotPrivate::get(ZDocumentSnapshot *doc) {
    return doc->tuiwidgets_impl();
}

const ZDocumentSnapshotPrivate *ZDocumentSnapshotPrivate::get(const ZDocumentSnapshot *doc) {
    return doc->tuiwidgets_impl();
}


ZDocumentSnapshot::ZDocumentSnapshot() : tuiwidgets_pimpl_ptr(std::make_shared<ZDocumentSnapshotPrivate>()) {
}

ZDocumentSnapshot::ZDocumentSnapshot(const ZDocumentSnapshot &other) : tuiwidgets_pimpl_ptr(other.tuiwidgets_pimpl_ptr) {
}

ZDocumentSnapshot::~ZDocumentSnapshot() {
}

ZDocumentSnapshot &ZDocumentSnapshot::operator=(const ZDocumentSnapshot &other) {
    tuiwidgets_pimpl_ptr = other.tuiwidgets_pimpl_ptr;
    return *this;
}

int ZDocumentSnapshot::lineCount() const {
    return tuiwidgets_impl()->lines.size();
}

QString ZDocumentSnapshot::line(int line) const {
    return tuiwidgets_impl()->lines[line].chars;
}

int ZDocumentSnapshot::lineCodeUnits(int line) const {
    return tuiwidgets_impl()->lines[line].chars.size();
}

unsigned ZDocumentSnapshot::lineRevision(int line) const {
    return tuiwidgets_impl()->lines[line].revision;
}

std::shared_ptr<ZDocumentLineUserData> ZDocumentSnapshot::lineUserData(int line) const {
    return tuiwidgets_impl()->lines[line].userData;
}

unsigned ZDocumentSnapshot::revision() const {
    return tuiwidgets_impl()->revision;
}

bool ZDocumentSnapshot::isUpToDate() const {
    return tuiwidgets_impl()->revision == tuiwidgets_impl()->revisionShared->load(std::memory_order_relaxed);
}

TUIWIDGETS_NS_END
