#ifndef TUIWIDGETS_ZTEXTMETRICS_INCLUDED
#define TUIWIDGETS_ZTEXTMETRICS_INCLUDED

#include <memory>
#include <string>
#if defined(__cpp_lib_string_view) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
#include <string_view>
#endif

#include <QString>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZTextMetricsPrivate;

class TUIWIDGETS_EXPORT ZTextMetrics {
public:
    ZTextMetrics(const ZTextMetrics& other);
    virtual ~ZTextMetrics();

public:
    struct ClusterSize {
        int codeUnits;
        int codePoints;
        int columns;
    };

    ClusterSize nextCluster(const QString &data, int offset) const;
    ClusterSize nextCluster(const QChar *data, int size) const;
    ClusterSize nextCluster(const char32_t *data, int size) const;
    ClusterSize nextCluster(const char16_t *data, int size) const;
    ClusterSize nextCluster(const char *stringUtf8, int utf8CodeUnits) const;
    ClusterSize splitByColumns(const QString &data, int maxWidth) const;
    ClusterSize splitByColumns(const QChar *data, int size, int maxWidth) const;
    ClusterSize splitByColumns(const char32_t *data, int size, int maxWidth) const;
    ClusterSize splitByColumns(const char16_t *data, int size, int maxWidth) const;
    ClusterSize splitByColumns(const char *stringUtf8, int utf8CodeUnits, int maxWidth) const;
    int sizeInColumns(const QString &data) const;
    int sizeInColumns(const QChar *data, int size) const;
    int sizeInColumns(const char32_t *data, int size) const;
    int sizeInColumns(const char16_t *data, int size) const;
    int sizeInColumns(const char *stringUtf8, int utf8CodeUnits) const;

    // Wrappers for more modern types:
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
    template <typename QSTRINGVIEW, Private::enable_if_same_remove_cvref<QSTRINGVIEW, QStringView> = 0>
    TUIWIDGETS_ABI_FORCE_INLINE ClusterSize nextCluster(QSTRINGVIEW data) const {
        return nextCluster(data.data(), data.size());
    }
    template <typename QSTRINGVIEW, Private::enable_if_same_remove_cvref<QSTRINGVIEW, QStringView> = 0>
    TUIWIDGETS_ABI_FORCE_INLINE ZTextMetrics::ClusterSize splitByColumns(QSTRINGVIEW data, int maxWidth) const {
        return splitByColumns(data.data(), data.size(), maxWidth);
    }
    template <typename QSTRINGVIEW, Private::enable_if_same_remove_cvref<QSTRINGVIEW, QStringView> = 0>
    TUIWIDGETS_ABI_FORCE_INLINE int sizeInColumns(QSTRINGVIEW data) const {
        return sizeInColumns(data.data(), data.size());
    }
#endif
#if defined(__cpp_lib_string_view) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
    template <typename U16STRINGVIEW, Private::enable_if_same_remove_cvref<U16STRINGVIEW, std::u16string_view> = 0>
    TUIWIDGETS_ABI_FORCE_INLINE ClusterSize nextCluster(U16STRINGVIEW data) {
        return nextCluster(data.data(), data.size());
    }
    template <typename U16STRINGVIEW, Private::enable_if_same_remove_cvref<U16STRINGVIEW, std::u16string_view> = 0>
    TUIWIDGETS_ABI_FORCE_INLINE ZTextMetrics::ClusterSize splitByColumns(U16STRINGVIEW data, int maxWidth) {
        return splitByColumns(data.data(), data.size(), maxWidth);
    }
    template <typename U16STRINGVIEW, Private::enable_if_same_remove_cvref<U16STRINGVIEW, std::u16string_view> = 0>
    TUIWIDGETS_ABI_FORCE_INLINE int sizeInColumns(U16STRINGVIEW data) {
        return sizeInColumns(data.data(), data.size());
    }

    // Assumes utf8 in string_view
    template <typename STRINGVIEW, Private::enable_if_same_remove_cvref<STRINGVIEW, std::string_view> = 0>
    TUIWIDGETS_ABI_FORCE_INLINE ClusterSize nextCluster(STRINGVIEW data) {
        return nextCluster(data.data(), data.size());
    }
    template <typename STRINGVIEW, Private::enable_if_same_remove_cvref<STRINGVIEW, std::string_view> = 0>
    TUIWIDGETS_ABI_FORCE_INLINE ZTextMetrics::ClusterSize splitByColumns(STRINGVIEW data, int maxWidth) {
        return splitByColumns(data.data(), data.size(), maxWidth);
    }
    template <typename STRINGVIEW, Private::enable_if_same_remove_cvref<STRINGVIEW, std::string_view> = 0>
    TUIWIDGETS_ABI_FORCE_INLINE int sizeInColumns(STRINGVIEW data) {
        return sizeInColumns(data.data(), data.size());
    }
#endif

    ZTextMetrics &operator=(const ZTextMetrics&);

private:
    friend class ZTerminal;
    friend class ZPainter;
    friend class ZTextMetricsPrivate;
    ZTextMetrics(std::shared_ptr<ZTextMetricsPrivate> impl);

protected:
    std::shared_ptr<ZTextMetricsPrivate> tuiwidgets_pimpl_ptr;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZTextMetrics)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZTEXTMETRICS_INCLUDED
