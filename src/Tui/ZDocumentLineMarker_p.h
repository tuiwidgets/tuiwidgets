// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZDOCUMENTLINEMARKER_P_INCLUDED
#define TUIWIDGETS_ZDOCUMENTLINEMARKER_P_INCLUDED

#include <Tui/ZDocumentLineMarker.h>

#include <Tui/ListNode_p.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

struct LineMarkerToDocumentTag;


class ZDocumentLineMarkerPrivate {
public:
    ZDocumentLineMarkerPrivate(ZDocumentLineMarker *pub, ZDocumentPrivate *doc, int line);
    ~ZDocumentLineMarkerPrivate();

public:
    int line = 0;
    ZDocumentPrivate *doc = nullptr;

public: // For use by Document
    ListNode<ZDocumentLineMarkerPrivate> markersList;
    bool changed = false;

public:
    ZDocumentLineMarker *pub_ptr;
    TUIWIDGETS_DECLARE_PUBLIC(ZDocumentLineMarker)
};

template<>
struct ListTrait<LineMarkerToDocumentTag> {
    static constexpr auto offset = &ZDocumentLineMarkerPrivate::markersList;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZDOCUMENTLINEMARKER_P_INCLUDED
