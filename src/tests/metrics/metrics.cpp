// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZTextMetrics.h>
#include <Tui/ZTextMetrics_p.h>

#include "../catchwrapper.h"

#include <termpaint.h>

#include <QVector>

#include "../termpaint_helpers.h"

namespace {

enum Kind {
    KindQString, KindQChar, KindChar16, KindChar32, KindUtf, KindQStringView, KindU16StringView, KindStringView
};

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
#define MAYBEKindQStringView , KindQStringView
#else
#define MAYBEKindQStringView
#endif

#if defined(__cpp_lib_string_view) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
#define MAYBEKindStringView , KindU16StringView, KindStringView
#else
#define MAYBEKindStringView
#endif

#define ALLKINDS KindQString, KindQChar, KindChar16, KindChar32, KindUtf MAYBEKindQStringView MAYBEKindStringView

Tui::ZTextMetrics::ClusterSize nextClusterWrapper(Kind kind, Tui::ZTextMetrics &tm, const QString &string) {
    switch (kind) {
        case KindQString:
            {
                QString padding = GENERATE(QString(""), QString("a"), QString("ab"),
                                           QString("はい"), QString("はい"), QString("😇"), QString("\n"), QString("\t") );
                UNSCOPED_INFO("padding: " << padding.toStdString());
                QString s = padding + string;
                return tm.nextCluster(s, padding.size());
            }
        case KindQChar:
            return tm.nextCluster(string.data(), string.size());
        case KindChar16:
            return tm.nextCluster(reinterpret_cast<const char16_t*>(string.data()), string.size());
        case KindChar32:
            {
                auto utf32 = string.toUcs4();
                return tm.nextCluster(reinterpret_cast<const char32_t*>(utf32.data()), utf32.size());
            }
        case KindUtf:
            {
                QByteArray utf8 = string.toUtf8();
                return tm.nextCluster(utf8.data(), utf8.size());
            }
        case KindQStringView:
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            return tm.nextCluster(QStringView{string});
#endif
            FAIL("KindQStringView: unexpected");
        case KindU16StringView:
#if defined(__cpp_lib_string_view) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            return tm.nextCluster(std::u16string_view(reinterpret_cast<const char16_t*>(string.data()), string.size()));
#endif
            FAIL("KindU16StringView: unexpected");
        case KindStringView:
#if defined(__cpp_lib_string_view) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            {
                QByteArray utf8 = string.toUtf8();
                return tm.nextCluster(std::string_view(utf8.data(), utf8.size()));
            }
#endif
            FAIL("KindStringView: unexpected");
    }
    FAIL("Unknown kind");
    return {};
}

Tui::ZTextMetrics::ClusterSize splitByColumnsWrapper(Kind kind, Tui::ZTextMetrics &tm, const QString &string, int maxWidth) {
    switch (kind) {
        case KindQString:
            return tm.splitByColumns(string, maxWidth);
        case KindQChar:
            return tm.splitByColumns(string.data(), string.size(), maxWidth);
        case KindChar16:
            return tm.splitByColumns(reinterpret_cast<const char16_t*>(string.data()), string.size(), maxWidth);
        case KindChar32:
            {
                auto utf32 = string.toUcs4();
                return tm.splitByColumns(reinterpret_cast<const char32_t*>(utf32.data()), utf32.size(), maxWidth);
            }
        case KindUtf:
            {
                QByteArray utf8 = string.toUtf8();
                return tm.splitByColumns(utf8.data(), utf8.size(), maxWidth);
            }
        case KindQStringView:
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            return tm.splitByColumns(QStringView{string}, maxWidth);
#endif
            FAIL("KindQStringView: unexpected");
        case KindU16StringView:
#if defined(__cpp_lib_string_view) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            return tm.splitByColumns(std::u16string_view(reinterpret_cast<const char16_t*>(string.data()), string.size()), maxWidth);
#endif
            FAIL("KindU16StringView: unexpected");
        case KindStringView:
#if defined(__cpp_lib_string_view) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            {
                QByteArray utf8 = string.toUtf8();
                return tm.splitByColumns(std::string_view(utf8.data(), utf8.size()), maxWidth);
            }
#endif
            FAIL("KindStringView: unexpected");
    }
    FAIL("Unknown kind");
    return {};
}


int sizeInColumnsWrapper(Kind kind, Tui::ZTextMetrics &tm, const QString &string) {
    switch (kind) {
        case KindQString:
            return tm.sizeInColumns(string);
        case KindQChar:
            return tm.sizeInColumns(string.data(), string.size());
        case KindChar16:
            return tm.sizeInColumns(reinterpret_cast<const char16_t*>(string.data()), string.size());
        case KindChar32:
            {
                auto utf32 = string.toUcs4();
                return tm.sizeInColumns(reinterpret_cast<const char32_t*>(utf32.data()), utf32.size());
            }
        case KindUtf:
            {
                QByteArray utf8 = string.toUtf8();
                return tm.sizeInColumns(utf8.data(), utf8.size());
            }
        case KindQStringView:
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            return tm.sizeInColumns(QStringView{string});
#endif
            FAIL("KindQStringView: unexpected");
        case KindU16StringView:
#if defined(__cpp_lib_string_view) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            return tm.sizeInColumns(std::u16string_view(reinterpret_cast<const char16_t*>(string.data()), string.size()));
#endif
            FAIL("KindU16StringView: unexpected");
        case KindStringView:
#if defined(__cpp_lib_string_view) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            {
                QByteArray utf8 = string.toUtf8();
                return tm.sizeInColumns(std::string_view(utf8.data(), utf8.size()));
            }
#endif
            FAIL("KindStringView: unexpected");
    }
    FAIL("Unknown kind");
    return 0;
}

int sizeInClustersWrapper(Kind kind, Tui::ZTextMetrics &tm, const QString &string) {
    switch (kind) {
        case KindQString:
            return tm.sizeInClusters(string);
        case KindQChar:
            return tm.sizeInClusters(string.data(), string.size());
        case KindChar16:
            return tm.sizeInClusters(reinterpret_cast<const char16_t*>(string.data()), string.size());
        case KindChar32:
            {
                auto utf32 = string.toUcs4();
                return tm.sizeInClusters(reinterpret_cast<const char32_t*>(utf32.data()), utf32.size());
            }
        case KindUtf:
            {
                QByteArray utf8 = string.toUtf8();
                return tm.sizeInClusters(utf8.data(), utf8.size());
            }
        case KindQStringView:
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            return tm.sizeInClusters(QStringView{string});
#endif
            FAIL("KindQStringView: unexpected");
        case KindU16StringView:
#if defined(__cpp_lib_string_view) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            return tm.sizeInClusters(std::u16string_view(reinterpret_cast<const char16_t*>(string.data()), string.size()));
#endif
            FAIL("KindU16StringView: unexpected");
        case KindStringView:
#if defined(__cpp_lib_string_view) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            {
                QByteArray utf8 = string.toUtf8();
                return tm.sizeInClusters(std::string_view(utf8.data(), utf8.size()));
            }
#endif
            FAIL("KindStringView: unexpected");
    }
    FAIL("Unknown kind");
    return 0;
}

int nCodeUnits(Kind kind, QString s) {
    switch (kind) {
        case KindQString:
        case KindQChar:
        case KindChar16:
        case KindQStringView:
        case KindU16StringView:
            return s.size();
        case KindUtf:
        case KindStringView:
            return s.toUtf8().size();
        case KindChar32:
            return s.toUcs4().size();
    }
    FAIL("Unknown kind");
    return 0;
}

int nCodePoints(QString s) {
    return s.toStdU32String().size();
}

}

TEST_CASE("metrics - copy and assigment") {
    TermpaintFixture f, f2;

    Tui::ZTextMetrics tm = Tui::ZTextMetricsPrivate::createForTesting(f.surface);
    Tui::ZTextMetrics tm2 = Tui::ZTextMetricsPrivate::createForTesting(f2.surface);

    REQUIRE(Tui::ZTextMetricsPrivate::get(&tm)->surface == f.surface);
    REQUIRE(Tui::ZTextMetricsPrivate::get(&tm2)->surface == f2.surface);

    Tui::ZTextMetrics tm3{tm};
    REQUIRE(Tui::ZTextMetricsPrivate::get(&tm3)->surface == f.surface);

    tm3 = tm2;
    REQUIRE(Tui::ZTextMetricsPrivate::get(&tm3)->surface == f2.surface);
}


TEST_CASE("metrics - nextCluster") {
    auto kind = GENERATE(ALLKINDS);

    struct TestCase { QString text; int columns; QString cluster; };
    const auto testCase = GENERATE(
                TestCase{ "test", 1, "t" },
                TestCase{ "はa", 2, "は" },
                TestCase{ "はい", 2, "は" },
                TestCase{ "😇bc", 2, "😇" },
                TestCase{"a\xcc\x88\xcc\xa4\x62\x63", 1, "a\xcc\x88\xcc\xa4"},
                TestCase{ "\n😇NewLine", 1, "\n" },
                TestCase{ "\tTab", 1, "\t" },
                TestCase{ "¹1", 1, "¹" },
                TestCase{ QString(1, QChar(0)) + "null", 1,  QString(1, QChar(0)) },
                TestCase{ "\x1b\x1b", 1, "\033" },
                TestCase{ "", 0, "" }
    );

    CAPTURE(kind);
    CAPTURE(testCase.text.toStdString());

    TermpaintFixture f, f2;

    Tui::ZTextMetrics tm = Tui::ZTextMetricsPrivate::createForTesting(f.surface);

    Tui::ZTextMetrics::ClusterSize result;
    result = nextClusterWrapper(kind, tm, testCase.text);
    CHECK(result.columns == testCase.columns);
    CHECK(result.codeUnits == nCodeUnits(kind, testCase.cluster));
    CHECK(result.codePoints == nCodePoints(testCase.cluster));

    // check copy contruction
    Tui::ZTextMetrics tm2 = tm;

    result = nextClusterWrapper(kind, tm2, "test");
    CHECK(result.columns == 1);
    CHECK(result.codeUnits == 1);
    CHECK(result.codePoints == 1);

    // check assignment does not break trivially
    Tui::ZTextMetrics tm3 = Tui::ZTextMetricsPrivate::createForTesting(f2.surface);
    tm2 = tm3;

    result = nextClusterWrapper(kind, tm3, "test");
    CHECK(result.columns == 1);
    CHECK(result.codeUnits == 1);
    CHECK(result.codePoints == 1);
}


TEST_CASE("metrics - splitByColumns") {
    auto kind = GENERATE(ALLKINDS);

    struct TestCase { QString text; int splitAt; int columns; QString left; };
    const auto testCase = GENERATE(
                TestCase{ "test", 1, 1, "t" },
                TestCase{ "test", 2, 2, "te" },
                TestCase{ "test", 4, 4, "test" },
                TestCase{ "test", 10, 4, "test" },
                TestCase{ "はい", 2, 2, "は" },
                TestCase{ "はい", 3, 2, "は" },
                TestCase{ "はい", 4, 4, "はい" },
                TestCase{ "😇bc", 2, 2, "😇" },
                TestCase{ "😇bc", 1, 0, "" },
                TestCase{ "😇😇", 3, 2, "😇" },
                TestCase{"a\xcc\x88\xcc\xa4\x62\x63", 1, 1, "a\xcc\x88\xcc\xa4"},
                TestCase{ "\n😇NewLine", 1, 1, "\n" },
                TestCase{ "\tTab", 1, 1, "\t" },
                TestCase{ "¹1", 1, 1, "¹" },
                TestCase{ QString(1, QChar(0)) + "null", 1, 1, QString(1, QChar(0)) },
                TestCase{ "\x1b\x1b", 1, 1, "\033" },
                TestCase{ "", 0, 0, "" },
                TestCase{ "", 2, 0, "" }
    );

    CAPTURE(kind);
    CAPTURE(testCase.text.toStdString());

    TermpaintFixture f, f2;

    Tui::ZTextMetrics tm = Tui::ZTextMetricsPrivate::createForTesting(f.surface);

    Tui::ZTextMetrics::ClusterSize result;
    result = splitByColumnsWrapper(kind, tm, testCase.text, testCase.splitAt);
    CHECK(result.columns == testCase.columns);
    CHECK(result.codeUnits == nCodeUnits(kind, testCase.left));
    CHECK(result.codePoints == nCodePoints(testCase.left));

    // check copy contruction
    Tui::ZTextMetrics tm2 = tm;

    result = splitByColumnsWrapper(kind, tm2, testCase.text, testCase.splitAt);
    CHECK(result.columns == testCase.columns);
    CHECK(result.codeUnits == nCodeUnits(kind, testCase.left));
    CHECK(result.codePoints == nCodePoints(testCase.left));

    // check assignment does not break trivially
    Tui::ZTextMetrics tm3 = Tui::ZTextMetricsPrivate::createForTesting(f2.surface);
    tm2 = tm3;

    result = splitByColumnsWrapper(kind, tm3, testCase.text, testCase.splitAt);
    CHECK(result.columns == testCase.columns);
    CHECK(result.codeUnits == nCodeUnits(kind, testCase.left));
    CHECK(result.codePoints == nCodePoints(testCase.left));
}

TEST_CASE("metrics - sizeInXXX") {
    auto kind = GENERATE(ALLKINDS);

    struct TestCase { QString text; int columns; int clusters; };
    const auto testCase = GENERATE(
                TestCase{ "test", 4, 4},
                TestCase{ "はい", 4, 2},
                TestCase{ "😇bc", 4, 3},
                TestCase{"a\xcc\x88\xcc\xa4\x62\x63", 3, 3},
                TestCase{ "😇a😇", 5, 3},
                TestCase{ "\n😇", 3, 2},
                TestCase{ "\tTab", 4, 4},
                TestCase{ "¹1", 2, 2},
                TestCase{ QString(1, QChar(0)) + "null", 5, 5},
                TestCase{ "null" + QString(1, QChar(0)), 5, 5},
                TestCase{ "\x1b\x1b", 2, 2},
                TestCase{ "", 0, 0}
    );

    CAPTURE(kind);
    CAPTURE(testCase.text.toStdString());

    TermpaintFixture f, f2;

    Tui::ZTextMetrics tm = Tui::ZTextMetricsPrivate::createForTesting(f.surface);

    CHECK(sizeInColumnsWrapper(kind, tm, testCase.text) == testCase.columns);
    CHECK(sizeInClustersWrapper(kind, tm, testCase.text) == testCase.clusters);

    // check copy contruction
    Tui::ZTextMetrics tm2 = tm;

    CHECK(sizeInColumnsWrapper(kind, tm2, testCase.text) == testCase.columns);
    CHECK(sizeInClustersWrapper(kind, tm2, testCase.text) == testCase.clusters);

    // check assignment does not break trivially
    Tui::ZTextMetrics tm3 = Tui::ZTextMetricsPrivate::createForTesting(f2.surface);
    tm2 = tm3;

    CHECK(sizeInColumnsWrapper(kind, tm3, testCase.text) == testCase.columns);
    CHECK(sizeInClustersWrapper(kind, tm3, testCase.text) == testCase.clusters);
}

