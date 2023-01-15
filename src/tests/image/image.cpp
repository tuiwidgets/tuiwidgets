// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZImage.h>
#include <Tui/ZPainter.h>

#include <QFile>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTemporaryFile>

#include "../Testhelper.h"
#include "termpaint.h"

TEST_CASE("image", "") {
    Testhelper t("image", "image", 15, 5);

    SECTION("constructors") {
        Tui::ZImage image(t.terminal.get(), 0, 0);
        Tui::ZImage image2(image);
        Tui::ZImage image3(std::move(image));
    }

    Tui::ZImage image(t.terminal.get(), 14, 4);
    SECTION("default") {
        CHECK(image.width() == 14);
        CHECK(image.height() == 4);
        CHECK(image.size() == QSize{14, 4});
    }

    SECTION("save") {
        QTemporaryFile tfile;
        tfile.open();
        tfile.close();
        QString fileName = tfile.fileName();

        CAPTURE(fileName);
        CHECK(image.save(fileName) == true);

        QFile file(fileName);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QByteArray jsonData = file.readAll();
        file.close();

        QJsonDocument document = QJsonDocument::fromJson(jsonData);
        QJsonObject jobject = document.object();

        CHECK(jobject.value("termpaint_image") == true);
        CHECK(jobject.value("width").toInt() == 14);
        CHECK(jobject.value("height").toInt() == 4);
    }

    SECTION("fromFile") {
        QTemporaryFile tfile;
        tfile.open();
        tfile.close();
        QString fileName = tfile.fileName();
        image.painter().writeWithAttributes(1, 1, "stuff", {0xFF, 0xFF, 0xFF}, {0x80, 0x80, 0x80}, Tui::ZTextAttribute::Bold);

        CAPTURE(fileName);
        CHECK(image.save(fileName) == true);

        Tui::ZImage image2(t.terminal.get(), 80, 11);
        CHECK(image2 != image);

        auto res = Tui::ZImage::fromFile(t.terminal.get(), fileName);
        REQUIRE(res);
        image2 = *res;
        CHECK(image2.size() == QSize{14, 4});
        CHECK(image2 == image);
    }

    SECTION("fromFile-invalid") {
        QTemporaryFile tfile;
        tfile.open();
        tfile.close();
        QString fileName = tfile.fileName();

        Tui::ZImage image2(t.terminal.get(), 80, 11);
        CHECK(image2 != image);

        auto res = Tui::ZImage::fromFile(t.terminal.get(), fileName);
        CHECK(!res);
    }

    SECTION("saveToByteArray") {
        QByteArray jsonData = image.saveToByteArray();

        QJsonDocument document = QJsonDocument::fromJson(jsonData);
        QJsonObject jobject = document.object();

        CHECK(jobject.value("termpaint_image") == true);
        CHECK(jobject.value("width").toInt() == 14);
        CHECK(jobject.value("height").toInt() == 4);
    }

    SECTION("fromByteArray") {
        image.painter().writeWithAttributes(1, 1, "stuff", {0xFF, 0xFF, 0xFF}, {0x80, 0x80, 0x80}, Tui::ZTextAttribute::Bold);
        QByteArray byteData = image.saveToByteArray();
        Tui::ZImage image2(t.terminal.get(), 80, 11);
        CHECK(image2 != image);
        image2 = *Tui::ZImage::fromByteArray(t.terminal.get(), byteData);
        CHECK(image2.size() == QSize{14, 4});
        CHECK(image2 == image);
    }

    SECTION("fromByteArray-invalid") {
        auto res = Tui::ZImage::fromByteArray(t.terminal.get(), QByteArray("{}"));
        CHECK(!res);
    }

    SECTION("peekText") {
        int l = 3, r = 4;
        CHECK(image.peekText(1, 2, &l, &r) == TERMPAINT_ERASED);
        CHECK(l == 1);
        CHECK(r == 1);
        image.painter().writeWithAttributes(1, 1, "😎", {0xFF, 0xFF, 0xFF}, {0x80, 0x80, 0x80}, Tui::ZTextAttribute::Bold);
        CHECK(image.peekText(1, 1, &l, &r) == "😎");
        CHECK(l == 1);
        CHECK(r == 2);
    }

    SECTION("peekForground") {
        CHECK(image.peekForground(0, 0) == Tui::ZColor::defaultColor());
        CHECK(image.peekForground(-1, -1) == Tui::ZColor::defaultColor());
    }

    SECTION("peekBackground") {
        CHECK(image.peekBackground(0, 0) == Tui::ZColor::defaultColor());
        CHECK(image.peekBackground(-1, -1) == Tui::ZColor::defaultColor());
    }

    SECTION("peekDecoration") {
        CHECK(image.peekDecoration(0, 0) == Tui::ZColor::defaultColor());
        CHECK(image.peekDecoration(-1, -1) == Tui::ZColor::defaultColor());
    }

    SECTION("peekSoftwrapMarker") {
        CHECK(image.peekSoftwrapMarker(0, 0) == false);
        CHECK(image.peekSoftwrapMarker(-1, -1) == false);
    }

    SECTION("peekAttributes") {
        CHECK(image.peekAttributes(0, 0) == Tui::ZTextAttributes{});
        CHECK(image.peekAttributes(-1, -1) == Tui::ZTextAttributes{});
    }

    image.painter().writeWithAttributes(1, 1, "stuff", {0xFF, 0xFF, 0xFF}, {0x80, 0x80, 0x80}, Tui::ZTextAttribute::Bold);
    image.painter().setSoftwrapMarker(0, 1);

    SECTION("peekForground-with-stuff") {
        CHECK(image.peekForground(1, 1) == Tui::ZColor{0xFF, 0xFF, 0xFF});
    }

    SECTION("peekBackground-with-stuff") {
        CHECK(image.peekBackground(1, 1) == Tui::ZColor {0x80, 0x80, 0x80});
    }
    SECTION("peekDecoration-with-stuff") {
        CHECK(image.peekDecoration(1, 1) == Tui::ZColor::defaultColor());
    }

    SECTION("peekSoftwrapMarker-with-stuff") {
        CHECK(image.peekSoftwrapMarker(0, 1) == true);
    }

    SECTION("peekAttributes-with-stuff") {
        CHECK(image.peekAttributes(1, 1) == Tui::ZTextAttributes(Tui::ZTextAttribute::Bold));
    }

    SECTION("comparison") {
        Tui::ZImage image2(t.terminal.get(), 14, 4);
        image2.painter().writeWithAttributes(1, 1, "stuff", {0xFF, 0xFF, 0xFF}, {0x80, 0x80, 0x80}, Tui::ZTextAttribute::Bold);
        image2.painter().setSoftwrapMarker(0, 1);

        CHECK(image == image2);
        CHECK(!(image != image2));

        image2.painter().writeWithAttributes(1, 1, "STUFF", {0xFF, 0xFF, 0xFF}, {0x80, 0x80, 0x80}, Tui::ZTextAttribute::Bold);

        CHECK(image != image2);
        CHECK(!(image == image2));
    }

    SECTION("swap") {
        CHECK(image.size() == QSize{14, 4});
        Tui::ZImage image2(t.terminal.get(), 80, 11);
        CHECK(image2.size() == QSize{80, 11});
        image.swap(image2);
        CHECK(image.size() == QSize{80, 11});
        CHECK(image2.size() == QSize{14, 4});
        CHECK(image2.peekForground(1, 1) == Tui::ZColor{0xFF, 0xFF, 0xFF});
    }

    SECTION("copy-on-write") {
        CHECK(image.peekForground(1, 1) == Tui::ZColor{0xFF, 0xFF, 0xFF});

        Tui::ZImage image2 = image;

        CHECK(image.peekForground(1, 1) == Tui::ZColor{0xFF, 0xFF, 0xFF});
        CHECK(image2.peekForground(1, 1) == Tui::ZColor{0xFF, 0xFF, 0xFF});

        image2.painter().setForeground(1, 1, Tui::ZColor{0xFF, 0xAE, 0xFF});

        CHECK(image.peekForground(1, 1) == Tui::ZColor{0xFF, 0xFF, 0xFF});
        CHECK(image2.peekForground(1, 1) == Tui::ZColor{0xFF, 0xAE, 0xFF});
    }

    SECTION("move") {
        CHECK(image.peekForground(1, 1) == Tui::ZColor{0xFF, 0xFF, 0xFF});

        Tui::ZImage backup = image;
        Tui::ZImage image2 = std::move(image);

        CHECK(backup.peekForground(1, 1) == Tui::ZColor{0xFF, 0xFF, 0xFF});
        CHECK(image2.peekForground(1, 1) == Tui::ZColor{0xFF, 0xFF, 0xFF});

        image2.painter().setForeground(1, 1, Tui::ZColor{0xFF, 0xAE, 0xFF});

        CHECK(backup.peekForground(1, 1) == Tui::ZColor{0xFF, 0xFF, 0xFF});
        CHECK(image2.peekForground(1, 1) == Tui::ZColor{0xFF, 0xAE, 0xFF});

        image = backup;
        CHECK(image.peekForground(1, 1) == Tui::ZColor{0xFF, 0xFF, 0xFF});
    }

    SECTION("copy-ctor-with-painter") {
        CHECK(image.peekForground(1, 1) == Tui::ZColor{0xFF, 0xFF, 0xFF});

        Tui::ZPainter painter = image.painter();

        Tui::ZImage image2 = image;

        CHECK(image.peekForground(1, 1) == Tui::ZColor{0xFF, 0xFF, 0xFF});
        CHECK(image2.peekForground(1, 1) == Tui::ZColor{0xFF, 0xFF, 0xFF});

        painter.setForeground(1, 1, Tui::ZColor{0xFF, 0xAE, 0xFF});

        CHECK(image.peekForground(1, 1) == Tui::ZColor{0xFF, 0xAE, 0xFF});
        CHECK(image2.peekForground(1, 1) == Tui::ZColor{0xFF, 0xFF, 0xFF});
    }

    SECTION("copy-operator-with-painter") {
        CHECK(image.peekForground(1, 1) == Tui::ZColor{0xFF, 0xFF, 0xFF});

        Tui::ZPainter painter = image.painter();

        Tui::ZImage image2(t.terminal.get(), 80, 11);
        image2 = image;

        CHECK(image.peekForground(1, 1) == Tui::ZColor{0xFF, 0xFF, 0xFF});
        CHECK(image2.peekForground(1, 1) == Tui::ZColor{0xFF, 0xFF, 0xFF});

        painter.setForeground(1, 1, Tui::ZColor{0xFF, 0xAE, 0xFF});

        CHECK(image.peekForground(1, 1) == Tui::ZColor{0xFF, 0xAE, 0xFF});
        CHECK(image2.peekForground(1, 1) == Tui::ZColor{0xFF, 0xFF, 0xFF});
    }

}
