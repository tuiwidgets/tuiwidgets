// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZDOCUMENTCURSOR_P_INCLUDED
#define TUIWIDGETS_ZDOCUMENTCURSOR_P_INCLUDED

#include <Tui/ZDocumentCursor.h>

#include <Tui/ListNode_p.h>


TUIWIDGETS_NS_START

struct TextCursorToDocumentTag;

class ZDocumentCursorPrivate {
public:
    ZDocumentCursorPrivate(ZDocumentCursor *pub, ZDocumentPrivate *doc,
                           std::function<ZTextLayout(int line, bool wrappingAllowed)> createTextLayout);
    ~ZDocumentCursorPrivate();

public:
    void updateVerticalMovementColumn(const ZTextLayout &layoutForCursorLine);
    void scheduleChangeSignal();

public:
    int cursorCodeUnit = 0;
    int cursorLine = 0;
    int anchorCodeUnit = 0;
    int anchorLine = 0;
    int verticalMovementColumn = 0;

    ZDocumentPrivate *doc;
    std::function<ZTextLayout(int line, bool wrappingAllowed)> createTextLayout;

public: // For use by Document
    ListNode<ZDocumentCursorPrivate> markersList;
    bool changed = false;

public:
    ZDocumentCursor *pub_ptr;
    TUIWIDGETS_DECLARE_PUBLIC(ZDocumentCursor)
};

template<>
struct ListTrait<TextCursorToDocumentTag> {
    static constexpr auto offset = &ZDocumentCursorPrivate::markersList;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZDOCUMENTCURSOR_P_INCLUDED
