// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Scanner.hpp>
#include <cminor/pl/Visitor.hpp>
#include <cminor/pl/Rule.hpp>
#include <cctype>

namespace cminor { namespace parsing {

CharParser::CharParser(char c_): Parser("char", "\"" + std::string(1, c_) + "\""), c(c_), ccRule(nullptr)
{
}

Match CharParser::Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData)
{
    if (!scanner.AtEnd())
    {
        if (scanner.GetChar() == c)
        {
            if (scanner.Synchronizing() && ccRule && ccRule->CCSkip() && c == ccRule->CCEnd() && ccRule->CCCount() > 0)
            {
                ccRule->DecCCCount();
                if (ccRule->CCCount() == 0)
                {
                    ++scanner;
                }
                return Match::One();
            }
            else
            {
                ++scanner;
                if (ccRule)
                {
                    if (c == ccRule->CCStart() && !scanner.Synchronizing())
                    {
                        ccRule->IncCCCount();
                    }
                    else if (c == ccRule->CCEnd())
                    {
                        ccRule->DecCCCount();
                    }
                }
                return Match::One();
            }
        }
    }
    return Match::Nothing();
}

void CharParser::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

StringParser::StringParser(const std::string& s_): Parser("string", "\"" + s_ + "\""), s(s_) 
{
}

Match StringParser::Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData)
{
    Match match = Match::Empty();
    int i = 0;
    int n = int(s.length());
    while (i < n && !scanner.AtEnd() && scanner.GetChar() == s[i])
    {
        ++scanner;
        ++i;
        match.Concatenate(Match::One());
    }
    if (i == n)
    {
        return match;
    }
    return Match::Nothing();
}

void StringParser::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

CharSetParser::CharSetParser(const std::string& s_): Parser("charSet", "[" + s_ + "]"), s(s_), inverse(false) 
{ 
    InitBits(); 
}

CharSetParser::CharSetParser(const std::string& s_, bool inverse_): Parser("charSet", "[" + s_ + "]"), s(s_), inverse(inverse_) 
{ 
    InitBits(); 
}

void CharSetParser::InitBits()
{
    int i = 0;
    int n = int(s.length());
    while (i < n)
    {
        int first = s[i];
        int last = first;
        ++i;
        if (i < n)
        {
            if (s[i] == '-')
            {
                ++i;
                if (i < n)
                {
                    last = s[i];
                    ++i;
                }
                else
                {
                    bits.set((unsigned char)first);
                    first = '-';
                    last = '-';
                }
            }
        }
        for (int b = first; b <= last; ++b)
        {
            bits.set((unsigned char)b);
        }
    }
}

Match CharSetParser::Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData)
{
    if (!scanner.AtEnd())
    {
        if (bits[(unsigned char)scanner.GetChar()] != inverse)
        {
            ++scanner;
            return Match::One();
        }
    }
    return Match::Nothing();
}

void CharSetParser::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

EmptyParser::EmptyParser(): Parser("empty", "") 
{
}

Match EmptyParser::Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData)
{
    return Match::Empty();
}

void EmptyParser::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

SpaceParser::SpaceParser(): Parser("space", "space") 
{
}

Match SpaceParser::Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData)
{
    if (!scanner.AtEnd())
    {
        if (std::isspace(scanner.GetChar()))
        {
            ++scanner;
            return Match::One();
        }
    }
    return Match::Nothing();
}

void SpaceParser::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

LetterParser::LetterParser(): Parser("letter", "letter") 
{
}

Match LetterParser::Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData)
{
    if (!scanner.AtEnd())
    {
        if (std::isalpha(scanner.GetChar()))
        {
            ++scanner;
            return Match::One();
        }
    }
    return Match::Nothing();
}

void LetterParser::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

DigitParser::DigitParser(): Parser("digit", "digit") 
{
}

Match DigitParser::Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData)
{
    if (!scanner.AtEnd())
    {
        if (std::isdigit(scanner.GetChar()))
        {
            ++scanner;
            return Match::One();
        }
    }
    return Match::Nothing();
}

void DigitParser::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

HexDigitParser::HexDigitParser(): Parser("hexdigit", "hexdigit") 
{
}

Match HexDigitParser::Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData)
{
    if (!scanner.AtEnd())
    {
        if (std::isxdigit(scanner.GetChar()))
        {
            ++scanner;
            return Match::One();
        }
    }
    return Match::Nothing();
}

void HexDigitParser::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

PunctuationParser::PunctuationParser(): Parser("punctuation", "punctuation") 
{
}

Match PunctuationParser::Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData)
{
    if (!scanner.AtEnd())
    {
        if (std::ispunct(scanner.GetChar()))
        {
            ++scanner;
            return Match::One();
        }
    }
    return Match::Nothing();
}

void PunctuationParser::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

AnyCharParser::AnyCharParser(): Parser("anychar", "anychar") 
{
}

Match AnyCharParser::Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData)
{
    if (!scanner.AtEnd())
    {
        ++scanner;
        return Match::One();
    }
    return Match::Nothing();
}

void AnyCharParser::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}


} } // namespace cminor::parsing
