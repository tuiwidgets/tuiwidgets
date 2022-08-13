// SPDX-License-Identifier: BSL-1.0

#include "ZFormatRange.h"
#include "ZFormatRange_p.h"

TUIWIDGETS_NS_START

ZFormatRangePrivate::ZFormatRangePrivate() {
}

ZFormatRangePrivate::ZFormatRangePrivate(int start, int length, ZTextStyle format, ZTextStyle formatingChar, int userData)
    : _start(start), _length(length), _format(format), _formattingChar(formatingChar), _userData(userData)
{
}

ZFormatRangePrivate::~ZFormatRangePrivate() {
}


ZFormatRange::ZFormatRange() = default;

ZFormatRange::ZFormatRange(int start, int length, ZTextStyle format, ZTextStyle formattingChar, int userData)
    : tuiwidgets_pimpl_ptr(ZFormatRangePrivate(start, length, format, formattingChar, userData))
{
}

ZFormatRange::ZFormatRange(const ZFormatRange&) = default;

ZFormatRange::ZFormatRange(ZFormatRange&&) = default;

ZFormatRange::~ZFormatRange() = default;

ZFormatRange &ZFormatRange::operator=(const ZFormatRange&) = default;

ZFormatRange &ZFormatRange::operator=(ZFormatRange&&) = default;

int ZFormatRange::start() const {
    auto *const p = tuiwidgets_impl();
    return p->_start;
}

void ZFormatRange::setStart(int start) {
    auto *const p = tuiwidgets_impl();
    p->_start = start;
}

int ZFormatRange::length() const {
    auto *const p = tuiwidgets_impl();
    return p->_length;
}

void ZFormatRange::setLength(int length) {
    auto *const p = tuiwidgets_impl();
    p->_length = length;
}

ZTextStyle ZFormatRange::format() const {
    auto *const p = tuiwidgets_impl();
    return p->_format;
}

void ZFormatRange::setFormat(const ZTextStyle &format) {
    auto *const p = tuiwidgets_impl();
    p->_format = format;
}

ZTextStyle ZFormatRange::formattingChar() const {
    auto *const p = tuiwidgets_impl();
    return p->_formattingChar;
}

void ZFormatRange::setFormattingChar(const ZTextStyle &formattingChar) {
    auto *const p = tuiwidgets_impl();
    p->_formattingChar = formattingChar;
}

int ZFormatRange::userData() const {
    auto *const p = tuiwidgets_impl();
    return p->_userData;
}

void ZFormatRange::setUserData(int userData) {
    auto *const p = tuiwidgets_impl();
    p->_userData = userData;
}

TUIWIDGETS_NS_END
