#ifndef TUIWIDGETS_ZIMAGE_INCLUDED
#define TUIWIDGETS_ZIMAGE_INCLUDED

#include <memory>

#include <QSharedDataPointer>
#include <QSize>

#include <Tui/ZCommon.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZTerminal;
class ZPainter;
class ZColor;

class ZImageData;

class TUIWIDGETS_EXPORT ZImage {
public:
    ZImage(const ZTerminal *terminal, int width, int height);
    static std::unique_ptr<ZImage> fromFile(const ZTerminal *terminal, QString fileName);
    static std::unique_ptr<ZImage> fromByteArray(const ZTerminal *terminal, QByteArray data);
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

    QString peekText(int x, int y, int *left, int *right);
    ZColor peekForground(int x, int y);
    ZColor peekBackground(int x, int y);
    ZColor peekDecoration(int x, int y);
    bool peekSoftwrapMarker(int x, int y);
    ZTextAttributes peekAttributes(int x, int y);

    ZPainter painter();

private:
    friend class ZImageData;
    QSharedDataPointer<ZImageData> tuiwidgets_pimpl_ptr;
    ZImage(QSharedDataPointer<ZImageData> pimpl);
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZIMAGE_INCLUDED
