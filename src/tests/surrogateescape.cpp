#include <Tui/Misc/SurrogateEscape.h>

#include "catchwrapper.h"

#include <array>

#define U8(x) reinterpret_cast<const char*>(u8##x)

TEST_CASE("surrogateescape encode valid") {
    QByteArray res = Tui::Misc::SurrogateEscape::encode(QString::fromUtf16(u" test\u0100\uffff\u10ffff"));
    CHECK(res == QByteArray(U8(" test\u0100\uffff\u10ffff")));
}

TEST_CASE("surrogateescape decode valid") {
    QString res = Tui::Misc::SurrogateEscape::decode(QByteArray(U8(" test\u0100\uffff\u10ffff")));
    CHECK(res == QString::fromUtf16(u" test\u0100\uffff\u10ffff"));
}

TEST_CASE("surrogateescape encode surrogate escape") {
    std::array<char16_t, 4> in = {0xdc80, 0xdc89, 0xdca0, 0xdcff};
    QByteArray res = Tui::Misc::SurrogateEscape::encode(QString::fromUtf16(in.data(), in.size()));
    CHECK(res == QByteArray("\x80\x89\xa0\xff"));
}

TEST_CASE("surrogateescape encode surrogate escape and valid surrogate") {
    std::array<char16_t, 7> in = {0xdc80, 0xdc89, 0xdca0, 0xdcff, 0x50, 0xd800, 0xdc00};
    QByteArray res = Tui::Misc::SurrogateEscape::encode(QString::fromUtf16(in.data(), in.size()));
    CHECK(res == QByteArray("\x80\x89\xa0\xffP\U00010000"));
}

TEST_CASE("surrogateescape encode invalid isolated surrogate") {
    std::array<char16_t, 1> in = { 0xdc00 };
    QByteArray res = Tui::Misc::SurrogateEscape::encode(QString::fromUtf16(in.data(), in.size()));
    CHECK(res == QByteArray("\357\277\275"));
}

TEST_CASE("surrogateescape decode to surrogate escape") {
    std::array<char16_t, 4> expected = {0xdc80, 0xdc89, 0xdca0, 0xdcff};
    QString res = Tui::Misc::SurrogateEscape::decode(QByteArray("\x80\x89\xa0\xff"));
    CHECK(res == QString::fromUtf16(expected.data(), expected.size()));
}

TEST_CASE("surrogateescape decode truncated sequence 1") {
    std::array<char16_t, 2> expected = {0xdcf0, 0xdc90};
    QString res = Tui::Misc::SurrogateEscape::decode(QByteArray("\xF0\x90"));
    CHECK(res == QString::fromUtf16(expected.data(), expected.size()));
}

TEST_CASE("surrogateescape decode truncated sequence 2") {
    std::array<char16_t, 1> expected = {0xdcf0};
    QString res = Tui::Misc::SurrogateEscape::decode(QByteArray("\xF0"));
    CHECK(res == QString::fromUtf16(expected.data(), expected.size()));
}

TEST_CASE("surrogateescape decode low surrogate to surrogate escapes") {
    std::array<char16_t, 3> expected = {0xdced, 0xdcb0, 0xdc80};
    QString res = Tui::Misc::SurrogateEscape::decode(QByteArray("\xed\xb0\x80"));
    CHECK(res == QString::fromUtf16(expected.data(), expected.size()));
}

TEST_CASE("surrogateescape decode high surrogate to surrogate escape") {
    std::array<char16_t, 3> expected = {0xdced, 0xdca0, 0xdc80};
    QString res = Tui::Misc::SurrogateEscape::decode(QByteArray("\xed\xa0\x80"));
    CHECK(res == QString::fromUtf16(expected.data(), expected.size()));
}

TEST_CASE("surrogateescape decode mixed valid and invalid to surrogate escape and utf-16") {
    std::array<char16_t, 16> expected = {0xdcff, 0x00ff, 0xffff, 0xdbff, 0xdfff,
                                        0xdcf8, 0xdc80, 0xdc80, 0xdc80, 0xdc80, 0xdcfc, 0xdc80, 0xdc80, 0xdc80, 0xdc80, 0xdc80
                                       };
    QString res = Tui::Misc::SurrogateEscape::decode(QByteArray(U8("\xff\u00ff\uffff\U0010ffff\xf8\x80\x80\x80\x80\xfc\x80\x80\x80\x80\x80")));
    CHECK(res == QString::fromUtf16(expected.data(), expected.size()));
}
