#include "ZStyledTextLine.h"
#include "ZStyledTextLine_p.h"

#include <Tui/MarkupParser.h>

#include <Tui/Utils_p.h>

TUIWIDGETS_NS_START

ZStyledTextLine::ZStyledTextLine() = default;

ZStyledTextLine::ZStyledTextLine(const ZStyledTextLine&) = default;
ZStyledTextLine::ZStyledTextLine(ZStyledTextLine&&) = default;
ZStyledTextLine::~ZStyledTextLine() = default;

ZStyledTextLine &ZStyledTextLine::operator=(const ZStyledTextLine&) = default;
ZStyledTextLine &ZStyledTextLine::operator=(ZStyledTextLine&&) = default;

void ZStyledTextLine::setMarkup(const QString &markup) {
    auto *const p = tuiwidgets_impl();
    p->markup = markup;
    p->text.clear();
    p->cached = false;
}

QString ZStyledTextLine::markup() const {
    auto *const p = tuiwidgets_impl();
    return p->markup;
}

void ZStyledTextLine::setText(const QString &text) {
    auto *const p = tuiwidgets_impl();
    p->markup.clear();
    p->text = text;
    p->cached = false;
}

QString ZStyledTextLine::text() const {
    auto *const p = tuiwidgets_impl();
    return p->text;
}

void ZStyledTextLine::setBaseStyle(ZTextStyle newBaseStyle) {
    auto *const p = tuiwidgets_impl();
    if (p->baseStyle != newBaseStyle || p->mnemonicStyle != newBaseStyle) {
        p->baseStyle = newBaseStyle;
        p->mnemonicStyle = std::move(newBaseStyle);
        p->cached = false;
    }
}

void ZStyledTextLine::setMnemonicStyle(ZTextStyle newBaseStyle, ZTextStyle newMnemonicStyle) {
    auto *const p = tuiwidgets_impl();
    if (p->baseStyle != newBaseStyle || p->mnemonicStyle != newMnemonicStyle) {
        p->baseStyle = std::move(newBaseStyle);
        p->mnemonicStyle = std::move(newMnemonicStyle);
        p->cached = false;
    }
}

QString ZStyledTextLine::mnemonic() const {
    auto *const p = tuiwidgets_impl();
    p->ensureCache();
    return p->mnemonic;
}

int ZStyledTextLine::width(const ZTextMetrics &metrics) const {
    auto *const p = tuiwidgets_impl();
    p->ensureCache();
    return metrics.sizeInColumns(p->textFromMarkup);
}

void ZStyledTextLine::write(ZPainter *painter, int x, int y, int width) const {
    auto *const p = tuiwidgets_impl();
    p->ensureCache();
    ZPainter localPainter = painter->translateAndClip(x, y, width, 1);
    ZTextMetrics textMetrics = painter->textMetrics();
    x = 0;
    localPainter.clear(p->baseStyle.foregroundColor(), p->baseStyle.backgroundColor(), p->baseStyle.attributes());
    int lastIdx = 0;
    for (int i = 0; i < p->styles.size(); i++) {
        if (lastIdx != p->styles[i].startIndex) {
            QString part = p->textFromMarkup.mid(lastIdx, p->styles[i].startIndex - lastIdx);
            localPainter.writeWithAttributes(x, 0,
                                             part,
                                             p->styles[i-1].style.foregroundColor(), p->styles[i-1].style.backgroundColor(),
                                             p->styles[i-1].style.attributes());
            x += textMetrics.sizeInColumns(part);
            lastIdx = p->styles[i].startIndex;
        }
    }
    if (lastIdx != p->textFromMarkup.size()) {
        localPainter.writeWithAttributes(x, 0,
                                         p->textFromMarkup.mid(lastIdx, p->textFromMarkup.size() - lastIdx),
                                         p->styles.last().style.foregroundColor(), p->styles.last().style.backgroundColor(),
                                         p->styles.last().style.attributes());
    }
}

void ZStyledTextLinePrivate::ensureCache() const {
    if (!cached) {
        cached = true;
        textFromMarkup.clear();
        styles.clear();
        mnemonic.clear();

        if (markup.size()) {
            QString maybeMnemonic;
            bool inMElement = false;
            QVector<ZTextStyle> stack;
            stack.append(baseStyle);

            using Private::MarkupParser;

            MarkupParser p(markup);

            bool done = false;
            bool error = false;
            while (!done && !error) {
                p.nextEvent();
                p.visit(overload(
                    [&](const MarkupParser::Error &) {
                        //qDebug() << "error";
                        error = true;
                    },
                    [&](const MarkupParser::ElementBegin &ev) {
                        //qDebug() << "begin" << ev.name() << ev.attributes();
                        stack.append(stack.last());
                        if (ev.name() == QStringLiteral("m")) {
                            if (inMElement) {
                                error = true;
                            }
                            inMElement = true;
                            stack.last() = mnemonicStyle;
                        }
                    },
                    [&](const MarkupParser::ElementEnd &ev) {
                        //qDebug() << "end" << ev.name();
                        stack.removeLast();
                        if (ev.name() == QStringLiteral("m")) {
                            inMElement = false;
                        }
                    },
                    [&](const MarkupParser::CharEvent &ev) {
                        //qDebug() << "char" << ev.asString();
                        if (styles.isEmpty()) {
                            styles.append({textFromMarkup.size(), stack.last()});
                        } else {
                            if (styles.last().style != stack.last()) {
                                styles.append({textFromMarkup.size(), stack.last()});
                            }
                        }
                        textFromMarkup.append(ev.asString());
                        if (inMElement) {
                            maybeMnemonic.append(ev.asString());
                        }
                    },
                    [&](const MarkupParser::DocumentEnd &) {
                        //qDebug() << "the end";
                        done = true;
                    }
                ));
            }
            if (error) {
                textFromMarkup.clear();
                styles.clear();
                textFromMarkup = QStringLiteral("Error parsing");
                styles.append({0, ZTextStyle({0xff, 0, 0}, {0, 0, 0})});
            } else {
                if (maybeMnemonic.size() == 1) {
                    mnemonic = maybeMnemonic;
                } else if (maybeMnemonic.size() == 2
                           && QChar::isHighSurrogate(maybeMnemonic[0].unicode())
                           && QChar::isLowSurrogate(maybeMnemonic[1].unicode())) {
                    mnemonic = maybeMnemonic;
                }
            }
        } else {
            styles.append({0, baseStyle});
            textFromMarkup = text;
        }
    }
}


TUIWIDGETS_NS_END
