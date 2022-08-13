// SPDX-License-Identifier: BSL-1.0

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
    TUIWIDGETS_NODISCARD("return value contains loaded image")
    static std::unique_ptr<ZImage> fromFile(const ZTerminal *terminal, const QString &fileName);
    TUIWIDGETS_NODISCARD("return value contains loaded image")
    static std::unique_ptr<ZImage> fromByteArray(const ZTerminal *terminal, QByteArray data);
    ZImage(const ZImage& other);
    ZImage(ZImage&& other);
    ZImage& operator=(const ZImage& other);
    ZImage& operator=(ZImage&& other);
    virtual ~ZImage();

    bool operator==(const ZImage& other) const;
    bool operator!=(const ZImage& other) const;

    void swap(ZImage &other);

public:
    int width() const;
    int height() const;
    QSize size() const;

    bool save(const QString &fileName) const;
    QByteArray saveToByteArray() const;

    QString peekText(int x, int y, int *left, int *right) const;
    ZColor peekForground(int x, int y) const;
    ZColor peekBackground(int x, int y) const;
    ZColor peekDecoration(int x, int y) const;
    bool peekSoftwrapMarker(int x, int y) const;
    ZTextAttributes peekAttributes(int x, int y) const;

    ZPainter painter();

private:
    friend class ZImageData;
    QSharedDataPointer<ZImageData> tuiwidgets_pimpl_ptr;
    ZImage(QSharedDataPointer<ZImageData> pimpl);
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZIMAGE_INCLUDED
