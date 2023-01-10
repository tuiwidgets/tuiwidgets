// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZSimpleStringLogger.h>
#include <Tui/ZSimpleFileLogger.h>

#include <QTemporaryFile>

#include "catchwrapper.h"

#include "Testhelper.h"

TEST_CASE("logger-string", "") {
    Tui::ZSimpleStringLogger::install();

    qDebug("Testmessage");

    CHECK(Tui::ZSimpleStringLogger::getMessages() == "Testmessage\n");
    // Does not clear messages
    CHECK(Tui::ZSimpleStringLogger::getMessages() == "Testmessage\n");

    qDebug("Testmessage2");

    CHECK(Tui::ZSimpleStringLogger::getMessages() == "Testmessage\nTestmessage2\n");
    Tui::ZSimpleStringLogger::clearMessages();
    CHECK(Tui::ZSimpleStringLogger::getMessages() == "");

    qDebug("Testmessage3");
    CHECK(Tui::ZSimpleStringLogger::getMessages() == "Testmessage3\n");
}

TEST_CASE("logger-file", "") {
    QTemporaryFile tfile;
    tfile.open();
    tfile.close();
    QString fileName = tfile.fileName();

    auto getWithoutTimeStamps = [&] {
        QFile file(fileName);
        REQUIRE(file.open(QFile::ReadOnly));
        QString ret;
        for (QString line: QString::fromUtf8(file.readAll()).split("\n", QString::SkipEmptyParts)) {
            ret += line.mid(21) + "\n";
        }
        return ret;
    };

    Tui::ZSimpleFileLogger::install(fileName);

    qDebug("Testmessage");

    CHECK(getWithoutTimeStamps() == "Testmessage\n");

    qDebug("Testmessage2");

    CHECK(getWithoutTimeStamps() == "Testmessage\nTestmessage2\n");
}
