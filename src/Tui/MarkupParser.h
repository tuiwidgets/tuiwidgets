// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_PRIVATE_MARKUPPARSER_INCLUDED
#define TUIWIDGETS_PRIVATE_MARKUPPARSER_INCLUDED

#include <map>
#include <queue>
#include <vector>

#include <QList>
#include <QMap>
#include <QString>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

namespace Private {

class TUIWIDGETS_EXPORT MarkupTokenizer {
public:
    MarkupTokenizer(QString markup);

public:
    void getToken();
    void ackSelfClosingTag();
    void errorUnimplemented(const char *msg);

public:
    enum TokenType {
        TT_Invalid,
        TT_Char,
        TT_Comment,
        TT_Doctype,
        TT_TagBegin,
        TT_TagEnd,
        TT_EOF,
    };

    bool _isError = false;

    TokenType currentToken = TT_Invalid;
    char32_t currentChar;
    std::u32string tagName;
    std::map<std::u32string, std::u32string> tagAttributes;

    enum TokMode {
        T_Data,
        T_TagOpen,
        T_TagEnd,
        T_TagName,
        T_TagSelfClose,
        T_BeforeAttrName,
        T_AttrName,
        T_AfterAttrName,
        T_BeforeAttrValue,
        T_AttrValueDblQuoted,
        T_AttrValueUnquoted,
        T_AttrValueSingleQuoted,
        T_AfterAttrValueQuoted,
    };
    TokMode tokMode = T_Data;

private:
    bool isAllowedCodepoint(char32_t ch);
    void storeAttribute();
    std::u32string tryTokenizeCharRef(char32_t additionalAllowedChar);

private:
    std::queue<char32_t> pendingChars;
    bool selfClosingMustAck = false;

    // Valid when tokMode in [T_TagName, T_TagSelfClose, T_BeforeAttrName, T_AttrName, T_AfterAttrName,
    // T_BeforeAttrValue, T_AttrValueDblQuoted, T_AttrValueUnquoted, T_AttrValueSingleQuoted, T_AfterAttrValueQuoted]
    bool isOpen;
    // Valid when tokMode in [T_AttrName, T_AfterAttrName, T_BeforeAttrValue, T_AttrValueDblQuoted, T_AttrValueUnquoted,
    // T_AttrValueSingleQuoted]
    std::u32string attrName;
    // Valid when tokMode in [T_AttrValueDblQuoted, T_AttrValueUnquoted, T_AttrValueSingleQuoted]
    std::u32string attrValue;

    uint nextInputChar = 0;
    std::u32string text;
};


class TUIWIDGETS_EXPORT MarkupParserBase {
public:
    MarkupParserBase(QString markup);

    class Error {
    public:
        Error(const Error&) = delete;

    private:
        friend class MarkupParserBase;
        Error(const MarkupParserBase &p);
        const MarkupParserBase &p;
    };

    class ElementBegin {
    public:
        ElementBegin(const ElementBegin&) = delete;
        QString name() const;
        QMap<QString, QString> attributes() const;

    private:
        friend class MarkupParserBase;
        ElementBegin(const MarkupParserBase &p);
        const MarkupParserBase &p;
    };

    class ElementEnd {
    public:
        ElementEnd(const ElementEnd&) = delete;
        QString name() const;

    private:
        friend class MarkupParserBase;
        ElementEnd(const MarkupParserBase &p);
        const MarkupParserBase &p;
    };

    class CharEvent {
    public:
        CharEvent(const CharEvent&) = delete;
        char32_t asChar32_t() const;
        QString asString() const;

    private:
        friend class MarkupParserBase;
        CharEvent(const MarkupParserBase &p);
        const MarkupParserBase &p;
    };

    class DocumentEnd {
        DocumentEnd(const DocumentEnd&) = delete;

    private:
        friend class MarkupParserBase;
        DocumentEnd(const MarkupParserBase &p);
        const MarkupParserBase &p;
    };


    bool isError();
    const Error &error();
    bool isElementBegin();
    const ElementBegin &elementBegin();
    bool isElementEnd();
    const ElementEnd &elementEnd();
    bool isCharEvent();
    const CharEvent &charEvent();
    bool isDocumentEnd();
    const DocumentEnd &documentEnd();

    template<typename Visitor>
    auto visit(Visitor&& vis) {
        if (isElementBegin()) {
            return vis(elementBegin());
        } else if (isElementEnd()) {
            return vis(elementEnd());
        } else if (isCharEvent()) {
            return vis(charEvent());
        } else if (isDocumentEnd()) {
            return vis(documentEnd());
        } else {
            return vis(error());
        }
    }

protected:
    void emitCurrentTokenizerElement();
    void emitSyntheticElement(std::u32string name);

    bool currentTokenIsChar();
    bool currentTokenIsComment();
    bool currentTokenIsDoctype();
    bool currentTokenIsTagBegin();
    bool currentTokenIsTagEnd();
    bool currentTokenIsEOF();

protected:
    enum Event {
        E_Invalid,
        E_ElementBegin,
        E_Char,
        E_DocumentEnd,
    };

    Event eventType = E_Invalid; // lower priority than hasEndTag
    bool hasEndTag = false;
    QString eventTagStart;
    QString eventTagEnd;
    QMap<QString, QString> eventAttributes;
    char32_t eventChar;
    QList<std::u32string> pendingElementEndEvents;

    QList<std::u32string> openElements;
    bool reprocessToken = false;
    bool discardOneLf = false;

    MarkupTokenizer t;

    const Error _error;
    const ElementBegin _elementBegin;
    const ElementEnd _elementEnd;
    const CharEvent _charEvent;
    const DocumentEnd _documentEnd;
};

class TUIWIDGETS_EXPORT MarkupParser : public MarkupParserBase {
public:
    MarkupParser(QString markup);

public:
    void nextEvent();

private:
    void miniAAA(std::u32string element);
    void closePElement();
    void generateImpliedEndTags(std::u32string excluded);
    bool inSpecificScope(std::u32string element, QSet<std::u32string> set);
    bool hasInButtonScope(std::u32string element);
    bool hasInScope(std::u32string element);
    bool isSpecial(std::u32string element);

private:
    enum InsertionMode {
        IM_Initial,
        IM_BeforeHtml,
        IM_BeforeHead,
        IM_InHead,
        IM_AfterHead,
        IM_InBody,
        IM_AfterBody,
        IM_AfterAfterBody,
    };
    InsertionMode insertionMode = IM_Initial;
};

}
TUIWIDGETS_NS_END

#endif // TUIWIDGETS_PRIVATE_MARKUPPARSER_INCLUDED
