#include "MarkupParser.h"

#include <stdlib.h>

#include <functional>
#include <unordered_map>

// This needs to be *above* the QSet include:
static uint qHash(const std::u32string &key, uint seed) noexcept {
    Q_UNUSED(seed);
    return static_cast<uint>(std::hash<std::u32string>{}(key));
}

#include <QSet>
#include <QDebug>

TUIWIDGETS_NS_START

namespace Private {

MarkupTokenizer::MarkupTokenizer(QString markup)
{
    text.reserve(markup.size());
    bool cr = false;
    for (auto it = markup.constBegin(); it != markup.constEnd(); it++) {
        char32_t ch = it->unicode();
        if (cr) {
            text.push_back(0xa);
            if (ch == 0xa) {
                continue;
            }
            cr = false;
        }
        if (it->isHighSurrogate()) {
            _isError = true;
            break;
        } else if (it->isLowSurrogate()) {
            it++;
            if (it != markup.constEnd() && it->isHighSurrogate()) {
                text.push_back(static_cast<char32_t>(QChar::surrogateToUcs4(it->unicode(), ch)));
            } else {
                _isError = true;
                break;
            }
        }
        if (!isAllowedCodepoint(ch)) {
            _isError = true;
            break;
        }
        if (ch == 0xd) {
            cr = true;
        } else {
            text.push_back(ch);
        }
    }
    if (cr && !_isError) {
        text.push_back(0xa);
    }
}

bool MarkupTokenizer::isAllowedCodepoint(char32_t ch)
{
    if ((ch >= 1 && ch <= 8)
        || (ch >= 0xe && ch <= 0x1f)
        || (ch >= 0x7f && ch <= 0x9f)
        || (ch >= 0xfdd0 && ch <= 0xfdef)) {
        return false;
    }
    switch (ch) {
        case 0xB: case 0xFFFE: case 0xFFFF: case 0x1FFFE: case 0x1FFFF: case 0x2FFFE: case 0x2FFFF: case 0x3FFFE:
        case 0x3FFFF: case 0x4FFFE: case 0x4FFFF: case 0x5FFFE: case 0x5FFFF: case 0x6FFFE: case 0x6FFFF:
        case 0x7FFFE: case 0x7FFFF: case 0x8FFFE: case 0x8FFFF: case 0x9FFFE: case 0x9FFFF: case 0xAFFFE:
        case 0xAFFFF: case 0xBFFFE: case 0xBFFFF: case 0xCFFFE: case 0xCFFFF: case 0xDFFFE: case 0xDFFFF:
        case 0xEFFFE: case 0xEFFFF: case 0xFFFFE: case 0xFFFFF: case 0x10FFFE: case 0x10FFFF:
            return false;
    }
    return true;
}

void MarkupTokenizer::storeAttribute()
{
    tagAttributes[attrName] = attrValue;
    attrName.clear();
    attrValue.clear();
}

static uint32_t hash_fnv1a(const unsigned char* text) {
    uint32_t hash = 2166136261;
    for (; *text; ++text) {
            hash = hash ^ *text;
            hash = hash * 16777619;
    }
    return hash;
}

namespace {
    struct EntitiesEntry {
        uint8_t len;
        uint8_t has_next;
        uint16_t strpool_offset;
        char32_t data[2];
    };

    #include "entities.inc"
}

std::u32string MarkupTokenizer::tryTokenizeCharRef(char32_t additionalAllowedChar) {
    if (nextInputChar >= text.size()) {
        return {};
    }
    char32_t ch1 = text[nextInputChar];
    if (ch1 == 9 || ch1 == 0xa || ch1 == 0xc || ch1 == ' ' || ch1 == '<' || ch1 == '&'
            || (additionalAllowedChar && ch1 == additionalAllowedChar)) {
        return {};
    } else if (ch1 == '#') {
        unsigned int peekChar = nextInputChar + 1;
        bool hex = false;
        if (peekChar < text.size()) {
            char32_t ch = text[peekChar];
            if (ch == 'x' || ch == 'X') {
                hex = true;
                ++peekChar;
            }
        }
        char32_t chEncoded = 0;
        while (peekChar < text.size()) {
            char32_t ch = text[peekChar];
            if (ch >= '0' && ch <= '9') {
                chEncoded *= hex ? 16 : 10;
                chEncoded += ch - '0';
            } else if (hex && ch >= 'a' && ch <= 'f') {
                chEncoded *= hex ? 16 : 10;
                chEncoded += ch - 'a' + 10;
            } else if (hex && ch >= 'A' && ch <= 'F') {
                chEncoded *= hex ? 16 : 10;
                chEncoded += ch - 'A' + 10;
            } else if (ch == ';') {
                break;
            } else {
                _isError = true;
                return {};
            }
            ++peekChar;
        }
        if (peekChar < text.size()) {
            if (!isAllowedCodepoint(chEncoded) || chEncoded == 0 || chEncoded == 0xd
                || (chEncoded >= 0xD800 && chEncoded <= 0xDFFF) || chEncoded > 0x10FFFF) {
                _isError = true;
                return {};
            }
            nextInputChar = peekChar + 1;
            std::u32string ret;
            ret.push_back(chEncoded);
            return ret;
        } else {
            _isError = true;
            return {};
        }
    } else {
        // Note: Needs to be fixed for spec compliance.
        unsigned int peekChar = nextInputChar;
        std::string entityName;
        while (peekChar < text.size()) {
            char32_t ch = text[peekChar];
            if ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
                entityName.push_back(ch);
                peekChar++;
            } else if (ch == ';') {
                const int hash = hash_fnv1a(reinterpret_cast<const unsigned char*>(entityName.c_str())) % entitiesSize;
                int entryIdx = entitiesHashtbl[hash];
                while (true) {
                    const EntitiesEntry& entry = entitiesEntries[entryIdx];
                    if (entry.len == entityName.size()) {
                        if (memcmp(entityName.c_str(), entitiesStringpool + entry.strpool_offset, entry.len) == 0) {
                            // match found
                            nextInputChar = peekChar + 1;
                            std::u32string ret;
                            if (entry.data[1] == 0) {
                                // one code point
                                return std::u32string(entry.data, 1);
                            } else {
                                return std::u32string(entry.data, 2);
                            }
                            return ret;
                        }
                    }
                    if (!entry.has_next) {
                        _isError = true;
                        return {};
                    }
                    entryIdx++;
                }
            } else {
                return {};
            }
        }
        _isError = true;
        return {};
    }
}

void MarkupTokenizer::ackSelfClosingTag() {
    selfClosingMustAck = false;
}

void MarkupTokenizer::errorUnimplemented(const char *msg) {
    qDebug() << "MarkupParser: Not yet implemented: " << msg;
    _isError = true;
}

void MarkupTokenizer::getToken() {
    if (selfClosingMustAck) {
        _isError = true;
        return;
    }
    if (pendingChars.size()) {
        currentToken = TT_Char;
        currentChar = pendingChars.front();
        pendingChars.pop();
        return;
    }
    if (nextInputChar >= text.size() && currentToken != TT_EOF) {
        currentToken = TT_EOF;
        return;
    }
    currentToken = TT_Invalid;
    while (currentToken == TT_Invalid) {
        if (_isError) {
            return;
        }
        if (nextInputChar >= text.size()) {
            _isError = true;
            return;
        }
        char32_t ch = text[nextInputChar];
        ++nextInputChar;
        switch (tokMode) {
            case T_Data: {
                if (ch == '&') {
                    std::u32string tmp = tryTokenizeCharRef(0);
                    if (tmp.size()) {
                        currentToken = TT_Char;
                        currentChar = tmp[0];
                        for (unsigned i = 1; i < tmp.size(); i++) {
                            pendingChars.push(tmp[i]);
                        }
                    } else {
                        currentToken = TT_Char;
                        currentChar = ch;
                    }
                } else if (ch == '<') {
                    tagAttributes.clear();
                    tagName.clear();
                    tokMode = T_TagOpen;
                } else if (ch == 0) {
                    _isError = true;
                } else {
                    currentToken = TT_Char;
                    currentChar = ch;
                }
                } break;
            case T_TagOpen: {
                if (ch == '!') {
                    errorUnimplemented("doctype parsing");
                    // FIXME Switch to the markup declaration open state.
                } else if (ch == '/') {
                    tokMode = T_TagEnd;
                } else if (ch >= 'A' && ch <= 'Z') {
                    isOpen = true;
                    tagName.push_back(ch + 0x20);
                    tokMode = T_TagName;
                } else if (ch >= 'a' && ch <= 'z') {
                    isOpen = true;
                    tagName.push_back(ch);
                    tokMode = T_TagName;
                } else {
                    _isError = true;
                }
                } break;
            case T_TagEnd: {
                if (ch >= 'A' && ch <= 'Z') {
                    isOpen = false;
                    tagName.push_back(ch + 0x20);
                    tokMode = T_TagName;
                } else if (ch >= 'a' && ch <= 'z') {
                    isOpen = false;
                    tagName.push_back(ch);
                    tokMode = T_TagName;
                } else {
                    _isError = true;
                }
                } break;
            case T_TagName: {
                if (ch == 9 || ch == 0xa || ch == 0xc || ch == ' ') {
                    tokMode = T_BeforeAttrName;
                } else if (ch == '/') {
                    tokMode = T_TagSelfClose;
                } else if (ch == '>') {
                    tokMode = T_Data;
                    currentToken = isOpen ? TT_TagBegin : TT_TagEnd;
                } else if (ch >= 'A' && ch <= 'Z') {
                    tagName.push_back(ch + 0x20);
                } else if (ch == 0) {
                    _isError = true;
                } else {
                    tagName.push_back(ch);
                }
                } break;
            case T_BeforeAttrName: {
                if (ch == 9 || ch == 0xa || ch == 0xc || ch == ' ') {
                    ; // ignore
                } else if (ch == '/') {
                    tokMode = T_TagSelfClose;
                } else if (ch == '>') {
                    tokMode = T_Data;
                    if (!isOpen && tagAttributes.size()) {
                        _isError = true;
                    } else {
                        currentToken = isOpen ? TT_TagBegin : TT_TagEnd;
                    }
                } else if (ch >= 'A' && ch <= 'Z') {
                    tokMode = T_AttrName;
                    attrName.push_back(ch + 0x20);
                } else if (ch == 0) {
                    _isError = true;
                } else if (ch == '"' || ch == '\'' || ch == '<' || ch == '=') {
                    _isError = true;
                } else {
                    tokMode = T_AttrName;
                    attrName.push_back(ch);
                }
                } break;
            case T_AttrName: {
                if (ch == 9 || ch == 0xa || ch == 0xc || ch == ' ') {
                    if (tagAttributes.count(attrName)) {
                        _isError = true;
                    } else {
                        tokMode = T_AfterAttrName;
                    }
                } else if (ch == '/') {
                    if (tagAttributes.count(attrName)) {
                        _isError = true;
                    } else {
                        storeAttribute();
                        tokMode = T_TagSelfClose;
                    }
                } else if (ch == '=') {
                    if (tagAttributes.count(attrName)) {
                        _isError = true;
                    } else {
                        tokMode = T_BeforeAttrValue;
                    }
                } else if (ch == '>') {
                    if (tagAttributes.count(attrName)) {
                        _isError = true;
                    } else {
                        storeAttribute();
                        tokMode = T_Data;
                        if (!isOpen && tagAttributes.size()) {
                            _isError = true;
                        } else {
                            currentToken = isOpen ? TT_TagBegin : TT_TagEnd;
                        }
                    }
                } else if (ch >= 'A' && ch <= 'Z') {
                    attrName.push_back(ch + 0x20);
                } else if (ch == 0) {
                    _isError = true;
                } else if (ch == '"' || ch == '\'' || ch == '<') {
                    _isError = true;
                } else {
                    attrName.push_back(ch);
                }
                } break;
            case T_AfterAttrName: {
                if (ch == 9 || ch == 0xa || ch == 0xc || ch == ' ') {
                    ; // ignore
                } else if (ch == '/') {
                    storeAttribute();
                    tokMode = T_TagSelfClose;
                } else if (ch == '=') {
                    tokMode = T_BeforeAttrValue;
                } else if (ch == '>') {
                    storeAttribute();
                    tokMode = T_Data;
                    if (!isOpen && tagAttributes.size()) {
                        _isError = true;
                    } else {
                        currentToken = isOpen ? TT_TagBegin : TT_TagEnd;
                    }
                } else if (ch >= 'A' && ch <= 'Z') {
                    storeAttribute();
                    tokMode = T_AttrName;
                    attrName.push_back(ch + 0x20);
                } else if (ch == 0) {
                    _isError = true;
                } else if (ch == '"' || ch == '\'' || ch == '<') {
                    _isError = true;
                } else {
                    storeAttribute();
                    tokMode = T_AttrName;
                    attrName.push_back(ch);
                }
                } break;
            case T_BeforeAttrValue: {
                if (ch == 9 || ch == 0xa || ch == 0xc || ch == ' ') {
                    ; // ignore
                } else if (ch == '"') {
                    tokMode = T_AttrValueDblQuoted;
                } else if (ch == '&') {
                    tokMode = T_AttrValueUnquoted;
                    --nextInputChar;
                } else if (ch == '\'') {
                    tokMode = T_AttrValueSingleQuoted;
                } else if (ch == 0) {
                    _isError = true;
                } else if (ch == '>') {
                    _isError = true;
                } else if (ch == '<' || ch == '=' || ch == '`') {
                    _isError = true;
                } else {
                    attrValue.push_back(ch);
                    tokMode = T_AttrValueUnquoted;
                }
                } break;
            case T_AttrValueDblQuoted: {
                if (ch == '"') {
                    storeAttribute();
                    tokMode = T_AfterAttrValueQuoted;
                } else if (ch == '&') {
                    std::u32string tmp = tryTokenizeCharRef('"');
                    if (tmp.size()) {
                        attrValue.append(tmp);
                    } else if (!_isError) {
                        attrValue.push_back(ch);
                    }
                } else if (ch == 0) {
                    _isError = true;
                } else {
                    attrValue.push_back(ch);
                }
                } break;
            case T_AttrValueSingleQuoted: {
                if (ch == '\'') {
                    storeAttribute();
                    tokMode = T_AfterAttrValueQuoted;
                } else if (ch == '&') {
                    std::u32string tmp = tryTokenizeCharRef('\'');
                    if (tmp.size()) {
                        attrValue.append(tmp);
                    } else if (!_isError) {
                        attrValue.push_back(ch);
                    }
                } else if (ch == 0) {
                    _isError = true;
                } else {
                    attrValue.push_back(ch);
                }
                } break;
            case T_AttrValueUnquoted: {
                if (ch == 9 || ch == 0xa || ch == 0xc || ch == ' ') {
                    storeAttribute();
                    tokMode = T_BeforeAttrName;
                } else if (ch == '&') {
                    std::u32string tmp = tryTokenizeCharRef('>');
                    if (tmp.size()) {
                        attrValue.append(tmp);
                    } else if (!_isError) {
                        attrValue.push_back(ch);
                    }
                } else if (ch == '>') {
                    storeAttribute();
                    tokMode = T_Data;
                    currentToken = isOpen ? TT_TagBegin : TT_TagEnd;
                } else if (ch == 0) {
                    _isError = true;
                } else if (ch == '"' || ch == '\'' || ch == '<' || ch == '=' || ch == '`') {
                    _isError = true;
                } else {
                    attrValue.push_back(ch);
                }
                } break;
            case T_AfterAttrValueQuoted: {
                if (ch == 9 || ch == 0xa || ch == 0xc || ch == ' ') {
                    tokMode = T_BeforeAttrName;
                } else if (ch == '/') {
                    tokMode = T_TagSelfClose;
                } else if (ch == '>') {
                    tokMode = T_Data;
                    if (!isOpen && tagAttributes.size()) {
                        _isError = true;
                    } else {
                        currentToken = isOpen ? TT_TagBegin : TT_TagEnd;
                    }
                } else {
                    _isError = true;
                }
                } break;
            case T_TagSelfClose: {
                if (ch == '>') {
                    tokMode = T_Data;
                    if (!isOpen) {
                        _isError = true;
                    } else {
                        currentToken = isOpen ? TT_TagBegin : TT_TagEnd;
                    }
                    selfClosingMustAck = true;
                } else {
                    _isError = true;
                }
                } break;
        }
    }
}


MarkupParser::Error::Error(const MarkupParserBase &p) : p(p) {
}

QString MarkupParser::ElementBegin::name() const {
    return p.eventTagStart;
}

QMap<QString, QString> MarkupParser::ElementBegin::attributes() const {
    return p.eventAttributes;
}

MarkupParser::ElementBegin::ElementBegin(const MarkupParserBase &p) : p(p) {
}

QString MarkupParser::ElementEnd::name() const {
    return p.eventTagEnd;
}

MarkupParser::ElementEnd::ElementEnd(const MarkupParserBase &p) : p(p) {
}

char32_t MarkupParser::CharEvent::asChar32_t() const {
    return p.eventChar;
}

QString MarkupParser::CharEvent::asString() const {
    return QString::fromUcs4(&p.eventChar, 1);
}

MarkupParser::CharEvent::CharEvent(const MarkupParserBase &p) : p(p) {
}

MarkupParser::DocumentEnd::DocumentEnd(const MarkupParserBase &p) : p(p) {
}


MarkupParserBase::MarkupParserBase(QString markup) : t(markup),
    _error(*this), _elementBegin(*this), _elementEnd(*this), _charEvent(*this), _documentEnd(*this)
{
}

bool MarkupParserBase::isError() {
    return t._isError || eventType == E_Invalid;
}

const MarkupParserBase::Error &MarkupParserBase::error() {
    return _error;
}

bool MarkupParserBase::isElementBegin() {
    return !t._isError && !hasEndTag && eventType == E_ElementBegin;
}

const MarkupParserBase::ElementBegin &MarkupParserBase::elementBegin() {
    if (isElementBegin()) {
        return _elementBegin;
    }
    // Precondition violation, cause explicit crash
    return *static_cast<MarkupParser::ElementBegin*>(nullptr);
}

bool MarkupParserBase::isElementEnd() {
    return !t._isError && hasEndTag;
}

const MarkupParserBase::ElementEnd &MarkupParserBase::elementEnd() {
    if (isElementEnd()) {
        return _elementEnd;
    }
    // Precondition violation, cause explicit crash
    abort();
}

bool MarkupParserBase::isCharEvent() {
    return !t._isError && !hasEndTag && eventType == E_Char;
}

const MarkupParserBase::CharEvent &MarkupParserBase::charEvent() {
    if (isCharEvent()) {
        return _charEvent;
    }
    // Precondition violation, cause explicit crash
    abort();
}

bool MarkupParserBase::isDocumentEnd() {
    return !t._isError && !hasEndTag && eventType == E_DocumentEnd;
}

const MarkupParserBase::DocumentEnd &MarkupParserBase::documentEnd() {
    if (isDocumentEnd()) {
        return _documentEnd;
    }
    // Precondition violation, cause explicit crash
    abort();
}

void MarkupParserBase::emitCurrentTokenizerElement() {
    eventType = E_ElementBegin;
    eventTagStart = QString::fromStdU32String(t.tagName);
    eventAttributes.clear();
    for (auto nameAndValue : t.tagAttributes) {
        eventAttributes[QString::fromStdU32String(nameAndValue.first)] = QString::fromStdU32String(nameAndValue.second);
    }
}

void MarkupParserBase::emitSyntheticElement(std::u32string name) {
    eventType = E_ElementBegin;
    eventTagStart = QString::fromStdU32String(name);
    eventAttributes.clear();
}

bool MarkupParserBase::currentTokenIsChar() {
    return t.currentToken == t.TT_Char;
}

bool MarkupParserBase::currentTokenIsComment() {
    return t.currentToken == t.TT_Comment;
}

bool MarkupParserBase::currentTokenIsDoctype() {
    return t.currentToken == t.TT_Doctype;
}

bool MarkupParserBase::currentTokenIsTagBegin() {
    return t.currentToken == t.TT_TagBegin;
}

bool MarkupParserBase::currentTokenIsTagEnd() {
    return t.currentToken == t.TT_TagEnd;
}

bool MarkupParserBase::currentTokenIsEOF() {
    return t.currentToken == t.TT_EOF;
}

MarkupParser::MarkupParser(QString markup)
    : MarkupParserBase(markup)
{
}

void MarkupParser::nextEvent() {
    if (t._isError) {
        return;
    }

    // emit queued events
    if (pendingElementEndEvents.size()) {
        hasEndTag = true;
        eventTagEnd = QString::fromStdU32String(pendingElementEndEvents.first());
        pendingElementEndEvents.removeFirst();
        return;
    }
    if (hasEndTag) {
        hasEndTag = false;
        if (eventType != E_Invalid) {
            // event was delayed by endtag events.
            return;
        }
    } else {
        eventType = E_Invalid;
    }

    while (eventType == E_Invalid) {

        if (!reprocessToken) {
            t.getToken();
            if (t._isError) {
                return;
            }
            if (discardOneLf && currentTokenIsChar() && t.currentChar == 0xa) {
                t.getToken();
                if (t._isError) {
                    return;
                }
            }
            discardOneLf = false;
        } else {
            reprocessToken = false;
        }
        switch (insertionMode) {
            case IM_Initial: {
                if (currentTokenIsChar() &&
                   (t.currentChar == 9 || t.currentChar == 0xa || t.currentChar == 0xc || t.currentChar == 0xd || t.currentChar == ' ')) {
                    ; // ignore
                } else if (currentTokenIsComment()) {
                    ; // ignore (we are not maintaining comments)
                } else if (currentTokenIsDoctype()) {
                    ; // ignore (we are not maintaining doctype)
                    insertionMode = IM_BeforeHtml;
                } else {
                    // This would be an error really (unless iframe srcdoc mode),
                    // but we ignore it for convenience in short snippets
                    insertionMode = IM_BeforeHtml;
                    reprocessToken = true;
                }
                } break;
            case IM_BeforeHtml: {
                if (currentTokenIsDoctype()) {
                    t._isError = true;
                } else if (currentTokenIsComment()) {
                    ; // ignore (we are not maintaining comments)
                } else if (currentTokenIsChar() &&
                       (t.currentChar == 9 || t.currentChar == 0xa || t.currentChar == 0xc || t.currentChar == 0xd || t.currentChar == ' ')) {
                    ; // ignore
                } else if (currentTokenIsTagBegin() && t.tagName == U"html") {
                    emitCurrentTokenizerElement();
                    openElements.append(t.tagName);
                    // we ignore the manifest / cache selection parts
                    insertionMode = IM_BeforeHead;
                } else if (currentTokenIsTagEnd()) {
                    if(t.tagName == U"head" || t.tagName == U"body" || t.tagName == U"html" || t.tagName == U"br") {
                        emitSyntheticElement(U"html");
                        openElements.append(U"html");
                        // we ignore the manifest / cache selection parts
                        insertionMode = IM_BeforeHead;
                    } else {
                        t._isError = true;
                    }
                } else {
                    emitSyntheticElement(U"html");
                    openElements.append(U"html");
                    // we ignore the manifest / cache selection parts
                    insertionMode = IM_BeforeHead;
                    reprocessToken = true;
                }
                } break;
            case IM_BeforeHead: {
                if (currentTokenIsChar() &&
                   (t.currentChar == 9 || t.currentChar == 0xa || t.currentChar == 0xc || t.currentChar == 0xd || t.currentChar == ' ')) {
                    ; // ignore
                } else if (currentTokenIsComment()) {
                    ; // ignore (we are not maintaining comments)
                } else if (currentTokenIsDoctype()) {
                    t._isError = true;
                } else if (currentTokenIsTagBegin() && t.tagName == U"html") {
                    t._isError = true;
                } else if (currentTokenIsTagBegin() && t.tagName == U"head") {
                    emitCurrentTokenizerElement();
                    openElements.append(t.tagName);
                    // ??? Set the head element pointer to the newly created head element.
                    insertionMode = IM_InHead;
                } else if (currentTokenIsTagEnd()) {
                    if(t.tagName == U"head" || t.tagName == U"body" || t.tagName == U"html" || t.tagName == U"br") {
                        emitSyntheticElement(U"head");
                        openElements.append(U"head");
                        insertionMode = IM_InHead;
                        reprocessToken = true;
                    } else {
                        t._isError = true;
                    }
                } else {
                    emitSyntheticElement(U"head");
                    openElements.append(U"head");
                    // ??? Set the head element pointer to the newly created head element.
                    insertionMode = IM_InHead;
                    reprocessToken = true;
                }
                } break;
            case IM_InHead: {
                if (currentTokenIsChar() &&
                   (t.currentChar == 9 || t.currentChar == 0xa || t.currentChar == 0xc || t.currentChar == 0xd || t.currentChar == ' ')) {
                    eventType = E_Char;
                    eventChar = t.currentChar;
                } else if (currentTokenIsComment()) {
                    ; // ignore (we are not maintaining comments)
                } else if (currentTokenIsDoctype()) {
                    t._isError = true;
                } else if (currentTokenIsTagBegin() && t.tagName == U"html") {
                    t._isError = true;
                } else if (currentTokenIsTagBegin()
                           && (t.tagName == U"base" || t.tagName == U"basefont"
                                || t.tagName == U"bgsound" || t.tagName == U"link")) {
                    emitCurrentTokenizerElement();
                    t.ackSelfClosingTag();
                } else if (currentTokenIsTagBegin() && t.tagName == U"meta") {
                    emitCurrentTokenizerElement();
                    t.ackSelfClosingTag();
                    // note: We ignore charset auto detection parts here
                } else if (currentTokenIsTagBegin() && t.tagName == U"title") {
                    // FIXME Follow the generic raw text element parsing algorithm.
                    t.errorUnimplemented("TODO title");
                } else if (currentTokenIsTagBegin() && t.tagName == U"noscript") {
                    emitCurrentTokenizerElement();
                    openElements.append(t.tagName);
                    // FIXME Switch the insertion mode to "in head noscript".
                    t.errorUnimplemented("TODO in head noscript");
                } else if (currentTokenIsTagBegin() && t.tagName == U"script") {
                    t.errorUnimplemented("TODO script");
                    // FIXME Switch the tokenizer to the script data state.
                    // FIXME Let the original insertion mode be the current insertion mode.
                    // FIXME Switch the insertion mode to "text".
                } else if (currentTokenIsTagEnd() && t.tagName == U"head") {
                    pendingElementEndEvents.append(openElements.last());
                    openElements.removeLast();
                    insertionMode = IM_AfterHead;
                } else if (currentTokenIsTagEnd() && (t.tagName == U"body" || t.tagName == U"html" || t.tagName == U"br")) {
                    pendingElementEndEvents.append(openElements.last());
                    openElements.removeLast();
                    insertionMode = IM_AfterHead;
                    reprocessToken = true;
                } else if (currentTokenIsTagBegin() && t.tagName == U"template") {
                    t.errorUnimplemented("template");
                } else if (currentTokenIsTagBegin() && t.tagName == U"head") {
                    t._isError = true;
                } else if (currentTokenIsTagEnd()) {
                    t._isError = true;
                } else {
                    pendingElementEndEvents.append(openElements.last());
                    openElements.removeLast();
                    insertionMode = IM_AfterHead;
                    reprocessToken = true;
                }
                } break;
            case IM_AfterHead: {
                if (currentTokenIsChar() &&
                   (t.currentChar == 9 || t.currentChar == 0xa || t.currentChar == 0xc || t.currentChar == 0xd || t.currentChar == ' ')) {
                    eventType = E_Char;
                    eventChar = t.currentChar;
                } else if (currentTokenIsComment()) {
                    ; // ignore (we are not maintaining comments)
                } else if (currentTokenIsDoctype()) {
                    t._isError = true;
                } else if (currentTokenIsTagBegin() && t.tagName == U"html") {
                    t._isError = true;
                } else if (currentTokenIsTagBegin() && t.tagName == U"body") {
                    emitCurrentTokenizerElement();
                    openElements.append(t.tagName);
                    // note: framesets are never ok in this impl.
                    insertionMode = IM_InBody;
                } else if (currentTokenIsTagBegin() && t.tagName == U"frameset") {
                    t.errorUnimplemented("frameset");
                } else if (currentTokenIsTagBegin() && (t.tagName == U"base" || t.tagName == U"basefont"
                          || t.tagName == U"bgsound" || t.tagName == U"link" || t.tagName == U"meta"
                          || t.tagName == U"noframes" || t.tagName == U"script" || t.tagName == U"style"
                          || t.tagName == U"template" || t.tagName == U"title")) {
                    t._isError = true;
                } else if (currentTokenIsTagEnd() && (t.tagName == U"body" || t.tagName == U"html" || t.tagName == U"br")) {
                    emitSyntheticElement(U"body");
                    openElements.append(U"body");
                    insertionMode = IM_InBody;
                    reprocessToken = true;
                } else if (currentTokenIsTagBegin() && t.tagName == U"head") {
                    t._isError = true;
                } else if (currentTokenIsTagEnd()) {
                    t._isError = true;
                } else {
                    emitSyntheticElement(U"body");
                    openElements.append(U"body");
                    insertionMode = IM_InBody;
                    reprocessToken = true;
                }
                } break;
            case IM_InBody: {
                if (currentTokenIsChar() && t.currentChar == 0) {
                    t._isError = true;
                } else if (currentTokenIsChar()) {
                    // ??? Reconstruct the active formatting elements, if any.
                    eventType = E_Char;
                    eventChar = t.currentChar;
                    // note: framesets are never ok in this impl.
                } else if (currentTokenIsComment()) {
                    ; // ignore (we are not maintaining comments)
                } else if (currentTokenIsDoctype()) {
                    t._isError = true;
                } else if (currentTokenIsTagBegin() && t.tagName == U"html") {
                    t._isError = true;
                } else if (currentTokenIsTagBegin() && (t.tagName == U"base" || t.tagName == U"basefont"
                          || t.tagName == U"bgsound" || t.tagName == U"link" || t.tagName == U"meta"
                          || t.tagName == U"noframes" || t.tagName == U"script" || t.tagName == U"style"
                          || t.tagName == U"template" || t.tagName == U"title")) {
                    t.errorUnimplemented("header tag in body");
                    // FIXME Process the token using the rules for the "in head" insertion mode.
                } else if (currentTokenIsTagBegin() && t.tagName == U"body") {
                    t._isError = true;
                } else if (currentTokenIsTagBegin() && t.tagName == U"frameset") {
                    t.errorUnimplemented("frameset");
                } else if (currentTokenIsEOF()) {
                    // note: template not supported
                    std::reverse(openElements.begin(), openElements.end());
                    for (std::u32string stillOpen : openElements) {
                        if (stillOpen != U"dd" && stillOpen != U"dt" && stillOpen != U"li" && stillOpen != U"p"
                            && stillOpen != U"tbody" && stillOpen != U"td" && stillOpen != U"tfoot" && stillOpen != U"th"
                            && stillOpen != U"thead" && stillOpen != U"tr" && stillOpen != U"body" && stillOpen != U"html") {
                            t._isError = true;
                            break;
                        }
                        pendingElementEndEvents.append(stillOpen);
                    }
                    openElements.clear();
                    if (!t._isError) {
                        eventType = E_DocumentEnd;
                    }
                } else if (currentTokenIsTagEnd() && t.tagName == U"body") {
                    if (!hasInScope(U"body")) {
                        t._isError = true;
                    } else {
                        for (std::u32string stillOpen : openElements) {
                            if (stillOpen != U"dd" && stillOpen != U"dt" && stillOpen != U"li"
                                && stillOpen != U"optgroup" && stillOpen != U"option" && stillOpen != U"p"
                                && stillOpen != U"rb" && stillOpen != U"rp" && stillOpen != U"rt" && stillOpen != U"rtc"
                                && stillOpen != U"tbody" && stillOpen != U"td" && stillOpen != U"tfoot"
                                && stillOpen != U"th" && stillOpen != U"thead" && stillOpen != U"tr"
                                && stillOpen != U"body" && stillOpen != U"html") {
                                t._isError = true;
                                break;
                            }
                        }
                        insertionMode = IM_AfterBody;
                    }
                } else if (currentTokenIsTagEnd() && t.tagName == U"html") {
                    if (!hasInScope(U"body")) {
                        t._isError = true;
                    } else {
                        for (std::u32string stillOpen : openElements) {
                            if (stillOpen != U"dd" && stillOpen != U"dt" && stillOpen != U"li"
                                && stillOpen != U"optgroup" && stillOpen != U"option" && stillOpen != U"p"
                                && stillOpen != U"rb" && stillOpen != U"rp" && stillOpen != U"rt" && stillOpen != U"rtc"
                                && stillOpen != U"tbody" && stillOpen != U"td" && stillOpen != U"tfoot"
                                && stillOpen != U"th" && stillOpen != U"thead" && stillOpen != U"tr"
                                && stillOpen != U"body" && stillOpen != U"html") {
                                t._isError = true;
                                break;
                            }
                        }
                        insertionMode = IM_AfterBody;
                        reprocessToken = true;
                    }
                } else if (currentTokenIsTagBegin() && (t.tagName == U"address" || t.tagName == U"article"
                          || t.tagName == U"aside" || t.tagName == U"blockquote" || t.tagName == U"center"
                          || t.tagName == U"details" || t.tagName == U"dialog" || t.tagName == U"dir"
                          || t.tagName == U"div" || t.tagName == U"dl" || t.tagName == U"fieldset"
                          || t.tagName == U"figcaption" || t.tagName == U"figure" || t.tagName == U"footer"
                          || t.tagName == U"header" || t.tagName == U"hgroup" || t.tagName == U"main" || t.tagName == U"nav"
                          || t.tagName == U"ol" || t.tagName == U"p" || t.tagName == U"section" || t.tagName == U"summary"
                          || t.tagName == U"ul")) {
                    if (hasInButtonScope(U"p")) {
                        closePElement();
                    }
                    if (!t._isError) {
                        emitCurrentTokenizerElement();
                        openElements.append(t.tagName);
                    }
                } else if (currentTokenIsTagBegin() && (t.tagName == U"h1" || t.tagName == U"h2"
                          || t.tagName == U"h3" || t.tagName == U"h4" || t.tagName == U"h5"
                          || t.tagName == U"h6")) {
                    if (hasInButtonScope(U"p")) {
                        closePElement();
                    }
                    if (!t._isError) {
                        static QSet<std::u32string> theSet = { U"h1", U"h2", U"h3", U"h4", U"h5", U"h6" };
                        if (theSet.contains(openElements.last())) {
                            t._isError = true;
                        }
                        emitCurrentTokenizerElement();
                        openElements.append(t.tagName);
                    }
                } else if (currentTokenIsTagBegin() && (t.tagName == U"pre" || t.tagName == U"listing")) {
                    if (hasInButtonScope(U"p")) {
                        closePElement();
                    }
                    if (!t._isError) {
                        emitCurrentTokenizerElement();
                        openElements.append(t.tagName);
                        discardOneLf = true;
                    }
                    // note: framesets are never ok in this impl.
                } else if (currentTokenIsTagBegin() && (t.tagName == U"form")) {
                    // ??? If the form element pointer is not null then this is a parse error
                    if (hasInButtonScope(U"p")) {
                        closePElement();
                    }
                    emitCurrentTokenizerElement();
                    openElements.append(t.tagName);
                    // ??? set the form element pointer to point to the element created.
                } else if (currentTokenIsTagBegin() && (t.tagName == U"li")) {
                    // note: framesets are never ok in this impl.
                    t.errorUnimplemented("TODO li");
                    // FIXME
                } else if (currentTokenIsTagBegin() && (t.tagName == U"dd" || t.tagName == U"dt")) {
                    t.errorUnimplemented("TODO dd/dt");
                    // FIXME
                } else if (currentTokenIsTagBegin() && (t.tagName == U"plaintext")) {
                    t.errorUnimplemented("depricated plaintext tag");
                } else if (currentTokenIsTagBegin() && (t.tagName == U"button")) {
                    if (hasInScope(U"button")) {
                        t._isError = true;
                    } else {
                        // ??? Reconstruct the active formatting elements, if any.
                        emitCurrentTokenizerElement();
                        openElements.append(t.tagName);
                    }
                    // note: framesets are never ok in this impl.
                } else if (currentTokenIsTagEnd() && (t.tagName == U"address" || t.tagName == U"article"
                          || t.tagName == U"aside" || t.tagName == U"blockquote" || t.tagName == U"button"
                          || t.tagName == U"center" || t.tagName == U"details" || t.tagName == U"dialog"
                          || t.tagName == U"dir" || t.tagName == U"div" || t.tagName == U"dl"
                          || t.tagName == U"fieldset" || t.tagName == U"figcaption" || t.tagName == U"figure"
                          || t.tagName == U"footer" || t.tagName == U"header" || t.tagName == U"hgroup"
                          || t.tagName == U"listing" || t.tagName == U"main" || t.tagName == U"nav"
                          || t.tagName == U"ol" || t.tagName == U"pre" || t.tagName == U"section"
                          || t.tagName == U"summary" || t.tagName == U"ul")) {
                    if (!hasInScope(t.tagName)) {
                        t._isError = true;
                    } else {
                        generateImpliedEndTags(U"");
                        if (openElements.last() != t.tagName) {
                            t._isError = true;
                        } else {
                            pendingElementEndEvents.append(openElements.last());
                            openElements.removeLast();
                        }
                    }
                } else if (currentTokenIsTagEnd() && (t.tagName == U"form")) {
                    t.errorUnimplemented("form");
                    // FIXME
                } else if (currentTokenIsTagEnd() && (t.tagName == U"p")) {
                    if (!hasInButtonScope(U"p")) {
                        t._isError = true;
                    } else {
                        closePElement();
                    }
                } else if (currentTokenIsTagEnd() && (t.tagName == U"li")) {
                    t.errorUnimplemented("TODO li");
                    // FIXME
                } else if (currentTokenIsTagEnd() && (t.tagName == U"dd" || t.tagName == U"dt")) {
                    t.errorUnimplemented("TODO dd/dt");
                    // FIXME
                } else if (currentTokenIsTagEnd() && (t.tagName == U"h1" || t.tagName == U"h2"
                          || t.tagName == U"h3" || t.tagName == U"h4" || t.tagName == U"h5"
                          || t.tagName == U"h6")) {

                    if (!hasInScope(t.tagName)) {
                        t._isError = true;
                    } else {
                        generateImpliedEndTags(U"");
                        if (openElements.last() != t.tagName) {
                            t._isError = true;
                        } else {
                            pendingElementEndEvents.append(openElements.last());
                            openElements.removeLast();
                        }
                    }
                } else if (currentTokenIsTagBegin() && (t.tagName == U"a")) {
                    t.errorUnimplemented("TODO a");
                    // FIXME
                } else if (currentTokenIsTagBegin() && (t.tagName == U"b" || t.tagName == U"big"
                          || t.tagName == U"code" || t.tagName == U"em" || t.tagName == U"font"
                          || t.tagName == U"i" || t.tagName == U"s" || t.tagName == U"small"
                          || t.tagName == U"strike" || t.tagName == U"strong" || t.tagName == U"tt"
                          || t.tagName == U"u")) {
                    // ??? Reconstruct the active formatting elements, if any.
                    emitCurrentTokenizerElement();
                    openElements.append(t.tagName);
                    // ??? Push onto the list of active formatting elements that element.
                } else if (currentTokenIsTagBegin() && (t.tagName == U"nobr")) {
                    // ??? Reconstruct the active formatting elements, if any.
                    if (!hasInScope(t.tagName)) {
                        t._isError = true;
                    } else {
                        emitCurrentTokenizerElement();
                        openElements.append(t.tagName);
                        // ??? Push onto the list of active formatting elements that element.
                    }
                } else if (currentTokenIsTagEnd() && (t.tagName == U"a" || t.tagName == U"b" || t.tagName == U"big"
                          || t.tagName == U"code" || t.tagName == U"em" || t.tagName == U"font"
                          || t.tagName == U"i" || t.tagName == U"nobr" || t.tagName == U"s" || t.tagName == U"small"
                          || t.tagName == U"strike" || t.tagName == U"strong" || t.tagName == U"tt"
                          || t.tagName == U"u")) {
                    miniAAA(t.tagName);
                } else if (currentTokenIsTagBegin() && (t.tagName == U"applet" || t.tagName == U"marquee"
                          || t.tagName == U"object")) {
                    // ??? Reconstruct the active formatting elements, if any.
                    emitCurrentTokenizerElement();
                    openElements.append(t.tagName);
                    // ??? Insert a marker at the end of the list of active formatting elements.
                    // note: framesets are never ok in this impl.
                } else if (currentTokenIsTagEnd() && (t.tagName == U"applet" || t.tagName == U"marquee"
                          || t.tagName == U"object")) {
                    if (!hasInScope(t.tagName)) {
                        t._isError = true;
                    } else {
                        generateImpliedEndTags(U"");
                        if (openElements.last() != t.tagName) {
                            t._isError = true;
                        } else {
                            pendingElementEndEvents.append(openElements.last());
                            openElements.removeLast();
                        }
                        // ??? Clear the list of active formatting elements up to the last marker.
                    }
                } else if (currentTokenIsTagBegin() && (t.tagName == U"table")) {
                    t.errorUnimplemented("tables");
                } else if (currentTokenIsTagEnd() && (t.tagName == U"br")) {
                    t._isError = true;
                } else if (currentTokenIsTagBegin() && (t.tagName == U"area" || t.tagName == U"br"
                          || t.tagName == U"embed" || t.tagName == U"img" || t.tagName == U"keygen"
                          || t.tagName == U"wbr")) {
                    // ??? Reconstruct the active formatting elements, if any.
                    emitCurrentTokenizerElement();
                    t.ackSelfClosingTag();
                    // note: framesets are never ok in this impl.
                } else if (currentTokenIsTagBegin() && (t.tagName == U"input")) {
                    // ??? Reconstruct the active formatting elements, if any.
                    emitCurrentTokenizerElement();
                    t.ackSelfClosingTag();
                    // note: framesets are never ok in this impl.
                } else if (currentTokenIsTagEnd() && (t.tagName == U"param" || t.tagName == U"source"
                          || t.tagName == U"track")) {
                    emitCurrentTokenizerElement();
                    t.ackSelfClosingTag();
                } else if (currentTokenIsTagBegin() && (t.tagName == U"hr")) {
                    if (hasInButtonScope(U"p")) {
                        closePElement();
                    }
                    emitCurrentTokenizerElement();
                    t.ackSelfClosingTag();
                    // note: framesets are never ok in this impl.
                } else if (currentTokenIsTagBegin() && (t.tagName == U"image")) {
                    t._isError = true;
                } else if (currentTokenIsTagBegin() && (t.tagName == U"isindex")) {
                    t._isError = true;
                } else if (currentTokenIsTagBegin() && (t.tagName == U"textarea")) {
                    t.errorUnimplemented("textarea");
                    emitCurrentTokenizerElement();
                    openElements.append(t.tagName);
                    discardOneLf = true;
                    // FIXME Switch the tokenizer to the RCDATA state.
                    // FIXME Let the original insertion mode be the current insertion mode.
                    // FIXME Switch the insertion mode to "text".
                } else if (currentTokenIsTagBegin() && (t.tagName == U"xmp")) {
                    t.errorUnimplemented("xmp");
                    // FIXME
                } else if (currentTokenIsTagBegin() && (t.tagName == U"iframe")) {
                    t.errorUnimplemented("iframe");
                    // FIXME
                } else if (currentTokenIsTagBegin() && (t.tagName == U"noembed")) {
                    t.errorUnimplemented("noembed");
                } else if (currentTokenIsTagBegin() && (t.tagName == U"select")) {
                    t.errorUnimplemented("select");
                } else if (currentTokenIsTagBegin() && (t.tagName == U"optgroup" || t.tagName == U"option")) {
                    if (openElements.last() == U"option") {
                        pendingElementEndEvents.append(openElements.last());
                        openElements.removeLast();
                    }
                    // ??? Reconstruct the active formatting elements, if any.
                    emitCurrentTokenizerElement();
                    openElements.append(t.tagName);
                } else if (currentTokenIsTagBegin() && (t.tagName == U"rb" || t.tagName == U"rp" || t.tagName == U"rtc")) {
                    if (hasInScope(U"ruby")) {
                        generateImpliedEndTags(U"");
                    }
                    if (openElements.last() != U"ruby") {
                        t._isError = true;
                    }
                    if (!t._isError) {
                        emitCurrentTokenizerElement();
                        openElements.append(t.tagName);
                    }
                } else if (currentTokenIsTagBegin() && (t.tagName == U"rt")) {
                    if (hasInScope(U"ruby")) {
                        generateImpliedEndTags(U"rtc");
                    }
                    if (openElements.last() != U"ruby" && openElements.last() != U"rtc") {
                        t._isError = true;
                    }
                    if (!t._isError) {
                        emitCurrentTokenizerElement();
                        openElements.append(t.tagName);
                    }
                } else if (currentTokenIsTagBegin() && (t.tagName == U"math")) {
                    t.errorUnimplemented("mathml");
                } else if (currentTokenIsTagBegin() && (t.tagName == U"svg")) {
                    t.errorUnimplemented("svg");
                } else if (currentTokenIsTagBegin() && (t.tagName == U"caption" || t.tagName == U"col"
                          || t.tagName == U"colgroup" || t.tagName == U"frame" || t.tagName == U"head"
                          || t.tagName == U"tbody" || t.tagName == U"td" || t.tagName == U"tfoot" || t.tagName == U"th"
                          || t.tagName == U"thead" || t.tagName == U"tr")) {
                    t._isError = true;
                } else if (currentTokenIsTagBegin()) {
                    // ??? Reconstruct the active formatting elements, if any.
                    emitCurrentTokenizerElement();
                    openElements.append(t.tagName);
                } else if (currentTokenIsTagEnd()) {
                    bool good = false;
                    for (auto it = openElements.rbegin(); it != openElements.rend(); it++) {
                        if (*it == t.tagName) {
                            good = true;
                            break;
                        }
                        if (isSpecial(*it)) {
                            break;
                        }
                    }
                    if (!good) {
                        t._isError = true;
                    } else {
                        generateImpliedEndTags(t.tagName);
                        if (openElements.last() != t.tagName) {
                            t._isError = true;
                        } else {
                            pendingElementEndEvents.append(openElements.last());
                            openElements.removeLast();
                        }
                    }
                }

                } break;
            case IM_AfterBody: {
                if (currentTokenIsChar()
                   && (t.currentChar == 9 || t.currentChar == 0xa || t.currentChar == 0xc || t.currentChar == 0xd || t.currentChar == ' ')) {
                    // ??? Reconstruct the active formatting elements, if any.
                    eventType = E_Char;
                    eventChar = t.currentChar;
                } else if (currentTokenIsComment()) {
                    ; // ignore (we are not maintaining comments)
                } else if (currentTokenIsDoctype()) {
                    t._isError = true;
                } else if (currentTokenIsTagBegin() && t.tagName == U"html") {
                    t._isError = true;
                } else if (currentTokenIsTagEnd() && t.tagName == U"html") {
                    insertionMode = IM_AfterAfterBody;
                } else if (currentTokenIsEOF()) {
                    std::reverse(openElements.begin(), openElements.end());
                    for (std::u32string stillOpen : openElements) {
                        pendingElementEndEvents.append(stillOpen);
                    }
                    openElements.clear();
                    eventType = E_DocumentEnd;
                } else {
                    t._isError = true;
                }
                } break;
            case IM_AfterAfterBody: {
                if (currentTokenIsComment()) {
                    ; // ignore (we are not maintaining comments)
                } else if (currentTokenIsDoctype()) {
                    t._isError = true;
                } else if (currentTokenIsChar()
                          && (t.currentChar == 9 || t.currentChar == 0xa || t.currentChar == 0xc || t.currentChar == 0xd || t.currentChar == ' ')) {
                    // ??? Reconstruct the active formatting elements, if any.
                    eventType = E_Char;
                    eventChar = t.currentChar;
                } else if (currentTokenIsTagBegin() && t.tagName == U"html") {
                    t._isError = true;
                } else if (currentTokenIsEOF()) {
                    std::reverse(openElements.begin(), openElements.end());
                    for (std::u32string stillOpen : openElements) {
                        pendingElementEndEvents.append(stillOpen);
                    }
                    openElements.clear();
                    eventType = E_DocumentEnd;
                } else {
                    t._isError = true;
                }
                } break;
        }
        if (pendingElementEndEvents.size()) {
            hasEndTag = true;
            eventTagEnd = QString::fromStdU32String(pendingElementEndEvents.first());
            pendingElementEndEvents.removeFirst();
            return;
        }
    }
}

void MarkupParser::miniAAA(std::u32string element) {
    if (openElements.last() == element) {
        pendingElementEndEvents.append(openElements.last());
        openElements.removeLast();
        // ??? If element is also in the list of active formatting elements, remove the element from the list.
    } else {
        t.errorUnimplemented("mis-nested tags: adoption agency algorithm");
    }
}

void MarkupParser::closePElement() {
    generateImpliedEndTags(U"p");
    if (openElements.last() != U"p") {
        t._isError = true;
    } else {
        pendingElementEndEvents.append(openElements.last());
        openElements.removeLast();
    }
}

void MarkupParser::generateImpliedEndTags(std::u32string excluded) {
    static QSet<std::u32string> theSet = {
        U"dd",
        U"dt",
        U"li",
        U"option",
        U"optgroup",
        U"p",
        U"rb",
        U"rp",
        U"rt",
        U"rtc"
    };

    while (theSet.contains(openElements.last())) {
        if (openElements.last() == excluded) {
            break;
        }
        pendingElementEndEvents.append(openElements.last());
        openElements.removeLast();
    }
}

bool MarkupParser::inSpecificScope(std::u32string element, QSet<std::u32string> set) {
    for (auto it = openElements.rbegin(); it != openElements.rend(); it++) {
        if (*it == element) {
            return true;
        }
        if (set.contains(element)) {
            return false;
        }
    }
    return false;
}

static QSet<std::u32string> scopeSet() {
    static QSet<std::u32string> theSet = {
        U"applet",
        U"caption",
        U"html",
        U"table",
        U"td",
        U"th",
        U"marquee",
        U"object",
        U"template"
    };
    return theSet;
}

bool MarkupParser::hasInButtonScope(std::u32string element) {
    static QSet<std::u32string> theSet = scopeSet()
            + QSet<std::u32string>{ U"button" };

    return inSpecificScope(element, theSet);
}

bool MarkupParser::hasInScope(std::u32string element) {
    return inSpecificScope(element, scopeSet());
}

bool MarkupParser::isSpecial(std::u32string element) {
    static QSet<std::u32string> theSet = {
        U"address", U"applet", U"area", U"article", U"aside", U"base", U"basefont", U"bgsound", U"blockquote",
        U"body", U"br", U"button", U"caption", U"center", U"col", U"colgroup", U"dd", U"details", U"dir", U"div", U"dl",
        U"dt", U"embed", U"fieldset", U"figcaption", U"figure", U"footer", U"form", U"frame", U"frameset", U"h1", U"h2",
        U"h3", U"h4", U"h5", U"h6", U"head", U"header", U"hgroup", U"hr", U"html", U"iframe", U"img", U"input",
        U"isindex", U"li", U"link", U"listing", U"main", U"marquee", U"meta", U"nav", U"noembed", U"noframes",
        U"noscript", U"object", U"ol", U"p", U"param", U"plaintext", U"pre", U"script", U"section", U"select",
        U"source", U"style", U"summary", U"table", U"tbody", U"td", U"template", U"textarea", U"tfoot", U"th", U"thead",
        U"title", U"tr", U"track", U"ul", U"wbr", U"xmp"
    };
    return theSet.contains(element);
}

}
TUIWIDGETS_NS_END
