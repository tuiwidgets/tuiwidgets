// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZDocument.h>
#include <Tui/ZDocument_p.h>

#include <variant>

#include <QThreadPool>
#include <Qt>

#include <Tui/ZDocumentSnapshot.h>

#include <Tui/Utils_p.h>

TUIWIDGETS_NS_START


namespace {
    struct SearchParameter {
        bool searchWrap = false;
        Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive;
        int startAtLine = 0;
        int startCodeUnit = 0;
        std::variant<QString, QRegularExpression> needle;
    };

    struct NoCanceler {
        bool isCanceled() {
            return false;
        }
    };

    ZDocumentFindAsyncResult ZDocumentFindAsyncResultNew(ZDocumentCursor::Position anchor,
                                                         ZDocumentCursor::Position cursor,
                                                         unsigned revision,
                                                         QRegularExpressionMatch match) {
        return ZDocumentFindAsyncResult(
                    std::make_unique<ZDocumentFindAsyncResultPrivate>(anchor, cursor, revision, match));
    }

    ZDocumentFindAsyncResult noMatch(const ZDocumentSnapshot &snap) {
        return ZDocumentFindAsyncResultNew({0, 0}, {0, 0}, snap.revision(), QRegularExpressionMatch{});
    }

    ZDocumentFindAsyncResult noMatch(unsigned revision) {
        return ZDocumentFindAsyncResultNew({0, 0}, {0, 0}, revision, QRegularExpressionMatch{});
    }

    void replaceInvalidUtf16ForRegexSearch(QString &buffer, int start) {
        for (int i = start; i < buffer.size(); i++) {
            QChar ch = buffer[i];
            if (ch.isHighSurrogate()) {
                if (i + 1 < buffer.size() && QChar(buffer[i + 1]).isLowSurrogate()) {
                    // ok, skip low surrogate
                    i++;
                } else {
                    // not valid utf16, replace so it doesn't break regex search
                    buffer[i] = 0xFFFD;
                }
            } else if (ch.isLowSurrogate()) {
                // not valid utf16, replace so it doesn't break regex search
                // this might be a surrogate escape but libpcre (used by QRegularExpression) can't work
                // with surrogate escapes
                buffer[i] = 0xFFFD;
            }
        }
    }

    bool isPotententialMultiLineMatch(const QRegularExpression &regex) {
        // This is mostly at placeholder at the moment, should parse the regex quite a bit more.

        // Currently everything that contains characters that would be quoted is considered potentially
        // multi line. This of course has lots of false positives but is very easy and does not have false negatives.
        for (QChar ch: regex.pattern()) {
            if ((ch >= QLatin1Char('a') && ch <= QLatin1Char('z'))
                    || (ch >= QLatin1Char('A') && ch <= QLatin1Char('Z'))
                    || (ch >= QLatin1Char('0') && ch <= QLatin1Char('9'))
                    || ch == QLatin1Char('_')) {
                // if not preceeded by any non literal characters this is a literal match
            } else {
                return true;
            }
        }
        return false;
    }

    template <typename CANCEL>
    static ZDocumentFindAsyncResult snapshotSearchForwardRegex(ZDocumentSnapshot snap, SearchParameter search, CANCEL &canceler) {

        auto regex = std::get<QRegularExpression>(search.needle);
        if ((regex.patternOptions() & QRegularExpression::PatternOption::MultilineOption) == 0) {
            regex.setPatternOptions(regex.patternOptions() | QRegularExpression::PatternOption::MultilineOption);
        }
        if (regex.patternOptions().testFlag(QRegularExpression::PatternOption::CaseInsensitiveOption) !=
                (search.caseSensitivity == Qt::CaseInsensitive)) {
            regex.setPatternOptions(regex.patternOptions() ^ QRegularExpression::PatternOption::CaseInsensitiveOption);
        }

        int line = search.startAtLine;
        int found = search.startCodeUnit - 1;
        int end = snap.lineCount();

        bool hasWrapped = false;

        while (true) {
            for (; line < end; line++) {
                QString buffer = snap.line(line);
                replaceInvalidUtf16ForRegexSearch(buffer, 0);
                if (line + 1 < snap.lineCount()) {
                    buffer += QStringLiteral("\n");
                }
                int foldedLine = line;
                QRegularExpressionMatchIterator remi
                        = regex.globalMatch(buffer, 0,
                                            foldedLine + 1 < snap.lineCount() ? QRegularExpression::MatchType::PartialPreferFirstMatch
                                                                              : QRegularExpression::MatchType::NormalMatch,
                                            QRegularExpression::MatchOption::DontCheckSubjectStringMatchOption);
                while (remi.hasNext()) {
                    QRegularExpressionMatch match = remi.next();
                    if (canceler.isCanceled()) {
                        return noMatch(snap);
                    }
                    if (match.hasPartialMatch()) {
                        const int cont = buffer.size();
                        foldedLine += 1;
                        buffer += snap.line(foldedLine);
                        replaceInvalidUtf16ForRegexSearch(buffer, cont);
                        if (foldedLine + 1 < snap.lineCount()) {
                            buffer += QStringLiteral("\n");
                        }
                        remi = regex.globalMatch(buffer, 0,
                                                 foldedLine + 1 < snap.lineCount() ? QRegularExpression::MatchType::PartialPreferFirstMatch
                                                                                   : QRegularExpression::MatchType::NormalMatch,
                                                 QRegularExpression::MatchOption::DontCheckSubjectStringMatchOption);
                        continue;
                    }
                    if (match.capturedLength() <= 0) continue;
                    if (match.capturedStart() < found + 1) continue;
                    found = match.capturedStart();
                    int foundLine = line;
                    while (found > snap.lineCodeUnits(foundLine)) {
                        found -= snap.lineCodeUnits(foundLine);
                        found -= 1; // the "\n" itself
                        foundLine += 1;
                    }
                    int endLine = line;
                    int endCodeUnit = match.capturedStart() + match.capturedLength();
                    while (endCodeUnit > snap.lineCodeUnits(endLine)) {
                        endCodeUnit -= snap.lineCodeUnits(endLine);
                        endCodeUnit -= 1; // the "\n" itself
                        endLine += 1;
                    }
                    return ZDocumentFindAsyncResultNew({found, foundLine},
                                                       {endCodeUnit, endLine},
                                                       snap.revision(),
                                                       match);
                }
                // we searched everything until including folded line, so no need to try those lines again.
                line = foldedLine;
                found = -1;

                if (canceler.isCanceled()) {
                    return noMatch(snap);
                }
            }
            if (!search.searchWrap || hasWrapped) {
                return noMatch(snap);
            }
            hasWrapped = true;
            end = std::min(search.startAtLine + 1, snap.lineCount());
            line = 0;
        }
        return noMatch(snap);
    }

    template <typename CANCEL>
    static ZDocumentFindAsyncResult snapshotSearchForwardLiteral(ZDocumentSnapshot snap, SearchParameter search, CANCEL &canceler) {

        const QString needle = std::get<QString>(search.needle);
        const QStringList parts = needle.split(QLatin1Char('\n'));

        int line = search.startAtLine;
        int found = search.startCodeUnit - 1;
        int end = snap.lineCount();

        bool hasWrapped = false;
        while (true) {
            for (; line < end; line++) {
                if (parts.size() > 1) {
                    const int numberLinesToCome = snap.lineCount() - line;
                    if (parts.size() > numberLinesToCome) {
                        found = -1;
                        continue;
                    }
                    const int markStart = snap.line(line).size() - parts.first().size();
                    if (found < markStart && snap.line(line).endsWith(parts.first(), search.caseSensitivity)) {
                        found = markStart;
                        if (snap.line(line + parts.size() - 1).startsWith(parts.last(), search.caseSensitivity)) {
                            for (int i = parts.size() - 2; i > 0; i--) {
                                if (snap.line(line + i).compare(parts.at(i), search.caseSensitivity)) {
                                    i = found = -1;
                                }
                            }
                            if (found != -1)
                                return ZDocumentFindAsyncResultNew({found, line},
                                                                   {parts.last().size(), line + parts.size() - 1},
                                                                   snap.revision(),
                                                                   QRegularExpressionMatch{});
                       }
                    }
                    found = -1;
                } else {
                    found = snap.line(line).indexOf(needle, found + 1, search.caseSensitivity);

                    if (found != -1) {
                        const int length = needle.size();
                        return ZDocumentFindAsyncResultNew({found, line},
                                                           {found + length, line},
                                                           snap.revision(),
                                                           QRegularExpressionMatch{});
                    }
                }

                if (canceler.isCanceled()) {
                    return noMatch(snap);
                }
            }
            if (!search.searchWrap || hasWrapped) {
                return noMatch(snap);
            }
            hasWrapped = true;
            end = std::min(search.startAtLine + 1, snap.lineCount());
            line = 0;
        }
        return noMatch(snap);
    }

    template <typename CANCEL>
    static ZDocumentFindAsyncResult snapshotSearchForward(ZDocumentSnapshot snap, SearchParameter search, CANCEL &canceler) {
        const bool regularExpressionMode = std::holds_alternative<QRegularExpression>(search.needle);
        if (regularExpressionMode) {
            return snapshotSearchForwardRegex(snap, search, canceler);
        } else {
            return snapshotSearchForwardLiteral(snap, search, canceler);
        }
    }

    template <typename CANCEL>
    static ZDocumentFindAsyncResult snapshotSearchBackwardsRegex(ZDocumentSnapshot snap, SearchParameter search, CANCEL &canceler) {

        auto regex = std::get<QRegularExpression>(search.needle);
        if (regex.patternOptions().testFlag(QRegularExpression::PatternOption::CaseInsensitiveOption) !=
                (search.caseSensitivity == Qt::CaseInsensitive)) {
            regex.setPatternOptions(regex.patternOptions() ^ QRegularExpression::PatternOption::CaseInsensitiveOption);
        }

        if (isPotententialMultiLineMatch(regex)) {
            if ((regex.patternOptions() & QRegularExpression::PatternOption::MultilineOption) == 0) {
                regex.setPatternOptions(regex.patternOptions() | QRegularExpression::PatternOption::MultilineOption);
            }

            // Matching in reverse is quite hard to get right and performant. For now just get it right.
            // If this ever is a bottleneck in actual use, we need to think how to improve performance.
            // For some cases a simple cache of all match positions might be enough.

            QString buffer;
            int startIndex = -1;
            for (int i = 0; i < snap.lineCount(); i++) {
                if (i == search.startAtLine) {
                    startIndex = buffer.size() + search.startCodeUnit;
                }

                buffer += snap.line(i);
                if (i + 1 < snap.lineCount()) {
                    buffer += QStringLiteral("\n");
                }

                if (canceler.isCanceled()) {
                    return noMatch(snap);
                }
            }

            if (startIndex == -1) {
                startIndex = buffer.size();
            }

            replaceInvalidUtf16ForRegexSearch(buffer, 0);

            std::optional<QRegularExpressionMatch> noWrapMatch;
            std::optional<QRegularExpressionMatch> wrapMatch;
            QRegularExpressionMatchIterator remi = regex.globalMatch(buffer);
            while (remi.hasNext()) {
                QRegularExpressionMatch match = remi.next();
                if (canceler.isCanceled()) {
                    return noMatch(snap);
                }
                if (match.capturedLength() <= 0) continue;

                if (match.capturedStart() <= startIndex - match.capturedLength()) {
                    noWrapMatch = match;
                    continue;
                }

                if (!search.searchWrap) {
                    // No wrapping requested, we have all we need.
                    break;
                }

                if (noWrapMatch) {
                    // No wrapping needed, we have the match.
                    break;
                }

                wrapMatch = match;
            }

            if (noWrapMatch || wrapMatch) {
                QRegularExpressionMatch match = noWrapMatch ? *noWrapMatch : *wrapMatch;
                int found = match.capturedStart();
                int foundLine = 0;
                while (found > snap.lineCodeUnits(foundLine)) {
                    found -= snap.lineCodeUnits(foundLine);
                    found -= 1; // the "\n" itself
                    foundLine += 1;
                }
                int endLine = 0;
                int endCodeUnit = match.capturedStart() + match.capturedLength();
                while (endCodeUnit > snap.lineCodeUnits(endLine)) {
                    endCodeUnit -= snap.lineCodeUnits(endLine);
                    endCodeUnit -= 1; // the "\n" itself
                    endLine += 1;
                }
                return ZDocumentFindAsyncResultNew({found, foundLine},
                                                   {endCodeUnit, endLine},
                                                   snap.revision(),
                                                   match);

            } else {
                return noMatch(snap);
            }
            // never reached
        }

        if ((regex.patternOptions() & QRegularExpression::PatternOption::MultilineOption)) {
            regex.setPatternOptions(regex.patternOptions() ^ QRegularExpression::PatternOption::MultilineOption);
        }

        int line = search.startAtLine;
        int searchAt = search.startCodeUnit;
        int end = 0;
        bool hasWrapped = false;
        while (true) {
            for (; line >= end;) {
                QString lineBuffer = snap.line(line);
                replaceInvalidUtf16ForRegexSearch(lineBuffer, 0);

                ZDocumentFindAsyncResult res = noMatch(snap);
                QRegularExpressionMatchIterator remi = regex.globalMatch(lineBuffer);
                while (remi.hasNext()) {
                    QRegularExpressionMatch match = remi.next();
                    if (canceler.isCanceled()) {
                        return noMatch(snap);
                    }
                    if (match.capturedLength() <= 0) continue;
                    if (match.capturedStart() <= searchAt - match.capturedLength()) {
                        res = ZDocumentFindAsyncResultNew({size2int(match.capturedStart()), line},
                                                          {size2int(match.capturedStart() + match.capturedLength()), line},
                                                          snap.revision(),
                                                          match);
                        continue;
                    }
                    break;
                }
                if (res.anchor() != res.cursor()) {
                    return res;
                }

                if (canceler.isCanceled()) {
                    return noMatch(snap);
                }
                line -= 1;
                if (line >= 0) {
                    searchAt = snap.line(line).size();
                }
            }
            if (!search.searchWrap || hasWrapped) {
                return noMatch(snap);
            }
            hasWrapped = true;
            end = search.startAtLine;
            line = snap.lineCount() - 1;
            searchAt = snap.lineCodeUnits(line);
        }
    }

    template <typename CANCEL>
    static ZDocumentFindAsyncResult snapshotSearchBackwardsLiteral(ZDocumentSnapshot snap, SearchParameter search, CANCEL &canceler) {

        const QString needle = std::get<QString>(search.needle);
        const QStringList parts = needle.split(QLatin1Char('\n'));

        int line = search.startAtLine;
        int searchAt = search.startCodeUnit;
        int end = 0;

        bool hasWrapped = false;
        while (true) {
            for (; line >= end;) {
                if (parts.size() > 1) {
                    int endLine = line - parts.size() + 1;
                    if (endLine < 0) {
                        line = -1; // because the for loop does not do this for us.
                        searchAt = -1;
                        continue;
                    }
                    if (searchAt >= parts.last().size() && snap.line(line).startsWith(parts.last(), search.caseSensitivity)) {
                        searchAt = parts.last().size();
                        if (snap.line(endLine).endsWith(parts.first(), search.caseSensitivity)) {
                            for (int i = parts.size() - 2; i > 0; i--) {
                                if (snap.line(line - i).compare(parts.at(i), search.caseSensitivity)) {
                                    i = searchAt = -1;
                                }
                            }
                            if (searchAt != -1) {
                                int endAt = snap.line(endLine).size() - parts.first().size();
                                return ZDocumentFindAsyncResultNew({endAt, endLine},
                                                                   {searchAt, line},
                                                                   snap.revision(),
                                                                   QRegularExpressionMatch{});
                            }
                        }
                    }
                    searchAt = -1;
                } else {
                    const int length = needle.size();
                    if (searchAt >= length) {
                        const int found = snap.line(line).lastIndexOf(needle,
                                                                      searchAt - length,
                                                                      search.caseSensitivity);
                        if (found != -1) {
                            return ZDocumentFindAsyncResultNew({found, line},
                                                               {found + length, line},
                                                               snap.revision(),
                                                               QRegularExpressionMatch{});
                        }
                    }
                }
                if (canceler.isCanceled()) {
                    return noMatch(snap);
                }
                line -= 1;
                if (line >= 0) {
                    searchAt = snap.line(line).size();
                }
            }
            if (!search.searchWrap || hasWrapped) {
                return noMatch(snap);
            }
            hasWrapped = true;
            end = search.startAtLine;
            line = snap.lineCount() - 1;
            searchAt = snap.lineCodeUnits(line);
        }
    }

    template <typename CANCEL>
    static ZDocumentFindAsyncResult snapshotSearchBackwards(ZDocumentSnapshot snap, SearchParameter search, CANCEL &canceler) {
        const bool regularExpressionMode = std::holds_alternative<QRegularExpression>(search.needle);
        if (regularExpressionMode) {
            return snapshotSearchBackwardsRegex(snap, search, canceler);
        } else {
            return snapshotSearchBackwardsLiteral(snap, search, canceler);
        }
    }

    SearchParameter prepareSearchParameter(const ZDocument *doc, const ZDocumentCursor &start, ZDocument::FindFlags options) {
        SearchParameter res;

        res.searchWrap = options & ZDocument::FindFlag::FindWrap;
        res.caseSensitivity = (options & ZDocument::FindFlag::FindCaseSensitively) ? Qt::CaseSensitive : Qt::CaseInsensitive;

        auto [startCodeUnit, startLine] = start.selectionEndPos();
        if (options & ZDocument::FindFlag::FindBackward) {
            if (start.hasSelection()) {
                if (options & ZDocument::FindFlag::FindWrap || startLine > 0 || startCodeUnit > 1) {
                    startCodeUnit = startCodeUnit - 1;
                    if (startCodeUnit < 0) {
                        if (--startLine < 0) {
                            startLine = doc->lineCount() - 1;
                        }
                        startCodeUnit = doc->lineCodeUnits(startLine);
                    }
                }
            }
        }

        res.startAtLine = startLine;
        res.startCodeUnit = startCodeUnit;
        return res;
    }

    void searchResultToTextCursor(ZDocumentCursor &cur, const ZDocumentFindAsyncResult &result) {
        if (result.anchor() == result.cursor()) {
            cur.clearSelection();
            return;
        }

        cur.setPosition(result.anchor());
        cur.setPosition(result.cursor(), true);
    }

    class SearchOnThread : public QRunnable {
    public:
        void run() override {
            if (promise.isCanceled()) {
                promise.reportFinished();
                return;
            }

            ZDocumentFindAsyncResult res = noMatch(snap);
            if (backwards) {
                res = snapshotSearchBackwards(snap, param, promise);
            } else {
                res = snapshotSearchForward(snap, param, promise);
            }

            promise.reportResult(res);

            promise.reportFinished();
        }

    public:
        QFutureInterface<ZDocumentFindAsyncResult> promise;
        ZDocumentSnapshot snap;
        SearchParameter param;
        bool backwards = false;
    };
}

ZDocumentCursor ZDocument::findSync(const QString &subString, const ZDocumentCursor &start,
                                    ZDocument::FindFlags options) const {
    ZDocumentCursor res = start;

    if (subString.isEmpty()) {
        res.clearSelection();
        return res;
    }

    SearchParameter param = prepareSearchParameter(this, start, options);
    param.needle = subString;

    ZDocumentFindAsyncResult resTmp = noMatch(revision());
    NoCanceler noCancler;
    if (options & ZDocument::FindFlag::FindBackward) {
        resTmp = snapshotSearchBackwards(snapshot(), param, noCancler);
    } else {
        resTmp = snapshotSearchForward(snapshot(), param, noCancler);
    }

    searchResultToTextCursor(res, resTmp);

    return res;
}

ZDocumentFindResult ZDocument::findSyncWithDetails(const QRegularExpression &regex, const ZDocumentCursor &start,
                                                   ZDocument::FindFlags options) const {
    ZDocumentCursor res = start;

    if (!regex.isValid()) {
        res.clearSelection();
        return {res, QRegularExpressionMatch{}};
    }

    SearchParameter param = prepareSearchParameter(this, start, options);
    param.needle = regex;

    ZDocumentFindAsyncResult resTmp = noMatch(revision());
    NoCanceler noCancler;
    if (options & ZDocument::FindFlag::FindBackward) {
        resTmp = snapshotSearchBackwards(snapshot(), param, noCancler);
    } else {
        resTmp = snapshotSearchForward(snapshot(), param, noCancler);
    }

    searchResultToTextCursor(res, resTmp);

    return {res, resTmp.tuiwidgets_impl()->match};
}

ZDocumentCursor ZDocument::findSync(const QRegularExpression &regex, const ZDocumentCursor &start,
                                    ZDocument::FindFlags options) const {
    return findSyncWithDetails(regex, start, options).cursor();
}

QFuture<ZDocumentFindAsyncResult> ZDocument::findAsync(const QString &subString, const ZDocumentCursor &start,
                                                       ZDocument::FindFlags options) const {
    return findAsyncWithPool(QThreadPool::globalInstance(), 0, subString, start, options);
}

QFuture<ZDocumentFindAsyncResult> ZDocument::findAsync(const QRegularExpression &regex, const ZDocumentCursor &start,
                                                       ZDocument::FindFlags options) const {
    return findAsyncWithPool(QThreadPool::globalInstance(), 0, regex, start, options);
}

QFuture<ZDocumentFindAsyncResult> ZDocument::findAsyncWithPool(QThreadPool *pool, int priority,
                                                               const QString &subString, const ZDocumentCursor &start,
                                                               ZDocument::FindFlags options) const {

    QFutureInterface<ZDocumentFindAsyncResult> promise;

    QFuture<ZDocumentFindAsyncResult> future = promise.future();

    promise.reportStarted();

    if (subString.isEmpty()) {
        ZDocumentFindAsyncResult res = ZDocumentFindAsyncResultNew(ZDocumentCursor::Position(0, 0),
                                                                   ZDocumentCursor::Position(0, 0),
                                                                   revision(), QRegularExpressionMatch{});

        promise.reportResult(res);
        promise.reportFinished();
        return future;
    }

    SearchParameter param = prepareSearchParameter(this, start, options);
    param.needle = subString;

    SearchOnThread *runnable = new SearchOnThread();
    runnable->param = param;
    runnable->backwards = options & ZDocument::FindFlag::FindBackward;
    runnable->snap = snapshot();
    runnable->promise = std::move(promise);

    pool->start(runnable, priority);

    return future;
}

QFuture<ZDocumentFindAsyncResult> ZDocument::findAsyncWithPool(QThreadPool *pool, int priority,
                                                               const QRegularExpression &regex, const ZDocumentCursor &start,
                                                               ZDocument::FindFlags options) const {

    QFutureInterface<ZDocumentFindAsyncResult> promise;

    QFuture<ZDocumentFindAsyncResult> future = promise.future();

    promise.reportStarted();

    if (!regex.isValid()) {
        ZDocumentFindAsyncResult res = ZDocumentFindAsyncResultNew(ZDocumentCursor::Position(0, 0),
                                                                   ZDocumentCursor::Position(0, 0),
                                                                   revision(), QRegularExpressionMatch{});

        promise.reportResult(res);
        promise.reportFinished();
        return future;
    }

    SearchParameter param = prepareSearchParameter(this, start, options);
    param.needle = regex;

    SearchOnThread *runnable = new SearchOnThread();
    runnable->param = param;
    runnable->backwards = options & ZDocument::FindFlag::FindBackward;
    runnable->snap = snapshot();
    runnable->promise = std::move(promise);

    pool->start(runnable, priority);

    return future;
}

ZDocumentFindAsyncResult::ZDocumentFindAsyncResult()
    : tuiwidgets_pimpl_ptr(std::make_unique<ZDocumentFindAsyncResultPrivate>())
{
}

ZDocumentFindAsyncResult::ZDocumentFindAsyncResult(const ZDocumentFindAsyncResult &other)
    : tuiwidgets_pimpl_ptr(std::make_unique<ZDocumentFindAsyncResultPrivate>(*other.tuiwidgets_impl()))
{
}

ZDocumentFindAsyncResult::ZDocumentFindAsyncResult(std::unique_ptr<ZDocumentFindAsyncResultPrivate> impl)
    : tuiwidgets_pimpl_ptr(std::move(impl))
{
}

ZDocumentFindAsyncResult::~ZDocumentFindAsyncResult() {}

ZDocumentFindAsyncResult &ZDocumentFindAsyncResult::operator=(const ZDocumentFindAsyncResult &other) {
    *tuiwidgets_impl() = *other.tuiwidgets_impl();
    return *this;
}

ZDocumentCursor::Position ZDocumentFindAsyncResult::anchor() const {
    auto *const p = tuiwidgets_impl();
    return p->anchor;
}

ZDocumentCursor::Position ZDocumentFindAsyncResult::cursor() const {
    auto *const p = tuiwidgets_impl();
    return p->cursor;
}

unsigned ZDocumentFindAsyncResult::revision() const {
    auto *const p = tuiwidgets_impl();
    return p->revision;
}

ZDocumentFindAsyncResultPrivate::ZDocumentFindAsyncResultPrivate() {
}

ZDocumentFindAsyncResultPrivate::ZDocumentFindAsyncResultPrivate(ZDocumentCursor::Position anchorIn,
                                                                 ZDocumentCursor::Position cursorIn,
                                                                 unsigned revisionIn,
                                                                 QRegularExpressionMatch matchIn) {
    anchor = anchorIn;
    cursor = cursorIn;
    revision = revisionIn;
    match = matchIn;
}

int ZDocumentFindAsyncResult::regexLastCapturedIndex() const {
    return tuiwidgets_impl()->match.lastCapturedIndex();
}

QString ZDocumentFindAsyncResult::regexCapture(int index) const {
    return tuiwidgets_impl()->match.captured(index);
}

QString ZDocumentFindAsyncResult::regexCapture(const QString &name) const {
    return tuiwidgets_impl()->match.captured(name);
}


ZDocumentFindResult::ZDocumentFindResult(const ZDocumentFindResult &other)
    : tuiwidgets_pimpl_ptr(std::make_unique<ZDocumentFindResultPrivate>(*other.tuiwidgets_impl()))
{
}

ZDocumentFindResult &ZDocumentFindResult::operator=(const ZDocumentFindResult &other) {
    *tuiwidgets_impl() = *other.tuiwidgets_impl();
    return *this;
}

ZDocumentCursor ZDocumentFindResult::cursor() const {
    auto *const p = tuiwidgets_impl();
    return p->cursor;
}

int ZDocumentFindResult::regexLastCapturedIndex() const {
    return tuiwidgets_impl()->_match.lastCapturedIndex();
}

QString ZDocumentFindResult::regexCapture(int index) const {
    return tuiwidgets_impl()->_match.captured(index);
}

QString ZDocumentFindResult::regexCapture(const QString &name) const {
    return tuiwidgets_impl()->_match.captured(name);
}

ZDocumentFindResult::ZDocumentFindResult(ZDocumentCursor cursor, QRegularExpressionMatch match) {
    tuiwidgets_pimpl_ptr = std::make_unique<ZDocumentFindResultPrivate>(cursor);
    auto *const p = tuiwidgets_impl();

    p->_match = match;
}

ZDocumentFindResult::~ZDocumentFindResult() {}

TUIWIDGETS_NS_END
