#include "ZTextMetrics.h"

#include "Tui/ZTextMetrics_p.h"

TUIWIDGETS_NS_START

ZTextMetrics::ZTextMetrics(const ZTextMetrics& other) : tuiwidgets_pimpl_ptr(other.tuiwidgets_pimpl_ptr)
{
}

ZTextMetrics::~ZTextMetrics() {
}

ZTextMetrics::ClusterSize ZTextMetrics::nextCluster(const QString &data, int offset) const {
    return nextCluster(data.constData() + offset, data.size() - offset);
}

ZTextMetrics::ClusterSize ZTextMetrics::nextCluster(const QChar *data, int size) const {
    return nextCluster(reinterpret_cast<const char16_t*>(data), size);
}

ZTextMetrics::ClusterSize ZTextMetrics::nextCluster(const char32_t *data, int size) const {
    const auto *const p = tuiwidgets_impl();
    // TODO have some kind of pool
    termpaint_text_measurement *tm = termpaint_text_measurement_new(p->surface);
    termpaint_text_measurement_set_limit_clusters(tm, 1);
    termpaint_text_measurement_feed_utf32(tm, reinterpret_cast<const uint32_t*>(data), size, true);
    ClusterSize result;
    result.codePoints = termpaint_text_measurement_last_codepoints(tm);
    result.codeUnits = termpaint_text_measurement_last_ref(tm);
    result.columns = termpaint_text_measurement_last_width(tm);
    termpaint_text_measurement_free(tm);
    return result;
}

ZTextMetrics::ClusterSize ZTextMetrics::nextCluster(const char16_t *data, int size) const {
    const auto *const p = tuiwidgets_impl();
    // TODO have some kind of pool
    termpaint_text_measurement *tm = termpaint_text_measurement_new(p->surface);
    termpaint_text_measurement_set_limit_clusters(tm, 1);
    termpaint_text_measurement_feed_utf16(tm, reinterpret_cast<const uint16_t*>(data), size, true);
    ClusterSize result;
    result.codePoints = termpaint_text_measurement_last_codepoints(tm);
    result.codeUnits = termpaint_text_measurement_last_ref(tm);
    result.columns = termpaint_text_measurement_last_width(tm);
    termpaint_text_measurement_free(tm);
    return result;
}

ZTextMetrics::ClusterSize ZTextMetrics::nextCluster(const char *stringUtf8, int utf8CodeUnits) const {
    const auto *const p = tuiwidgets_impl();
    // TODO have some kind of pool
    termpaint_text_measurement *tm = termpaint_text_measurement_new(p->surface);
    termpaint_text_measurement_set_limit_clusters(tm, 1);
    termpaint_text_measurement_feed_utf8(tm, stringUtf8, utf8CodeUnits, true);
    ClusterSize result;
    result.codePoints = termpaint_text_measurement_last_codepoints(tm);
    result.codeUnits = termpaint_text_measurement_last_ref(tm);
    result.columns = termpaint_text_measurement_last_width(tm);
    termpaint_text_measurement_free(tm);
    return result;
}

ZTextMetrics::ClusterSize ZTextMetrics::splitByColumns(const QString &data, int maxWidth) const {
    return ZTextMetrics::splitByColumns(data.constData(), data.size(), maxWidth);
}

ZTextMetrics::ClusterSize ZTextMetrics::splitByColumns(const QChar *data, int size, int maxWidth) const {
    return ZTextMetrics::splitByColumns(reinterpret_cast<const char16_t*>(data), size, maxWidth);
}

ZTextMetrics::ClusterSize ZTextMetrics::splitByColumns(const char32_t *data, int size, int maxWidth) const {
    const auto *const p = tuiwidgets_impl();
    termpaint_text_measurement *tm = termpaint_text_measurement_new(p->surface);
    termpaint_text_measurement_set_limit_width(tm, maxWidth);
    termpaint_text_measurement_feed_utf32(tm, reinterpret_cast<const uint32_t*>(data), size, true);

    ClusterSize result;
    result.codePoints = termpaint_text_measurement_last_codepoints(tm);
    result.codeUnits = termpaint_text_measurement_last_ref(tm);
    result.columns = termpaint_text_measurement_last_width(tm);
    termpaint_text_measurement_free(tm);
    return result;
}

ZTextMetrics::ClusterSize ZTextMetrics::splitByColumns(const char16_t *data, int size, int maxWidth) const {
    const auto *const p = tuiwidgets_impl();
    termpaint_text_measurement *tm = termpaint_text_measurement_new(p->surface);
    termpaint_text_measurement_set_limit_width(tm, maxWidth);
    termpaint_text_measurement_feed_utf16(tm, reinterpret_cast<const uint16_t*>(data), size, true);

    ClusterSize result;
    result.codePoints = termpaint_text_measurement_last_codepoints(tm);
    result.codeUnits = termpaint_text_measurement_last_ref(tm);
    result.columns = termpaint_text_measurement_last_width(tm);
    termpaint_text_measurement_free(tm);
    return result;
}

ZTextMetrics::ClusterSize ZTextMetrics::splitByColumns(const char *stringUtf8, int utf8CodeUnits, int maxWidth) const {
    const auto *const p = tuiwidgets_impl();
    termpaint_text_measurement *tm = termpaint_text_measurement_new(p->surface);
    termpaint_text_measurement_set_limit_width(tm, maxWidth);
    termpaint_text_measurement_feed_utf8(tm, stringUtf8, utf8CodeUnits, true);

    ClusterSize result;
    result.codePoints = termpaint_text_measurement_last_codepoints(tm);
    result.codeUnits = termpaint_text_measurement_last_ref(tm);
    result.columns = termpaint_text_measurement_last_width(tm);
    termpaint_text_measurement_free(tm);
    return result;
}

int ZTextMetrics::sizeInColumns(const QString &data) const {
    return sizeInColumns(data.constData(), data.size());
}

int ZTextMetrics::sizeInColumns(const QChar *data, int size) const {
    return sizeInColumns(reinterpret_cast<const char16_t*>(data), size);
}

int ZTextMetrics::sizeInColumns(const char32_t *data, int size) const {
    const auto *const p = tuiwidgets_impl();
    termpaint_text_measurement *tm = termpaint_text_measurement_new(p->surface);
    termpaint_text_measurement_feed_utf32(tm, reinterpret_cast<const uint32_t*>(data), size, true);

    int res = termpaint_text_measurement_last_width(tm);
    termpaint_text_measurement_free(tm);
    return res;
}

int ZTextMetrics::sizeInColumns(const char16_t *data, int size) const {
    const auto *const p = tuiwidgets_impl();
    termpaint_text_measurement *tm = termpaint_text_measurement_new(p->surface);
    termpaint_text_measurement_feed_utf16(tm, reinterpret_cast<const uint16_t*>(data), size, true);

    int res = termpaint_text_measurement_last_width(tm);
    termpaint_text_measurement_free(tm);
    return res;
}

int ZTextMetrics::sizeInColumns(const char *stringUtf8, int utf8CodeUnits) const {
    const auto *const p = tuiwidgets_impl();
    termpaint_text_measurement *tm = termpaint_text_measurement_new(p->surface);
    termpaint_text_measurement_feed_utf8(tm, stringUtf8, utf8CodeUnits, true);

    int res = termpaint_text_measurement_last_width(tm);
    termpaint_text_measurement_free(tm);
    return res;
}

int ZTextMetrics::sizeInClusters(const QString &data) const {
    return sizeInClusters(data.constData(), data.size());
}

int ZTextMetrics::sizeInClusters(const QChar *data, int size) const {
    return sizeInClusters(reinterpret_cast<const char16_t*>(data), size);
}

int ZTextMetrics::sizeInClusters(const char32_t *data, int size) const {
    const auto *const p = tuiwidgets_impl();
    termpaint_text_measurement *tm = termpaint_text_measurement_new(p->surface);
    termpaint_text_measurement_feed_utf32(tm, reinterpret_cast<const uint32_t*>(data), size, true);

    int res = termpaint_text_measurement_last_clusters(tm);
    termpaint_text_measurement_free(tm);
    return res;
}

int ZTextMetrics::sizeInClusters(const char16_t *data, int size) const {
    const auto *const p = tuiwidgets_impl();
    termpaint_text_measurement *tm = termpaint_text_measurement_new(p->surface);
    termpaint_text_measurement_feed_utf16(tm, reinterpret_cast<const uint16_t*>(data), size, true);

    int res = termpaint_text_measurement_last_clusters(tm);
    termpaint_text_measurement_free(tm);
    return res;
}

int ZTextMetrics::sizeInClusters(const char *stringUtf8, int utf8CodeUnits) const {
    const auto *const p = tuiwidgets_impl();
    termpaint_text_measurement *tm = termpaint_text_measurement_new(p->surface);
    termpaint_text_measurement_feed_utf8(tm, stringUtf8, utf8CodeUnits, true);

    int res = termpaint_text_measurement_last_clusters(tm);
    termpaint_text_measurement_free(tm);
    return res;
}

ZTextMetrics &ZTextMetrics::operator=(const ZTextMetrics&) = default;

ZTextMetrics::ZTextMetrics(std::shared_ptr<ZTextMetricsPrivate> impl) : tuiwidgets_pimpl_ptr(impl)
{
}

ZTextMetricsPrivate::ZTextMetricsPrivate(termpaint_surface *surface) : surface(surface)
{
}

ZTextMetricsPrivate::~ZTextMetricsPrivate() {
}

ZTextMetrics ZTextMetricsPrivate::createForTesting(termpaint_surface *surface) {
    return ZTextMetrics(std::make_shared<ZTextMetricsPrivate>(surface));
}

TUIWIDGETS_NS_END
