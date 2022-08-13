// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZTEXTLAYOUT_INCLUDED
#define TUIWIDGETS_ZTEXTLAYOUT_INCLUDED

#include <QRect>
#include <QVector>

#include <Tui/ZFormatRange.h>
#include <Tui/ZValuePtr.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZPainter;
class ZTextMetrics;
class ZTextOption;
class ZTextStyle;

class ZTextLineRef;

class ZTextLayoutPrivate;

class TUIWIDGETS_EXPORT ZTextLayout {
public:
    explicit ZTextLayout(ZTextMetrics metrics);
    explicit ZTextLayout(ZTextMetrics metrics, const QString &text);
    ZTextLayout(const ZTextLayout&);
    ~ZTextLayout();

    ZTextLayout &operator=(const ZTextLayout&);

public:
    enum CursorMode : int {
        SkipCharacters = 0,
        SkipWords = 1
    };

    enum Edge : int {
        Leading = 0,
        Trailing = 1
    };

public:
    QString text() const;
    void setText(const QString &text);

    const ZTextOption &textOption() const;
    void setTextOption(const ZTextOption &option);

    void beginLayout();
    void doLayout(int width);
    void endLayout();
    ZTextLineRef createLine();

    void draw(ZPainter painter, const QPoint &pos, ZTextStyle style, const ZTextStyle *formattingChars = nullptr,
              const QVector<ZFormatRange> &ranges = QVector<ZFormatRange>()) const;
    void showCursor(ZPainter painter, const QPoint &pos, int cursorPosition) const;

    QRect boundingRect() const;
    int lineCount() const;
    int maximumWidth() const;
    ZTextLineRef lineAt(int i) const;
    ZTextLineRef lineForTextPosition(int pos) const;

    int previousCursorPosition(int oldPos, CursorMode mode = SkipCharacters) const;
    int nextCursorPosition(int oldPos, CursorMode mode = SkipCharacters) const;
    bool isValidCursorPosition(int pos) const;

private:
    int lineNumberForTextPosition(int pos) const;
    void layoutLine(int index, int width);

private:
    friend class ZTextLineRef;
    ZValuePtr<ZTextLayoutPrivate> tuiwidgets_pimpl_ptr;

    TUIWIDGETS_DECLARE_PRIVATE(ZTextLayout)
};

class TUIWIDGETS_EXPORT ZTextLineRef {
public:
    ZTextLineRef();
    ZTextLineRef(const ZTextLineRef&);
    ~ZTextLineRef();

    ZTextLineRef &operator=(const ZTextLineRef&);

public:
    bool isValid() const;

    void draw(ZPainter painter, const QPoint &pos, ZTextStyle color, ZTextStyle formattingChars,
              const QVector<ZFormatRange> &ranges = QVector<ZFormatRange>()) const;

    int cursorToX(int *cursorPos, ZTextLayout::Edge edge) const;
    int cursorToX(int cursorPos, ZTextLayout::Edge edge) const;
    int xToCursor(int x) const;

    void setPosition(const QPoint &pos);
    QPoint position() const;
    int x() const;
    int y() const;
    int width() const;
    int height() const;
    QRect rect() const;

    int lineNumber() const;
    int textStart() const;
    int textLength() const;

    void setLineWidth(int width);

private:
    friend class ZTextLayout;
    ZTextLineRef(ZTextLayout *layout, int index);

    ZTextLayout *_layout = nullptr;
    int _index = -1;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZTEXTLAYOUT_INCLUDED
