// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_PARSING_PRIMITIVE_INCLUDED
#define CMINOR_PARSING_PRIMITIVE_INCLUDED
#include <cminor/parsing/Parser.hpp>
#include <limits.h>
#include <bitset>

namespace cminor { namespace parsing {

class Visitor;
class Rule;

class CharParser : public Parser
{
public:
    CharParser(char32_t c_);
    char32_t GetChar() const { return c; }
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
private:
    char32_t c;
};

class StringParser : public Parser
{
public:
    StringParser(const std::u32string& s_);
    const std::u32string& GetString() const { return s; }
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
private:
    std::u32string s;
};

struct CharRange
{
    CharRange(char32_t start_, char32_t end_) : start(start_), end(end_) {}
    bool Includes(char32_t c) const { return c >= start && c <= end; }
    char32_t start;
    char32_t end;
};

class CharSetParser : public Parser
{
public:
    CharSetParser(const std::u32string& s_);
    CharSetParser(const std::u32string& s_, bool inverse_);
    const std::u32string& Set() const { return s; }
    bool Inverse() const { return inverse; }
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
private:
    std::u32string s;
    bool inverse;
    std::vector<CharRange> ranges;
    void InitRanges();
};

class EmptyParser : public Parser
{
public:
    EmptyParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class SpaceParser : public Parser
{
public:
    SpaceParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class LetterParser : public Parser
{
public:
    LetterParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class UpperLetterParser : public Parser
{
public:
    UpperLetterParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class LowerLetterParser : public Parser
{
public:
    LowerLetterParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class TitleLetterParser : public Parser
{
public:
    TitleLetterParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class ModifierLetterParser : public Parser
{
public:
    ModifierLetterParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class OtherLetterParser : public Parser
{
public:
    OtherLetterParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class CasedLetterParser : public Parser
{
public:
    CasedLetterParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class DigitParser : public Parser
{
public:
    DigitParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class HexDigitParser : public Parser
{
public:
    HexDigitParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class MarkParser : public Parser
{
public:
    MarkParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class NonspacingMarkParser : public Parser
{
public:
    NonspacingMarkParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class SpacingMarkParser : public Parser
{
public:
    SpacingMarkParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class EnclosingMarkParser : public Parser
{
public:
    EnclosingMarkParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class NumberParser : public Parser
{
public:
    NumberParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class DecimalNumberParser : public Parser
{
public:
    DecimalNumberParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class LetterNumberParser : public Parser
{
public:
    LetterNumberParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class OtherNumberParser : public Parser
{
public:
    OtherNumberParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class PunctuationParser : public Parser
{
public:
    PunctuationParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class ConnectorPunctuationParser : public Parser
{
public:
    ConnectorPunctuationParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class DashPunctuationParser : public Parser
{
public:
    DashPunctuationParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class OpenPunctuationParser : public Parser
{
public:
    OpenPunctuationParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class ClosePunctuationParser : public Parser
{
public:
    ClosePunctuationParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class InitialPunctuationParser : public Parser
{
public:
    InitialPunctuationParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class FinalPunctuationParser : public Parser
{
public:
    FinalPunctuationParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class OtherPunctuationParser : public Parser
{
public:
    OtherPunctuationParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class SymbolParser : public Parser
{
public:
    SymbolParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class MathSymbolParser : public Parser
{
public:
    MathSymbolParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class CurrencySymbolParser : public Parser
{
public:
    CurrencySymbolParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class ModifierSymbolParser : public Parser
{
public:
    ModifierSymbolParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class OtherSymbolParser : public Parser
{
public:
    OtherSymbolParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class SeparatorParser : public Parser
{
public:
    SeparatorParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class SpaceSeparatorParser : public Parser
{
public:
    SpaceSeparatorParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class LineSeparatorParser : public Parser
{
public:
    LineSeparatorParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class ParagraphSeparatorParser : public Parser
{
public:
    ParagraphSeparatorParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class OtherParser : public Parser
{
public:
    OtherParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class ControlParser : public Parser
{
public:
    ControlParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class FormatParser : public Parser
{
public:
    FormatParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class SurrogateParser : public Parser
{
public:
    SurrogateParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class PrivateUseParser : public Parser
{
public:
    PrivateUseParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class UnassignedParser : public Parser
{
public:
    UnassignedParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class GraphicParser : public Parser
{
public:
    GraphicParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class BaseCharParser : public Parser
{
public:
    BaseCharParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class AlphabeticParser : public Parser
{
public:
    AlphabeticParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class IdStartParser : public Parser
{
public:
    IdStartParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class IdContParser : public Parser
{
public:
    IdContParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class AnyCharParser : public Parser
{
public:
    AnyCharParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class RangeParser : public Parser
{
public:
    RangeParser(uint32_t start_, uint32_t end_);
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
    uint32_t Start() const { return start; }
    uint32_t End() const { return end; }
private:
    uint32_t start;
    uint32_t end;
};

} } // namespace cminor::parsing

#endif // CMINOR_PARSING_PRIMITIVE_INCLUDED
