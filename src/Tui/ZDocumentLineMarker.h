// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZDOCUMENTLINEMARKER_INCLUDED
#define TUIWIDGETS_ZDOCUMENTLINEMARKER_INCLUDED

#include <memory>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZDocument;
class ZDocumentPrivate;

class ZDocumentLineMarkerPrivate;

class TUIWIDGETS_EXPORT ZDocumentLineMarker {
public:
    explicit ZDocumentLineMarker(ZDocument *doc);
    ZDocumentLineMarker(const ZDocumentLineMarker &other);
    explicit ZDocumentLineMarker(ZDocument *doc, int line);
    ~ZDocumentLineMarker();

    ZDocumentLineMarker &operator=(const ZDocumentLineMarker &other);

public:
    int line() const;
    void setLine(int line);

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZDocumentLineMarker)
    std::unique_ptr<ZDocumentLineMarkerPrivate> tuiwidgets_pimpl_ptr;
};


TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZDOCUMENTLINEMARKER_INCLUDED
