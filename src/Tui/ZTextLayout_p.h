#ifndef TUIWIDGETS_ZTEXTLAYOUT_P_INCLUDED
#define TUIWIDGETS_ZTEXTLAYOUT_P_INCLUDED

#include <Tui/ZTextLayout.h>

#include <Tui/ZTextMetrics.h>
#include <Tui/ZTextOption.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZTextLayoutPrivate {
public:
    class TextRun {
    public:
        enum { COPY, TAB, SPACES, SPECIAL_BYTE_OR_CHAR } type = COPY;
        int x = -1;
        int offset = -1;
        int endIndex = -1;
        int width = -1;
    };

    class LineData {
    public:
        int offset = -1;
        int endIndex = -1;
        QPoint pos;
        QVector<TextRun> textRuns;
        int maxWidth = -1;
    };

public:
    ZTextLayoutPrivate(ZTextMetrics metrics) : metrics(metrics) {}
    ZTextLayoutPrivate(ZTextMetrics metrics, const QString &text) : metrics(metrics), text(text) {}

public:
    ZTextMetrics metrics;

    ZTextOption textOption;

    QString text;
    QVector<unsigned short> columns; // for each code unit the column (relative to pos in LineData) after the cluster
    QVector<LineData> lines;
    int nextIndex = -1;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZTEXTLAYOUT_P_INCLUDED
