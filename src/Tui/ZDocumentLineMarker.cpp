// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZDocumentLineMarker.h>
#include <Tui/ZDocumentLineMarker_p.h>

#include <Tui/ZDocument_p.h>

TUIWIDGETS_NS_START

ZDocumentLineMarkerPrivate::ZDocumentLineMarkerPrivate(ZDocumentLineMarker *pub, ZDocumentPrivate *doc, int line)
    : line(line), doc(doc), pub_ptr(pub) {
    doc->registerLineMarker(this);
}


ZDocumentLineMarkerPrivate::~ZDocumentLineMarkerPrivate() {
    doc->unregisterLineMarker(this);
}

ZDocumentLineMarker::ZDocumentLineMarker(ZDocument *doc) : ZDocumentLineMarker(doc, 0) {
}

ZDocumentLineMarker::ZDocumentLineMarker(const ZDocumentLineMarker &other)
    : tuiwidgets_pimpl_ptr(new ZDocumentLineMarkerPrivate(this,
                                                          other.tuiwidgets_impl()->doc, other.tuiwidgets_impl()->line))
{
}

ZDocumentLineMarker::ZDocumentLineMarker(ZDocument *doc, int line)
    : tuiwidgets_pimpl_ptr(new ZDocumentLineMarkerPrivate(this, ZDocumentPrivate::get(doc), line))
{
}

ZDocumentLineMarker::~ZDocumentLineMarker() {
}

ZDocumentLineMarker &ZDocumentLineMarker::operator=(const ZDocumentLineMarker &other) {
    auto *const p = tuiwidgets_impl();
    auto *const otherP = other.tuiwidgets_impl();

    if (p->doc != otherP->doc) {
        p->doc->unregisterLineMarker(p);
        p->doc = otherP->doc;
        p->doc->registerLineMarker(p);
        p->changed = true;
        p->doc->scheduleChangeSignals();
    }

    if (p->line != otherP->line) {
        p->line = otherP->line;
        p->changed = true;
        p->doc->scheduleChangeSignals();
    }

    return *this;
}

int ZDocumentLineMarker::line() const {
    auto *const p = tuiwidgets_impl();
    return p->line;
}

void ZDocumentLineMarker::setLine(int line) {
    auto *const p = tuiwidgets_impl();
    line = std::max(std::min(line, p->doc->pub()->lineCount() - 1), 0);

    if (p->line != line) {
        p->line = line;
        p->changed = true;
        p->doc->scheduleChangeSignals();
    }
}

TUIWIDGETS_NS_END
