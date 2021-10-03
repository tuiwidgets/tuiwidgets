#ifndef CATCHQT_H
#define CATCHQT_H

#include <QString>
#include <QRect>

#include "../third-party/catch.hpp"

namespace Catch {
    template<>
    struct StringMaker<QString, void> {
        static std::string convert(QString const& value) {
            return value.toStdString();
        }
    };

    template<>
    struct StringMaker<QRect, void> {
        static std::string convert(QRect const& value) {
            return QStringLiteral("(x: %0, y: %1, w: %2, h: %3)").arg(
                        QString::number(value.x()),
                        QString::number(value.y()),
                        QString::number(value.width()),
                        QString::number(value.height())).toStdString();
        }
    };

}

#endif
