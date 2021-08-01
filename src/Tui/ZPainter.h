#ifndef TUIWIDGETS_ZPAINTER_INCLUDED
#define TUIWIDGETS_ZPAINTER_INCLUDED

#include <memory>
#include <string>
#if defined(__cpp_lib_string_view) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
#include <string_view>
#endif

#include <QString>

#include <Tui/ZCommon.h>
#include <Tui/ZColor.h>
#include <Tui/tuiwidgets_internal.h>

class QRect;

TUIWIDGETS_NS_START

class ZImage;
class ZTextMetrics;
class ZWidget;

class ZPainterPrivate;
class TUIWIDGETS_EXPORT ZPainter {
public:
    using Attribute = ::TUIWIDGETS_NS_FULL::ZTextAttribute;
    using Attributes = ::TUIWIDGETS_NS_FULL::ZTextAttributes;

    static constexpr int Erased = ::TUIWIDGETS_NS_FULL::Erased;

public:
    ZPainter(const ZPainter &other);
    virtual ~ZPainter();

public:
    ZPainter translateAndClip(QRect transform);
    ZPainter translateAndClip(int x, int y, int width, int height);

    void writeWithColors(int x, int y, const QString &string, ZColor fg, ZColor bg);
    void writeWithColors(int x, int y, const QChar *string, int size, ZColor fg, ZColor bg);
    void writeWithColors(int x, int y, const char16_t *string, int size, ZColor fg, ZColor bg);
    void writeWithColors(int x, int y, const char *stringUtf8, int utf8CodeUnits, ZColor fg, ZColor bg);
    void writeWithAttributes(int x, int y, const QString &string, ZColor fg, ZColor bg, Attributes attr);
    void writeWithAttributes(int x, int y, const QChar *string, int size, ZColor fg, ZColor bg, Attributes attr);
    void writeWithAttributes(int x, int y, const char16_t *string, int size, ZColor fg, ZColor bg, Attributes attr);
    void writeWithAttributes(int x, int y, const char *stringUtf8, int utf8CodeUnits, ZColor fg, ZColor bg, Attributes attr);

    // Wrappers for more modern types:
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
    template <typename QSTRINGVIEW, Private::enable_if_same_remove_cvref<QSTRINGVIEW, QStringView> = 0>
    TUIWIDGETS_ABI_FORCE_INLINE void writeWithColors(int x, int y, QSTRINGVIEW string, ZColor fg, ZColor bg) {
        writeWithColors(x, y, string.data(), string.size(), fg, bg);
    }
    template <typename QSTRINGVIEW, Private::enable_if_same_remove_cvref<QSTRINGVIEW, QStringView> = 0>
    TUIWIDGETS_ABI_FORCE_INLINE void writeWithAttributes(int x, int y, QSTRINGVIEW string, ZColor fg, ZColor bg, Attributes attr) {
        writeWithAttributes(x, y, string.data(), string.size(), fg, bg, attr);
    }
#endif
#if defined(__cpp_lib_string_view) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
    template <typename U16STRINGVIEW, Private::enable_if_same_remove_cvref<U16STRINGVIEW, std::u16string_view> = 0>
    TUIWIDGETS_ABI_FORCE_INLINE void writeWithColors(int x, int y, U16STRINGVIEW string, ZColor fg, ZColor bg) {
        writeWithColors(x, y, string.data(), string.size(), fg, bg);
    }
    template <typename U16STRINGVIEW, Private::enable_if_same_remove_cvref<U16STRINGVIEW, std::u16string_view> = 0>
    TUIWIDGETS_ABI_FORCE_INLINE void writeWithAttributes(int x, int y, U16STRINGVIEW string, ZColor fg, ZColor bg, Attributes attr) {
        writeWithAttributes(x, y, string.data(), string.size(), fg, bg, attr);
    }

    // Assumes utf8 in string_view
    template <typename STRINGVIEW, Private::enable_if_same_remove_cvref<STRINGVIEW, std::string_view> = 0>
    TUIWIDGETS_ABI_FORCE_INLINE void writeWithColors(int x, int y, STRINGVIEW string, ZColor fg, ZColor bg) {
        writeWithColors(x, y, string.data(), string.size(), fg, bg);
    }
    template <typename STRINGVIEW, Private::enable_if_same_remove_cvref<STRINGVIEW, std::string_view> = 0>
    TUIWIDGETS_ABI_FORCE_INLINE void writeWithAttributes(int x, int y, STRINGVIEW string, ZColor fg, ZColor bg, Attributes attr) {
        writeWithAttributes(x, y, string.data(), string.size(), fg, bg, attr);
    }
#endif

    void clear(ZColor fg, ZColor bg, Attributes attr = {});
    void clearWithChar(ZColor fg, ZColor bg, int fillChar, Attributes attr = {});
    void clearRect(int x, int y, int width, int height, ZColor fg, ZColor bg, Attributes attr = {});
    void clearRectWithChar(int x, int y, int width, int height, ZColor fg, ZColor bg, int fillChar, Attributes attr = {});
    void setSoftwrapMarker(int x, int y);
    void clearSoftwrapMarker(int x, int y);

    void drawImage(int x, int y, const ZImage &sourceImage, int sourceX = 0, int sourceY = 0, int width = -1, int height = -1);

    void setForeground(int x, int y, ZColor fg);
    void setBackground(int x, int y, ZColor bg);

    void setCursor(int x, int y);

    void setWidget(ZWidget *widget);

    ZTextMetrics textMetrics();

private:
    friend class ZTerminal;
    friend class ZImage;
    friend class ZPainterPrivate;
    ZPainter(std::unique_ptr<ZPainterPrivate> impl);

protected:
    std::unique_ptr<ZPainterPrivate> tuiwidgets_pimpl_ptr;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZPainter)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZPAINTER_INCLUDED
