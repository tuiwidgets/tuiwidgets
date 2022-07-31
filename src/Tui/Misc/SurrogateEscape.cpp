#include "SurrogateEscape.h"

#include <QTextCodec>

TUIWIDGETS_NS_START

namespace Misc {

QString SurrogateEscape::decode(const QByteArray &data) {
    return decode(data.constData(), data.length());
}

QString SurrogateEscape::decode(const char *data, int len) {
    QTextCodec::ConverterState state(QTextCodec::IgnoreHeader);
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QString text = codec->toUnicode(data, len, &state);
    if (state.invalidChars > 0 || state.remainingChars > 0) {
        // contains invalid utf8, use slowpath for surrogate escape
        text = QStringLiteral("");
        for (int i = 0; i < len; i++) {
            int first_byte = (unsigned char)data[i];

            // one code point at a time

            int size;
            if (0xf0 == (0xf8 & first_byte)) {
                size = 4;
            } else if (0xe0 == (0xf0 & first_byte)) {
                size = 3;
            } else if (0xc0 == (0xe0 & first_byte)) {
                size = 2;
            } else {
                size = 1;
            }

            if (i + size > len) {
                text.append((QChar)(0xdc00 + (unsigned char)data[i]));
                continue;
            }

            QTextCodec::ConverterState state2(QTextCodec::IgnoreHeader);
            QString tmp = codec->toUnicode(data + i, size, &state2);
            if (state2.invalidChars) {
                text.append((QChar)(0xdc00 + (unsigned char)data[i]));
                continue;
            } else {
                text += tmp;
                i += size - 1;
            }
        }
    }
    return text;
}

QByteArray SurrogateEscape::encode(const QString &str) {
    return encode(str.constData(), str.length());
}

QByteArray SurrogateEscape::encode(const QChar *str, int len) {
    QByteArray data;
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::ConverterState state(QTextCodec::IgnoreHeader);
    data = codec->fromUnicode(str, len, &state);

    if (state.invalidChars || state.remainingChars) {
        // contains invalid utf16, use slowpath for surrogate escape and other invalid code
        data.resize(0);
        for (int i = 0; i < len; i++) {
            QChar ch = str[i];
            if (ch.isSurrogate()) {
                if (ch.isHighSurrogate() && i + 1 < len && str[i + 1].isLowSurrogate()) {
                    data += codec->fromUnicode(str + i, 2);
                    i++;
                } else {
                    if ((ch.unicode() & 0xff80) == 0xdc80) { // surrogate escape
                        data.append((char)ch.unicode() & 0xff);
                    } else { // invalid
                        // output is utf-8, so use U+FFFD REPLACEMENT CHARACTER for conversion error.
                        data.append("\357\277\275", 3);
                    }
                }
            } else {
                data += codec->fromUnicode(str + i, 1);
            }
        }
    }

    return data;

}
}

TUIWIDGETS_NS_END
