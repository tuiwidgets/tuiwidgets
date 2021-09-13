#ifndef TUIWIDGETS_ZSTYLEDTEXTLINE_P_INCLUDED
#define TUIWIDGETS_ZSTYLEDTEXTLINE_P_INCLUDED

#include <QVector>

#include <Tui/ZStyledTextLine.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZStyledTextLinePrivate {
public:
    void ensureCache() const;

public:
    QString markup;
    QString text;
    ZTextStyle baseStyle;
    ZTextStyle mnemonicStyle;

    struct StylePos {
        int startIndex;
        ZTextStyle style;
    };

    mutable bool cached = false;
    mutable QString textFromMarkup;
    mutable QVector<StylePos> styles;
    mutable QString mnemonic;
    mutable bool parsingError = false;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZSTYLEDTEXTLINE_P_INCLUDED
