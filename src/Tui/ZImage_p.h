#ifndef TUIWIDGETS_ZIMAGE_P_INCLUDED
#define TUIWIDGETS_ZIMAGE_P_INCLUDED

#include <QSharedData>

#include <termpaint.h>

#include <Tui/ZImage.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZImageData : public QSharedData {
public:
    ZImageData(termpaint_terminal *terminal, int width, int height);
    ZImageData(termpaint_terminal *terminal, termpaint_surface *surface);
    ZImageData(const ZImageData& other);
    ZImageData(ZImageData&& other) = delete;
    ZImageData& operator=(const ZImageData& other);
    ZImageData& operator=(ZImageData&& other) = delete;
    virtual ~ZImageData();

public:
    bool hasPainters() const { return !hasPainter.expired(); };

    // back door
    static ZImageData *get(ZImage *image) { return image->tuiwidgets_pimpl_ptr.data(); }
    static const ZImageData *get(const ZImage *image) { return image->tuiwidgets_pimpl_ptr.data(); }
    static ZImage createForTesting(termpaint_terminal *terminal, int width, int height);

public:
    std::weak_ptr<char> hasPainter;
    termpaint_terminal *terminal;
    termpaint_surface *surface;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZIMAGE_P_INCLUDED
