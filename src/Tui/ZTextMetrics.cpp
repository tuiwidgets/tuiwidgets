#include "ZTextMetrics.h"

#include "Tui/ZTextMetrics_p.h"

TUIWIDGETS_NS_START

ZTextMetrics::ZTextMetrics(const ZTextMetrics& other) : tuiwidgets_pimpl_ptr(other.tuiwidgets_pimpl_ptr)
{
}

ZTextMetrics::~ZTextMetrics() {
}

ZTextMetrics::ClusterSize ZTextMetrics::nextCluster(const QString &data, int offset) {
    return nextCluster(data.constData()+offset, data.size() - offset);
}

ZTextMetrics::ClusterSize ZTextMetrics::nextCluster(const QChar *data, int size) {
    return nextCluster(reinterpret_cast<const char16_t*>(data), size);
}

ZTextMetrics::ClusterSize ZTextMetrics::nextCluster(const char16_t *data, int size) {
    const auto* p = tuiwidgets_impl();
    // TODO have some kind of pool
    termpaint_text_measurement* tm = termpaint_text_measurement_new(p->surface);
    termpaint_text_measurement_set_limit_clusters(tm, 1);
    termpaint_text_measurement_feed_utf16(tm, reinterpret_cast<const uint16_t*>(data), size, true);
    ClusterSize result;
    result.codePoints = termpaint_text_measurement_last_codepoints(tm);
    result.codeUnits = termpaint_text_measurement_last_ref(tm);
    result.columns = termpaint_text_measurement_last_width(tm);
    termpaint_text_measurement_free(tm);
    return result;
}

ZTextMetrics::ClusterSize ZTextMetrics::nextCluster(const char *stringUtf8, int utf8CodeUnits) {
    const auto* p = tuiwidgets_impl();
    // TODO have some kind of pool
    termpaint_text_measurement* tm = termpaint_text_measurement_new(p->surface);
    termpaint_text_measurement_set_limit_clusters(tm, 1);
    termpaint_text_measurement_feed_utf8(tm, stringUtf8, utf8CodeUnits, true);
    ClusterSize result;
    result.codePoints = termpaint_text_measurement_last_codepoints(tm);
    result.codeUnits = termpaint_text_measurement_last_ref(tm);
    result.columns = termpaint_text_measurement_last_width(tm);
    termpaint_text_measurement_free(tm);
    return result;
}

ZTextMetrics::ClusterSize ZTextMetrics::splitByColumns(const QString &data, int maxWidth) {
    return ZTextMetrics::splitByColumns(data.constData(), data.size(), maxWidth);
}

ZTextMetrics::ClusterSize ZTextMetrics::splitByColumns(const QChar *data, int size, int maxWidth) {
    return ZTextMetrics::splitByColumns(reinterpret_cast<const char16_t*>(data), size, maxWidth);
}

ZTextMetrics::ClusterSize ZTextMetrics::splitByColumns(const char16_t *data, int size, int maxWidth) {
    const auto* p = tuiwidgets_impl();
    termpaint_text_measurement* tm = termpaint_text_measurement_new(p->surface);
    termpaint_text_measurement_set_limit_width(tm, maxWidth);
    termpaint_text_measurement_feed_utf16(tm, reinterpret_cast<const uint16_t*>(data), size, true);

    ClusterSize result;
    result.codePoints = termpaint_text_measurement_last_codepoints(tm);
    result.codeUnits = termpaint_text_measurement_last_ref(tm);
    result.columns = termpaint_text_measurement_last_width(tm);
    termpaint_text_measurement_free(tm);
    return result;
}

ZTextMetrics::ClusterSize ZTextMetrics::splitByColumns(const char *stringUtf8, int utf8CodeUnits, int maxWidth) {
    const auto* p = tuiwidgets_impl();
    termpaint_text_measurement* tm = termpaint_text_measurement_new(p->surface);
    termpaint_text_measurement_set_limit_width(tm, maxWidth);
    termpaint_text_measurement_feed_utf8(tm, stringUtf8, utf8CodeUnits, true);

    ClusterSize result;
    result.codePoints = termpaint_text_measurement_last_codepoints(tm);
    result.codeUnits = termpaint_text_measurement_last_ref(tm);
    result.columns = termpaint_text_measurement_last_width(tm);
    termpaint_text_measurement_free(tm);
    return result;
}

int ZTextMetrics::sizeInColumns(const QString &data) {
    return sizeInColumns(data.constData(), data.size());
}

int ZTextMetrics::sizeInColumns(const QChar *data, int size) {
    return sizeInColumns(reinterpret_cast<const char16_t*>(data), size);
}

int ZTextMetrics::sizeInColumns(const char16_t *data, int size) {
    const auto* p = tuiwidgets_impl();
    termpaint_text_measurement* tm = termpaint_text_measurement_new(p->surface);
    termpaint_text_measurement_feed_utf16(tm, reinterpret_cast<const uint16_t*>(data), size, true);

    int res = termpaint_text_measurement_last_width(tm);
    termpaint_text_measurement_free(tm);
    return res;
}

int ZTextMetrics::sizeInColumns(const char *stringUtf8, int utf8CodeUnits) {
    const auto* p = tuiwidgets_impl();
    termpaint_text_measurement* tm = termpaint_text_measurement_new(p->surface);
    termpaint_text_measurement_feed_utf8(tm, stringUtf8, utf8CodeUnits, true);

    int res = termpaint_text_measurement_last_width(tm);
    termpaint_text_measurement_free(tm);
    return res;
}

ZTextMetrics::ZTextMetrics(std::shared_ptr<ZTextMetricsPrivate> impl) : tuiwidgets_pimpl_ptr(impl)
{
}

Tui::ZTextMetricsPrivate::ZTextMetricsPrivate(termpaint_surface *surface) : surface(surface)
{
}

TUIWIDGETS_NS_END
