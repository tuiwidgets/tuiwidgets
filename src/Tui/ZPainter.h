#ifndef TUIWIDGETS_ZPAINTER_INCLUDED
#define TUIWIDGETS_ZPAINTER_INCLUDED

#include <memory>

#include <QString>

#include <Tui/ZColor.h>
#include <Tui/tuiwidgets_internal.h>

class QRect;

TUIWIDGETS_NS_START

class ZPainterPrivate;
class ZImage;
class ZWidget;

class TUIWIDGETS_EXPORT ZPainter {
public:
    enum Attribute {
        Bold = (1<<0),
        Italic = (1<<1),
        Blink = (1<<4),
        Overline = (1<<5),
        Inverse = (1<<6),
        Strike = (1<<7),
        Underline = (1<<16),
        UnderlineDouble = (1<<17),
        UnderlineCurly = (1<<18)
    };
    Q_DECLARE_FLAGS(Attributes, Attribute)

public:
    ZPainter(const ZPainter &other);
    virtual ~ZPainter();

public:
    ZPainter translateAndClip(QRect transform);
    ZPainter translateAndClip(int x, int y, int width, int height);

    void writeWithColors(int x, int y, QString string, ZColor fg, ZColor bg);
    void writeWithAttributes(int x, int y, QString string, ZColor fg, ZColor bg, Attributes attr);
    void clear(ZColor fg, ZColor bg, Attributes attr = 0);
    void clearRect(int x, int y, int width, int height, ZColor fg, ZColor bg, Attributes attr = 0);

    void drawImage(int x, int y, const ZImage& image, int sx=0, int sy = 0, int width = -1, int height = -1);

    void setCursor(int x, int y);

    void setWidget(ZWidget *widget);

private:
    friend class ZTerminal;
    friend class ZImage;
    ZPainter(std::unique_ptr<ZPainterPrivate> impl);

protected:
    std::unique_ptr<ZPainterPrivate> tuiwidgets_pimpl_ptr;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZPainter)
};
Q_DECLARE_OPERATORS_FOR_FLAGS(ZPainter::Attributes)

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZPAINTER_INCLUDED
