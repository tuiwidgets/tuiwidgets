#include "ZTextLayout.h"
#include "ZTextLayout_p.h"

#include <QList>
#include <QTextBoundaryFinder>

#include <Tui/ZFormatRange.h>
#include <Tui/ZPainter.h>
#include <Tui/ZTextStyle.h>


TUIWIDGETS_NS_START

ZTextLayout::ZTextLayout(ZTextMetrics metrics) : tuiwidgets_pimpl_ptr(ZTextLayoutPrivate(metrics))
{
}

ZTextLayout::ZTextLayout(ZTextMetrics metrics, const QString &text) : tuiwidgets_pimpl_ptr(ZTextLayoutPrivate(metrics, text))
{
}

ZTextLayout::ZTextLayout(const ZTextLayout&) = default;

ZTextLayout::~ZTextLayout() = default;

ZTextLayout &ZTextLayout::operator=(const ZTextLayout&) = default;

QString ZTextLayout::text() const {
    auto *const p = tuiwidgets_impl();
    return p->text;
}

void ZTextLayout::setText(const QString &text) {
    auto *const p = tuiwidgets_impl();
    p->text = text;
}

const ZTextOption &ZTextLayout::textOption() const {
    auto *const p = tuiwidgets_impl();
    return p->textOption;
}

void ZTextLayout::setTextOption(const ZTextOption &option) {
    auto *const p = tuiwidgets_impl();
    p->textOption = option;
}

void ZTextLayout::beginLayout() {
    auto *const p = tuiwidgets_impl();
    p->lines.clear();
    p->nextIndex = 0;
    p->columns.resize(p->text.size());
}

void ZTextLayout::doLayout(int width) {
    beginLayout();
    int y = 0;
    while (true) {
        ZTextLineRef line = createLine();
        line.setLineWidth(width);
        line.setPosition({0, y});

        if (!line.isValid()) {
            break;
        }
        ++y;
    }
    endLayout();
}

void ZTextLayout::endLayout() {
}

ZTextLineRef ZTextLayout::createLine() {
    auto *const p = tuiwidgets_impl();
    if (p->nextIndex >= p->text.size() && p->nextIndex != 0) {
        return ZTextLineRef(); // done marker
    }
    p->lines.append(ZTextLayoutPrivate::LineData());
    return ZTextLineRef(this, p->lines.size() - 1);
}

void ZTextLayout::draw(ZPainter painter, const QPoint &pos, ZTextStyle style, const ZTextStyle *formattingChars,
                      const QVector<ZFormatRange> &ranges) const {
    auto *const p = tuiwidgets_impl();
    if (!formattingChars) {
        formattingChars = &style;
    }
    for (int i = 0; i < p->lines.size(); i++) {
        lineAt(i).draw(painter, pos, style, *formattingChars, ranges);
    }
}

void ZTextLayout::showCursor(ZPainter painter, const QPoint &pos, int cursorPosition) const {
    auto *const p = tuiwidgets_impl();
    if (cursorPosition == 0) {
        if (p->lines.isEmpty()) {
            painter.setCursor(pos.x(), pos.y());
        } else {
            painter.setCursor(p->lines.front().pos.x() + pos.x(),
                              p->lines.front().pos.y() + pos.y());
        }
        return;
    }

    int lineNumber = lineNumberForTextPosition(cursorPosition);
    if (lineNumber < 0) {
        return;
    }
    if (cursorPosition == p->text.size()) {
        ZTextLineRef line = lineAt(p->lines.size() - 1);
        painter.setCursor(pos.x() + line.x() + line.width(), pos.y() + line.y());
    } else {
        ZTextLineRef tlr = lineAt(lineNumber);
        painter.setCursor(pos.x() + tlr.cursorToX(cursorPosition, ZTextLayout::Leading), pos.y() + tlr.y());
    }
}

QRect ZTextLayout::boundingRect() const {
    auto *const p = tuiwidgets_impl();
    if (p->lines.isEmpty()) {
        return QRect();
    }
    int x0 = p->lines.first().pos.x();
    int y0 = p->lines.first().pos.y();
    int x1 = x0;
    int y1 = y0;
    for (int i = 0; i < p->lines.size(); i++) {
        ZTextLineRef tlr = lineAt(i);
        x0 = std::min(x0, tlr.x());
        y0 = std::min(y0, tlr.y());

        x1 = std::max(x1, tlr.x() + tlr.width() - 1);
        y1 = std::max(y1, tlr.y());
    }
    return QRect(QPoint(x0, y0), QPoint(x1, y1));
}

int ZTextLayout::lineCount() const {
    auto *const p = tuiwidgets_impl();
    return p->lines.size();
}

int ZTextLayout::maximumWidth() const {
    auto *const p = tuiwidgets_impl();
    int res = 0;
    for (int i = 0; i < p->lines.size(); i++) {
        res = std::max(res, lineAt(i).width());
    }
    return res;
}

ZTextLineRef ZTextLayout::lineAt(int i) const {
    auto *const p = tuiwidgets_impl();
    if (i >= 0 && i < p->lines.size()) {
        return ZTextLineRef(const_cast<ZTextLayout*>(this), i);
    } else {
        return ZTextLineRef();
    }
}

int ZTextLayout::lineNumberForTextPosition(int pos) const {
    auto *const p = tuiwidgets_impl();
    if (!p->lines.size() || pos < 0 || pos > p->text.size()) {
        return -1;
    }
    if (pos == p->text.size()) {
        return p->lines.size() - 1;
    }
    auto it = std::lower_bound(p->lines.begin(), p->lines.end(), pos, [&] (const ZTextLayoutPrivate::LineData &a, int b) {
        return a.endIndex <= b;
    });
    if (it == p->lines.end()) {
        return p->lines.size() - 1;
    } else {
        return it - p->lines.begin();
    }
}

ZTextLineRef ZTextLayout::lineForTextPosition(int pos) const {
    return lineAt(lineNumberForTextPosition(pos));
}

int ZTextLayout::previousCursorPosition(int oldPos, ZTextLayout::CursorMode mode) const {
    auto *const p = tuiwidgets_impl();
    if (oldPos <= 0) {
        return 0;
    } else if (oldPos > p->text.size()) {
        return p->text.size();
    }

    if (mode == SkipWords) {
        QTextBoundaryFinder finder(QTextBoundaryFinder::Word, p->text);
        // skip runs of "spaces"
        int candidate = oldPos;
        do {
            oldPos = candidate;
            candidate = previousCursorPosition(oldPos, SkipCharacters);
        } while (candidate != oldPos && (candidate + 1 == oldPos) && p->text.at(candidate).isSpace());

        // skip word
        int prevPos;
        do {
            prevPos = oldPos;
            oldPos = previousCursorPosition(oldPos, SkipCharacters);
            finder.setPosition(oldPos);
        } while (prevPos != oldPos && !finder.isAtBoundary());

        return oldPos;
    } else {
        --oldPos;
        int column = p->columns[oldPos];
        int line = lineNumberForTextPosition(oldPos);
        while (oldPos >= 0 && p->columns[oldPos] == column && lineNumberForTextPosition(oldPos) == line) {
            --oldPos;
        }
        return oldPos + 1;
    }
}

int ZTextLayout::nextCursorPosition(int oldPos, ZTextLayout::CursorMode mode) const {
    auto *const p = tuiwidgets_impl();
    if (oldPos < 0) {
        return 0;
    } else if (oldPos >= p->text.size()) {
        return p->text.size();
    }

    if (mode == SkipWords) {
        QTextBoundaryFinder finder(QTextBoundaryFinder::Word, p->text);
        // skip runs of "spaces"
        int candidate = oldPos;
        do {
            oldPos = candidate;
            candidate = nextCursorPosition(oldPos, SkipCharacters);
        } while (candidate != oldPos && (candidate - 1 == oldPos) && p->text.at(oldPos).isSpace());

        // skip word
        int prevPos;
        do {
            prevPos = oldPos;
            oldPos = nextCursorPosition(oldPos, SkipCharacters);
            finder.setPosition(oldPos);
        } while (prevPos != oldPos && !finder.isAtBoundary());

        return oldPos;
    } else {
        int column = p->columns[oldPos];
        int line = lineNumberForTextPosition(oldPos);
        while (oldPos < p->text.size() && p->columns[oldPos] == column && lineNumberForTextPosition(oldPos) == line) {
            ++oldPos;
        }
        return oldPos;
    }
}

bool ZTextLayout::isValidCursorPosition(int pos) const {
    auto *const p = tuiwidgets_impl();
    if (pos < 0 || pos > p->text.size()) {
        return false;
    }
    if (pos == 0 || pos == p->text.size()) {
        return true;
    }

    return p->columns[pos] != p->columns[pos - 1] || lineNumberForTextPosition(pos) != lineNumberForTextPosition(pos - 1);
}

void ZTextLayout::layoutLine(int index, int width) {
    auto *const p = tuiwidgets_impl();
    if (index >= p->lines.size()) {
        return;
    }
    ZTextLayoutPrivate::LineData &ld = p->lines[index];
    ld.maxWidth = width;
    ld.offset = p->nextIndex;
    int offset = ld.offset;

    if (offset == p->text.size()) {
        ld.endIndex = p->nextIndex;
        p->nextIndex += 1;
        return;
    }

    auto textOptionFlags = p->textOption.flags();
    auto textOptionWrapMode = p->textOption.wrapMode();
    auto textOptionTabWidth = p->textOption.tabStopDistance();

    int column = 0;

    ZTextLayoutPrivate::TextRun run;
    run.type = ZTextLayoutPrivate::TextRun::COPY;
    run.offset = offset;
    run.x = column;

    auto wordWrap = [&] {
        if (run.type == ZTextLayoutPrivate::TextRun::COPY) {
            ZTextMetrics::ClusterSize cs = p->metrics.nextCluster(p->text, offset);
            if (cs.codeUnits == 1 && p->text[offset].unicode() == ' ') {
                // the unicode line wrapping (annex 14) expects spaces at the end of the line not to take visual space
                // and thus has break points only after a run of spaces is finished.
                // As text layout does not compress spaces every point in a run
                // of ascii spaces should count as a break opportunity as well.
                return;
            }
            QTextBoundaryFinder bndFinder(QTextBoundaryFinder::Line, p->text.data() + run.offset, std::max(offset + 2 /* allow for surrogate */, p->text.size()) - run.offset);
            bndFinder.setPosition(offset - run.offset);
            int boundary = bndFinder.isAtBoundary() ? bndFinder.position() : bndFinder.toPreviousBoundary();
            if (boundary != -1 && run.offset + boundary != ld.offset) {
                for (int i = run.offset + boundary; i < offset; i++) {
                    // remove _columns values for wrapped code units
                    p->columns[i] = 0;
                }
                offset = run.offset + boundary;
            }
        }
    };

    while (offset < p->text.size()) {
        // column is the colum relative to pos in LineData where the next cluster would be placed
        uint ch = p->text[offset].unicode();

        if (ch == '\n') {
            // '\n' is invisible for wrapping, so handle this first.
            run.endIndex = offset;
            if (run.endIndex != run.offset) {
                run.width = column - run.x;
                ld.textRuns.append(run);
            }
            p->nextIndex = offset + 1;
            ld.endIndex = offset + 1;
            p->columns[offset] = column + 1;
            return;
        }

        if (textOptionWrapMode != ZTextOption::NoWrap) {
            if (column && column >= width) { // but always consume at least one cluster
                if (textOptionWrapMode == ZTextOption::WordWrap) {
                    wordWrap();
                }
                break;
            }
        }
        int chLen = 1;
        if (QChar::isHighSurrogate(ch)) {
            if (offset + 1 < p->text.size() && QChar::isLowSurrogate(p->text[offset + 1].unicode())) {
                ch = QChar::surrogateToUcs4(ch, p->text[offset + 1].unicode());
                chLen = 2;
            }
        }
        ZTextMetrics::ClusterSize cs = p->metrics.nextCluster(p->text, offset);
        if (ch == '\t') {
            // commit text run
            run.endIndex = offset;
            if (run.endIndex != run.offset) {
                run.width = column - run.x;
                ld.textRuns.append(run);
            }

            // add tab run
            run.type = ZTextLayoutPrivate::TextRun::TAB;
            run.offset = offset;
            run.endIndex = offset + 1;
            run.x = column;
            if (p->textOption.tabs().size()) {
                auto tabs = p->textOption.tabs();
                for (const ZTextOption::Tab &tab: qAsConst(tabs)) {
                    if (tab.position > column) {
                        column = tab.position;
                        break;
                    }
                }
            }
            if (run.x == column) {
                column = (column / textOptionTabWidth) * textOptionTabWidth + textOptionTabWidth;;
            }

            if (textOptionWrapMode != ZTextOption::NoWrap) {
                if (column > width) {
                    column = width;
                }
            }

            run.width = column - run.x;
            p->columns[offset] = column;
            ld.textRuns.append(run);
            run.width = -1;

            offset += 1;

            // prepare new run
            run.type = ZTextLayoutPrivate::TextRun::COPY;
            run.offset = offset;
            run.x = column;

            continue;
        } else if ((ch & 0xffffff00) == 0xdc00 // surrogate escape
                   // vvv other specially rendered codepoints
                   || (ch < 32
                       && ch != '\n'
                       && ch != '\t')
                   || ch == 127 // DEL
                   || (ch >= 0x80 && ch <= 0x9f) // C1
                   || (ch == 0xa0 && cs.codePoints == 1) // NBSP
                   || (ch == 0xad) // SHY (soft hyphen)
                   || (ch == 0x034f) // COMBINING GRAPHEME JOINER
                   || (ch == 0x061c) // ARABIC LETTER MARK
                   || (ch == 0x070f) // SYRIAC ABBREVIATION MARK
                   || (ch >= 0x115F && ch <= 0x1160) // HANGUL CHOSEONG FILLER..HANGUL JUNGSEONG FILLER
                   || (ch == 0x17b4) // KHMER VOWEL INHERENT AQ
                   || (ch == 0x17b5) // KHMER VOWEL INHERENT AA
                   || (ch >= 0x180B && ch <= 0x180E) // MONGOLIAN FREE VARIATION SELECTOR ONE..THREE, MONGOLIAN VOWEL SEPARATOR
                   || ((ch & 0xfffffff0) == 0x2000) // U+2000..U+200F ..., LEFT-TO-RIGHT MARK, RIGHT-TO-LEFT MARK
                   || (ch >= 0x2028 && ch <= 0x202F) // U+2028..U+202F LINE SEPARATOR, PARAGRAPH SEPARATOR, LEFT-TO-RIGHT EMBEDDING, RIGHT-TO-LEFT EMBEDDING, POP DIRECTIONAL FORMATTING, LEFT-TO-RIGHT OVERRIDE, RIGHT-TO-LEFT OVERRIDE, NARROW NO-BREAK SPACE
                   || (ch >= 0x205F && ch <= 0x206F) // U+205F..U+206F ..., LEFT-TO-RIGHT ISOLATE, RIGHT-TO-LEFT ISOLATE, FIRST STRONG ISOLATE, POP DIRECTIONAL ISOLATE
                   || (ch == 0x3164) // HANGUL FILLER
                   || ((ch & 0xfffffff0) == 0xFDD0 || (ch & 0xfffffff0) == 0xFDE0) // U+FDD0..U+FDEF non characters
                   || ((ch & 0xfffffff0) == 0xfe00) // U+FE00..U+FE0F VARIATION SELECTOR-1..VARIATION SELECTOR-16
                   || (ch == 0xFEFF) // BOM or Zero Width No-Break Space
                   || (ch == 0xFFA0) // HALFWIDTH HANGUL FILLER
                   || (ch == 0xFFEF) // permanently unassigned as reverse of BOM
                   || ((ch & 0xfffffff0) == 0xfff0) // U+FFF0..U+FFFF INTERLINEAR ANNOTATION *, OBJECT REPLACEMENT CHARACTER, REPLACEMENT CHARACTER
                   || ((ch & 0xfffff000) == 0xE0000) // U+E0000..U+E0FFF unassinged default ignorable, TAG characters, VARIATION SELECTOR-17..256
                   || ((ch & 0x0000fffe) == 0xfffe) // non characters U+FFFE, U+FFFF in each plain
                   || (ch == 0x16FE4) // KHITAN SMALL SCRIPT FILLER
                   || ((ch & 0xfffffff0) == 0x1BCA0) // SHORTHAND FORMAT CONTROL block
                   || (ch >= 0x1D173 && ch <= 0x1D17A) // MUSICAL SYMBOL BEGIN BEAM..MUSICAL SYMBOL END PHRASE
                   ) {
            // NOTE: This can only trigger on cluster start. Codepoints that are zero width according to p->metrics and
            //       follow a non zero width character are handled in the copy run code below. For changes here check
            //       if that code needs the change too.

            // commit text run
            run.endIndex = offset;
            if (run.endIndex != run.offset) {
                run.width = column - run.x;
                ld.textRuns.append(run);
            }

            int widthOfDisplay;
            if (ch < 128) {
                widthOfDisplay = 2;
            } else if ((ch & 0xff00) == 0xdc00) {
                widthOfDisplay = 4;
            } else if (ch < 0x10000) {
                widthOfDisplay = 8;
            } else {
                widthOfDisplay = 10;
            }

            if (textOptionWrapMode != ZTextOption::NoWrap) {
                if (column && column + widthOfDisplay > width) { // but always consume at least one cluster
                    column = width;
                    continue;
                }
            }

            // add special bytes or char run
            run.type = ZTextLayoutPrivate::TextRun::SPECIAL_BYTE_OR_CHAR;
            run.offset = offset;
            run.endIndex = offset + chLen;
            run.x = column;
            column += widthOfDisplay;

            run.width = column - run.x;
            for (int i = 0; i < chLen; i++) {
                p->columns[offset + i] = column;
            }
            ld.textRuns.append(run);
            run.width = -1;

            offset += chLen;

            // prepare new run
            run.type = ZTextLayoutPrivate::TextRun::COPY;
            run.offset = offset;
            run.x = column;

            continue;
        } else if ((textOptionFlags & (ZTextOption::ShowTabsAndSpaces | ZTextOption::ShowTabsAndSpacesWithColors))
                   && ch == ' ' && cs.codePoints == 1) {
            // commit text run
            run.endIndex = offset;
            if (run.endIndex != run.offset) {
                run.width = column - run.x;
                ld.textRuns.append(run);
            }

            // add spaces run
            run.type = ZTextLayoutPrivate::TextRun::SPACES;
            run.offset = offset;
            run.x = column;

            // find offset that is after the spaces run
            while (offset < p->text.size() && p->text[offset] == u' ' && cs.codePoints == 1) {
                if (textOptionWrapMode != ZTextOption::NoWrap) {
                    if (column >= width) {
                        break;
                    }
                }
                column += 1;
                p->columns[offset] = column;
                offset += 1;
                cs = p->metrics.nextCluster(p->text, offset);
            }
            run.width = column - run.x;
            run.endIndex = offset;
            ld.textRuns.append(run);
            run.width = -1;

            // prepare new run
            run.type = ZTextLayoutPrivate::TextRun::COPY;
            run.offset = offset;
            run.x = column;

            continue;
        }

        for (int i = chLen; i < cs.codeUnits; i++) {
            int ch2 = p->text.at(offset + i).unicode();
            int chSkip2 = 0;
            if (QChar::isHighSurrogate(ch2)) {
                if (offset + i + 1 < p->text.size() && QChar::isLowSurrogate(p->text[offset + i + 1].unicode())) {
                    ch2 = QChar::surrogateToUcs4(ch2, p->text[offset + i + 1].unicode());
                    chSkip2 = 1;
                }
            }
            // see sibling for ch
            if ((ch2 & 0xffffff00) == 0xdc00 // surrogate escape
                               // vvv other specially rendered codepoints
                               || (ch2 < 32
                                   && ch2 != '\n'
                                   && ch2 != '\t')
                               || ch2 == 127 // DEL
                               || (ch2 >= 0x80 && ch2 <= 0xa0) // C1 and NBSP
                               || (ch2 == 0xad) // SHY (soft hyphen)
                               || (ch2 == 0x034f) // COMBINING GRAPHEME JOINER
                               || (ch2 == 0x061c) // ARABIC LETTER MARK
                               || (ch2 == 0x070f) // SYRIAC ABBREVIATION MARK
                               || (ch2 >= 0x115F && ch2 <= 0x1160) // HANGUL CHOSEONG FILLER..HANGUL JUNGSEONG FILLER
                               || (ch2 == 0x17b4) // KHMER VOWEL INHERENT AQ
                               || (ch2 == 0x17b5) // KHMER VOWEL INHERENT AA
                               || (ch2 >= 0x180B && ch2 <= 0x180E) // MONGOLIAN FREE VARIATION SELECTOR ONE..THREE, MONGOLIAN VOWEL SEPARATOR
                               || ((ch2 & 0xfffffff0) == 0x2000) // U+2000..U+200F ..., LEFT-TO-RIGHT MARK, RIGHT-TO-LEFT MARK
                               || (ch2 >= 0x2028 && ch2 <= 0x202F) // U+2028..U+202F LINE SEPARATOR, PARAGRAPH SEPARATOR, LEFT-TO-RIGHT EMBEDDING, RIGHT-TO-LEFT EMBEDDING, POP DIRECTIONAL FORMATTING, LEFT-TO-RIGHT OVERRIDE, RIGHT-TO-LEFT OVERRIDE, NARROW NO-BREAK SPACE
                               || (ch2 >= 0x205F && ch2 <= 0x206F) // U+205F..U+206F ..., LEFT-TO-RIGHT ISOLATE, RIGHT-TO-LEFT ISOLATE, FIRST STRONG ISOLATE, POP DIRECTIONAL ISOLATE
                               || (ch2 == 0x3164) // HANGUL FILLER
                               || ((ch2 & 0xfffffff0) == 0xFDD0 || (ch2 & 0xfffffff0) == 0xFDE0) // U+FDD0..U+FDEF non characters
                               || ((ch2 & 0xfffffff0) == 0xfe00) // U+FE00..U+FE0F VARIATION SELECTOR-1..VARIATION SELECTOR-16
                               || (ch2 == 0xFEFF) // BOM or Zero Width No-Break Space
                               || (ch2 == 0xFFA0) // HALFWIDTH HANGUL FILLER
                               || (ch2 == 0xFFEF) // permanently unassigned as reverse of BOM
                               || ((ch2 & 0xfffffff0) == 0xfff0) // U+FFF0..U+FFFF INTERLINEAR ANNOTATION *, OBJECT REPLACEMENT CHARACTER, REPLACEMENT CHARACTER
                               || ((ch2 & 0xfffff000) == 0xE0000) // U+E0000..U+E0FFF unassinged default ignorable, TAG characters, VARIATION SELECTOR-17..256
                               || ((ch2 & 0x0000fffe) == 0xfffe) // non characters U+FFFE, U+FFFF in each plain
                               || (ch2 == 0x16FE4) // KHITAN SMALL SCRIPT FILLER
                               || ((ch2 & 0xfffffff0) == 0x1BCA0) // SHORTHAND FORMAT CONTROL block
                               || (ch2 >= 0x1D173 && ch2 <= 0x1D17A) // MUSICAL SYMBOL BEGIN BEAM..MUSICAL SYMBOL END PHRASE
                               ) {
                cs.codeUnits = i;
                break;
            }
            i += chSkip2;
        }

        if (cs.codeUnits == 0) {
            cs.codeUnits = 1;
            cs.columns = 1;
        }

        if (textOptionWrapMode != ZTextOption::NoWrap) {
            // Is a break *before* the currently processed character needed?
            if (column && column + cs.columns > width) { // but always consume at least one cluster
                if (textOptionWrapMode == ZTextOption::WordWrap) {
                    wordWrap();
                }
                break;
            }
        }

        column += cs.columns;
        for (int i = 0; i < cs.codeUnits; i++) {
            p->columns[offset + i] = column;
        }
        offset += cs.codeUnits;
    }
    // commit text run
    run.endIndex = offset;
    if (run.endIndex != run.offset) {
        run.width = column - run.x;
        ld.textRuns.append(run);
    }

    // finalize
    p->nextIndex = offset;
    ld.endIndex = offset;
    return;
}

ZTextLineRef::ZTextLineRef(ZTextLayout *layout, int index) : _layout(layout), _index(index)
{
}

ZTextLineRef::ZTextLineRef() {
}

ZTextLineRef::ZTextLineRef(const ZTextLineRef &) = default;

ZTextLineRef::~ZTextLineRef() = default;

ZTextLineRef &ZTextLineRef::operator=(const ZTextLineRef &) = default;


bool ZTextLineRef::isValid() const {
    return _index != -1;
}

void ZTextLineRef::draw(ZPainter painter, const QPoint &pos, ZTextStyle color, ZTextStyle formattingChars,
                        const QVector<ZFormatRange> &ranges) const {
    if (_layout) {
        auto *const p = _layout->tuiwidgets_impl();
        auto textOptionFlags = p->textOption.flags();

        ZTextLayoutPrivate::LineData &ld = p->lines[_index];
        ZPainter painterClipped = painter.translateAndClip({0, 0, pos.x() + ld.pos.x() + ld.maxWidth,
                                                            pos.y() + ld.pos.y() + 1});

        for (int i = 0; i < ld.textRuns.size(); i++) {
            const ZTextLayoutPrivate::TextRun &run = ld.textRuns[i];
            if (run.type == ZTextLayoutPrivate::TextRun::COPY) {
                painterClipped.writeWithAttributes(pos.x() + ld.pos.x() + run.x, pos.y() + ld.pos.y(),
                                                   p->text.mid(run.offset, run.endIndex - run.offset),
                                                   color.foregroundColor(), color.backgroundColor(), color.attributes());
                for (const ZFormatRange &formatRange: ranges) {
                    if (formatRange.length() <= 0) {
                        continue;
                    }
                    int formatRangeEnd = formatRange.start() + formatRange.length();
                    if (formatRange.start() <= run.offset && formatRangeEnd > run.offset) {
                        // selection ends in run
                        // make sure it's not an invalid position
                        while (formatRangeEnd < run.endIndex
                               && p->columns[formatRangeEnd - 1] == p->columns[formatRangeEnd]) {
                            ++formatRangeEnd;
                        }
                        painterClipped.writeWithAttributes(pos.x() + ld.pos.x() + run.x, pos.y() + ld.pos.y(),
                                                    p->text.mid(run.offset, std::min(run.endIndex, formatRangeEnd) - run.offset),
                                                    formatRange.format().foregroundColor(), formatRange.format().backgroundColor(),
                                                    formatRange.format().attributes());
                    } else if (formatRange.start() > run.offset && formatRange.start() < run.endIndex) {
                        // selection starts in run
                        int start = formatRange.start();
                        // make sure it's not an invalid position
                        while (start > 0 && p->columns[start - 1] == p->columns[start]) {
                            start--;
                        }
                        while (formatRangeEnd < run.endIndex
                               && p->columns[formatRangeEnd - 1] == p->columns[formatRangeEnd]) {
                            ++formatRangeEnd;
                        }
                        int startX = 0;
                        if (start > 0) {
                            startX = p->columns[start - 1];
                        }
                        painterClipped.writeWithAttributes(pos.x() + ld.pos.x() + startX, pos.y() + ld.pos.y(),
                                                           p->text.mid(start, std::min(run.endIndex, formatRangeEnd) - start),
                                                           formatRange.format().foregroundColor(),
                                                           formatRange.format().backgroundColor(),
                                                           formatRange.format().attributes());
                    }
                }
            } else if (run.type == ZTextLayoutPrivate::TextRun::TAB) {
                const ZFormatRange *range = nullptr;
                for (const ZFormatRange &formatRange: ranges) {
                    if (formatRange.length() <= 0) {
                        continue;
                    }
                    int formatRangeEnd = formatRange.start() + formatRange.length();
                    if (run.offset >= formatRange.start() && run.offset < formatRangeEnd) {
                        range = &formatRange;
                    }
                }
                if (textOptionFlags & ZTextOption::ShowTabsAndSpacesWithColors) {
                    ZTextStyle style;
                    int hidden = p->textOption.tabStopDistance() - run.width;
                    for (int j = 0; j < run.width; j++) {
                        style = p->textOption.mapTabColor(j + hidden, run.width, hidden, color, formattingChars, range);
                        painterClipped.writeWithAttributes(pos.x() + ld.pos.x() + run.x + j, pos.y() + ld.pos.y(),
                                                    (textOptionFlags & ZTextOption::ShowTabsAndSpaces && j == run.width / 2) ? QStringLiteral("→") : QStringLiteral(" "),
                                                    style.foregroundColor(), style.backgroundColor(), style.attributes());
                    }
                } else if (textOptionFlags & ZTextOption::ShowTabsAndSpaces) {
                    ZTextStyle style = range ? range->format() : color;
                    painterClipped.clearRect(pos.x() + ld.pos.x() + run.x, pos.y() + ld.pos.y(), run.width, 1,
                                             style.foregroundColor(), style.backgroundColor(), style.attributes());
                    style = range ? range->formattingChar() : formattingChars;
                    painterClipped.writeWithAttributes(pos.x() + ld.pos.x() + run.x + (run.width / 2), pos.y() + ld.pos.y(),
                                                       QStringLiteral("→"),
                                                       style.foregroundColor(), style.backgroundColor(), style.attributes());
                } else {
                    ZTextStyle style = range ? range->format() : color;
                    painterClipped.clearRect(pos.x() + ld.pos.x() + run.x, pos.y() + ld.pos.y(), run.width, 1,
                                             style.foregroundColor(), style.backgroundColor(), style.attributes());
                }
            } else if (run.type == ZTextLayoutPrivate::TextRun::SPACES) {
                // only generated with ShowTabsAndSpaces or ShowTabsAndSpacesWithColors active
                ZTextStyle style = formattingChars;
                QChar ch = U' ';
                if (textOptionFlags & ZTextOption::ShowTabsAndSpaces) {
                    ch = U'·';
                }
                bool highlightingTrailingWhitespace = false;
                if (textOptionFlags & ZTextOption::ShowTabsAndSpacesWithColors) {
                    if (i + 1 == ld.textRuns.size() && (run.endIndex != ld.endIndex || ld.endIndex == p->text.size())) {
                        highlightingTrailingWhitespace = true;
                        style = p->textOption.mapTrailingWhitespaceColor(color, formattingChars, nullptr);
                    }
                }
                painterClipped.writeWithAttributes(pos.x() + ld.pos.x() + run.x, pos.y() + ld.pos.y(),
                                                   QString(run.width, ch),
                                                   style.foregroundColor(), style.backgroundColor(), style.attributes());
                for (const ZFormatRange& formatRange: ranges) {
                    if (formatRange.length() <= 0) {
                        continue;
                    }
                    int formatRangeEnd = formatRange.start() + formatRange.length();
                    if (formatRange.start() <= run.offset && formatRangeEnd > run.offset) {
                        // selection ends in run
                        style = formatRange.formattingChar();
                        if (highlightingTrailingWhitespace) {
                            style = p->textOption.mapTrailingWhitespaceColor(color, formattingChars, &formatRange);
                        }
                        painterClipped.writeWithAttributes(pos.x() + ld.pos.x() + run.x, pos.y() + ld.pos.y(),
                                                           QString(std::min(run.width, formatRangeEnd - run.offset), ch),
                                                           style.foregroundColor(), style.backgroundColor(), style.attributes());
                    } else if (formatRange.start() > run.offset && formatRange.start() < run.endIndex) {
                        // selection starts in run
                        style = formatRange.formattingChar();
                        if (highlightingTrailingWhitespace) {
                            style = p->textOption.mapTrailingWhitespaceColor(color, formattingChars, &formatRange);
                        }
                        painterClipped.writeWithAttributes(pos.x() + ld.pos.x() + p->columns[formatRange.start() - 1],
                                                           pos.y() + ld.pos.y(),
                                                           QString(std::min(run.endIndex, formatRangeEnd) - formatRange.start(), ch),
                                                           style.foregroundColor(), style.backgroundColor(), style.attributes());
                    }
                }
            } else if (run.type == ZTextLayoutPrivate::TextRun::SPECIAL_BYTE_OR_CHAR) {
                unsigned short ch = p->text[run.offset].unicode();
                auto formatHex = [] (unsigned int value, int length) {
                    return QString::number(value, 16).rightJustified(length, u'0').toUpper();
                };
                QString contents;
                if (ch < 32) {
                    contents = QStringLiteral("^") + QString(QChar('@' + ch));
                } else if (ch == 127) {
                    contents = QStringLiteral("^?");
                } else if ((ch & 0xffffff00) == 0xdc00) { // surrogate escape
                    contents = QStringLiteral("<%0>").arg(formatHex(ch & 0xff, 2));
                } else if (!QChar::isHighSurrogate(ch)) { // invariant: a run of type SPECIAL_BYTE_OR_CHAR is only generated for valid pairs
                    contents = QStringLiteral("<U+%0>").arg(formatHex(ch, 4));
                } else {
                    unsigned int ch32 = QChar::surrogateToUcs4(ch, p->text[run.offset + 1].unicode());
                    contents = QStringLiteral("<U+%0>").arg(formatHex(ch32, 6));
                }
                contents = contents.left(run.width);

                ZColor fg = color.backgroundColor();
                ZColor bg = color.foregroundColor();
                ZPainter::Attributes attr = color.attributes();
                for (const ZFormatRange& formatRange: ranges) {
                    if (formatRange.length() <= 0) {
                        continue;
                    }
                    int formatRangeEnd = formatRange.start() + formatRange.length();
                    if ((formatRange.start() <= run.offset && formatRangeEnd > run.offset)
                            || (formatRange.start() > run.offset && formatRange.start() < run.endIndex)) {
                        fg = formatRange.format().backgroundColor();
                        bg = formatRange.format().foregroundColor();
                        attr = formatRange.format().attributes();
                    }
                }
                painterClipped.writeWithAttributes(pos.x() + ld.pos.x() + run.x, pos.y() + ld.pos.y(),
                                                   contents,
                                                   fg, bg, attr);
            }
        }
        if (_index != 0) {
            painterClipped.setSoftwrapMarker(pos.x() + ld.pos.x(), pos.y() + ld.pos.y());
        }
        bool lastLine = ld.endIndex >= p->text.size();
        if (!lastLine) {
            if (ld.textRuns.size()) {
                auto lastRun = ld.textRuns.last();
                painterClipped.setSoftwrapMarker(pos.x() + ld.pos.x() + lastRun.x + lastRun.width - 1, pos.y() + ld.pos.y());
            }
        }
    }
}

int ZTextLineRef::cursorToX(int *cursorPos, ZTextLayout::Edge edge) const {
    if (_layout) {
        auto *const p = _layout->tuiwidgets_impl();
        ZTextLayoutPrivate::LineData &ld = p->lines[_index];

        if (ld.offset == ld.endIndex) {
            *cursorPos = ld.offset;
            return ld.pos.x();
        }

        if (*cursorPos < ld.offset) {
            *cursorPos = ld.offset;
        } else if (*cursorPos >= ld.endIndex) {
            *cursorPos = ld.endIndex;
            return x() + width();
        }

        int leading, trailing;
        // if *cursorPos does not point to initial codepoint in a cluster move it
        // to the beginning of the current cluster
        if (*cursorPos != ld.offset) {
            int col = p->columns[*cursorPos];
            if (p->columns[*cursorPos - 1] == col) {
                // *cursorPos does not point to the beginning of a cluster
                --*cursorPos;
                while (true) {
                    if (p->columns[*cursorPos] != col) {
                        ++*cursorPos;
                        break;
                    }
                    if (*cursorPos > ld.offset) {
                        --*cursorPos;
                    } else {
                        break;
                    }
                }
                if (*cursorPos == ld.offset) {
                    leading = 0;
                    trailing = p->columns[*cursorPos];
                } else {
                    leading = p->columns[*cursorPos - 1];
                    trailing = p->columns[*cursorPos];
                }
            } else {
                // *cursorPos points to the beginning of a cluster
                leading = p->columns[*cursorPos - 1];
                trailing = p->columns[*cursorPos];
                if (p->text[*cursorPos] == u'\n') {
                    trailing = leading;
                }
            }
        } else {
            leading = 0;
            trailing = p->columns[*cursorPos];
            if (p->text[*cursorPos] == u'\n') {
                trailing = leading;
            }
        }

        if (edge == ZTextLayout::Leading) {
            return ld.pos.x() + leading;
        } else {
            return ld.pos.x() + trailing;
        }
    } else {
        *cursorPos = 0;
        return 0;
    }
}

int ZTextLineRef::cursorToX(int cursorPos, ZTextLayout::Edge edge) const {
    return cursorToX(&cursorPos, edge);
}

int ZTextLineRef::xToCursor(int x) const {
    if (_layout) {
        auto *const p = _layout->tuiwidgets_impl();
        ZTextLayoutPrivate::LineData &ld = p->lines[_index];
        x -= ld.pos.x();
        int charLeft = 0;
        for (int i = ld.offset; i < ld.endIndex; i++) {
            int charRight = p->columns[i];
            if (charLeft == charRight) {
                continue;
            }
            if (charRight > x) {
                if (charLeft + 1 == charRight || (charLeft + charRight) / 2 > x) {
                    return i;
                } else {
                    return _layout->nextCursorPosition(i);
                }
            }
            charLeft = charRight;
        }
        return ld.endIndex;
    } else {
        return 0;
    }
}

void ZTextLineRef::setPosition(const QPoint &pos) {
    if (_layout) {
        auto *const p = _layout->tuiwidgets_impl();
        ZTextLayoutPrivate::LineData &ld = p->lines[_index];
        ld.pos = pos;
    }
}

QPoint ZTextLineRef::position() const {
    if (_layout) {
        auto *const p = _layout->tuiwidgets_impl();
        ZTextLayoutPrivate::LineData &ld = p->lines[_index];
        return ld.pos;
    } else {
        return QPoint();
    }
}

int ZTextLineRef::x() const {
    return position().x();
}

int ZTextLineRef::y() const {
    return position().y();
}

int ZTextLineRef::width() const {
    if (_layout) {
        auto *const p = _layout->tuiwidgets_impl();
        ZTextLayoutPrivate::LineData &ld = p->lines[_index];
        if (ld.offset != ld.endIndex) {
            return p->columns[ld.endIndex - 1] - (p->text[ld.endIndex - 1] == u'\n' ? 1 : 0);
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

int ZTextLineRef::height() const {
    return 1;
}

QRect ZTextLineRef::rect() const {
    return QRect(position(), QSize{width(), height()});
}

int ZTextLineRef::lineNumber() const {
    return _index;
}

int ZTextLineRef::textStart() const {
    if (_layout) {
        auto *const p = _layout->tuiwidgets_impl();
        ZTextLayoutPrivate::LineData &ld = p->lines[_index];
        return ld.offset;
    } else {
        return 0;
    }
}

int ZTextLineRef::textLength() const {
    if (_layout) {
        auto *const p = _layout->tuiwidgets_impl();
        ZTextLayoutPrivate::LineData &ld = p->lines[_index];
        return ld.endIndex - ld.offset;
    } else {
        return 0;
    }
}

void ZTextLineRef::setLineWidth(int width) {
    if (_layout) {
        _layout->layoutLine(_index, width);
    }
}

TUIWIDGETS_NS_END
