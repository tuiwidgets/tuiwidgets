#ifndef CATCHQT_H
#define CATCHQT_H

#include <QString>
#include <QRect>

#include <Tui/ZColor.h>

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

    template<>
    struct StringMaker<QPoint, void> {
        static std::string convert(QPoint const& value) {
            return QStringLiteral("(x: %0, y: %1)").arg(
                        QString::number(value.x()),
                        QString::number(value.y())).toStdString();
        }
    };

    template<>
    struct StringMaker<QSize, void> {
        static std::string convert(QSize const& value) {
            return QStringLiteral("(width: %0, height: %1)").arg(
                        QString::number(value.width()),
                        QString::number(value.height())).toStdString();
        }
    };

    template<>
    struct StringMaker<Tui::ZColor, void> {
        static std::string convert(Tui::ZColor const& value) {
            if (value.colorType() == Tui::ZColor::RGB) {
                return QStringLiteral("(color: rgb(%0, %1, %2))").arg(QString::number(value.red()),
                                                                      QString::number(value.green()),
                                                                      QString::number(value.blue())).toStdString();
            } else if (value.colorType() == Tui::ZColor::Default) {
                return "(color: default)";
            } else if (value.colorType() == Tui::ZColor::Terminal) {
                return QStringLiteral("(color: %0)").arg(QString::number(static_cast<int>(value.terminalColor()))).toStdString();
            } else if (value.colorType() == Tui::ZColor::TerminalIndexed) {
                return QStringLiteral("(color indexed: %0)").arg(QString::number(value.terminalColorIndexed())).toStdString();
            } else {
                return "(invalid color)";
            }
        }
    };
}

#endif
