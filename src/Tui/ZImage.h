#ifndef TUIWIDGETS_ZIMAGE_INCLUDED
#define TUIWIDGETS_ZIMAGE_INCLUDED

#include <memory>

#include <QSharedDataPointer>
#include <QSize>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZTerminal;
class ZPainter;

class ZImageData;

class TUIWIDGETS_EXPORT ZImage {
public:
    ZImage(const ZTerminal* terminal, int width, int height);
    ZImage(const ZTerminal *terminal, const QString& fileName); // this throws
    static ZImage *fromFile(const ZTerminal *terminal, QString fileName);
    static ZImage *fromByteArray(const ZTerminal *terminal, QByteArray data);
    ZImage(const ZImage& other);
    ZImage(ZImage&& other);
    ZImage& operator=(const ZImage& other);
    ZImage& operator=(ZImage&& other);
    virtual ~ZImage();
    bool operator==(const ZImage& other);
    bool operator!=(const ZImage& other);

    void swap(ZImage &other);

public:
    int width() const;
    int height() const;
    QSize size() const;

    bool save(const QString &fileName) const;
    QByteArray saveToByteArray() const;

    ZPainter painter();

private:
    friend class ZImageData;
    QSharedDataPointer<ZImageData> tuiwidgets_pimpl_ptr;
    ZImage(QSharedDataPointer<ZImageData> pimpl);
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZIMAGE_INCLUDED
