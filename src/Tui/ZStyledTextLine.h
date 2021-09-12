#ifndef TUIWIDGETS_ZSTYLEDTEXTLINE_INCLUDED
#define TUIWIDGETS_ZSTYLEDTEXTLINE_INCLUDED

#include <QString>

#include <Tui/ZPainter.h>
#include <Tui/ZTextMetrics.h>
#include <Tui/ZTextStyle.h>
#include <Tui/ZValuePtr.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZStyledTextLinePrivate;

class TUIWIDGETS_EXPORT ZStyledTextLine {
public:
    ZStyledTextLine();
    ZStyledTextLine(const ZStyledTextLine&);
    ZStyledTextLine(ZStyledTextLine&&);
    ~ZStyledTextLine();

    ZStyledTextLine &operator=(const ZStyledTextLine&);
    ZStyledTextLine &operator=(ZStyledTextLine&&);

public:
    void setMarkup(const QString &markup);
    QString markup() const;
    void setText(const QString &text);
    QString text() const;

    void setBaseStyle(ZTextStyle newBaseStyle);
    void setMnemonicStyle(ZTextStyle newBaseStyle, ZTextStyle newMnemonicStyle);

    QString mnemonic() const;
    int width(const ZTextMetrics &metrics) const;
    void write(ZPainter* painter, int x, int y, int width) const;

protected:
    ZValuePtr<ZStyledTextLinePrivate> tuiwidgets_pimpl_ptr;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZStyledTextLine)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZSTYLEDTEXTLINE_INCLUDED
