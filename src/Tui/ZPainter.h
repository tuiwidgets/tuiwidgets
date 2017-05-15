#ifndef TUIWIDGETS_ZPAINTER_INCLUDED
#define TUIWIDGETS_ZPAINTER_INCLUDED

#include <memory>

#include <QString>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZPainterPrivate;

class TUIWIDGETS_EXPORT ZPainter {
public:
    ZPainter(const ZPainter &other);
    virtual ~ZPainter();

public:
    ZPainter translateAndClip(int x, int y, int width, int height);

    void writeWithColors(int x, int y, QString string, int fg, int bg);
    void clear(int bg);
    void flush();

private:
    friend class ZTerminal;
    ZPainter(std::unique_ptr<ZPainterPrivate> impl);

protected:
    std::unique_ptr<ZPainterPrivate> tuiwidgets_pimpl_ptr;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZPainter)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZPAINTER_INCLUDED
