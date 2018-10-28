#ifndef TUIWIDGETS_ZTEXTMETRICS_INCLUDED
#define TUIWIDGETS_ZTEXTMETRICS_INCLUDED

#include <memory>

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

    ClusterSize nextCluster(const QString &data, int offset);
    ClusterSize nextCluster(const QChar *data, int size);

private:
    friend class ZTerminal;
    ZTextMetrics(std::shared_ptr<ZTextMetricsPrivate> impl);

protected:
    std::shared_ptr<ZTextMetricsPrivate> tuiwidgets_pimpl_ptr;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZTextMetrics)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZTEXTMETRICS_INCLUDED
