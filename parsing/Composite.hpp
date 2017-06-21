// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_PARSING_COMPOSITE_INCLUDED
#define CMINOR_PARSING_COMPOSITE_INCLUDED
#include <cminor/parsing/Parser.hpp>

namespace cminor { namespace parsing {

class Visitor;

class UnaryParser : public Parser
{
public:
    UnaryParser(const std::u32string& name_, Parser* child_, const std::u32string& info_);
    Parser* Child() const { return child; }
private:
    Parser* child;
};

class OptionalParser : public UnaryParser
{
public:
    OptionalParser(Parser* child_);
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class PositiveParser : public UnaryParser
{
public:
    PositiveParser(Parser* child_);
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class KleeneStarParser : public UnaryParser
{
public:
    KleeneStarParser(Parser* child_);
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class ExpectationParser : public UnaryParser
{
public:
    ExpectationParser(Parser* child_);
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class TokenParser : public UnaryParser
{
public:
    TokenParser(Parser* child_);
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class BinaryParser : public Parser
{
public:
    BinaryParser(const std::u32string& name_, Parser* left_, Parser* right_, const std::u32string& info_);
    Parser* Left() const { return left; }
    Parser* Right() const { return right; }
private:
    Parser* left;
    Parser* right;
};

class SequenceParser : public BinaryParser
{
public:
    SequenceParser(Parser* left_, Parser* right_);
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class AlternativeParser : public BinaryParser
{
public:
    AlternativeParser(Parser* left_, Parser* right_);
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class DifferenceParser : public BinaryParser
{
public:
    DifferenceParser(Parser* left_, Parser* right_);
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class ExclusiveOrParser : public BinaryParser
{
public:
    ExclusiveOrParser(Parser* left_, Parser* right_);
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class IntersectionParser : public BinaryParser
{
public:
    IntersectionParser(Parser* left_, Parser* right_);
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
};

class ListParser : public UnaryParser
{
public:
    ListParser(Parser* left_, Parser* right_);
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData);
    virtual void Accept(Visitor& visitor);
private:
    Parser* left;
    Parser* right;
};

} } // namespace cminor::parsing

#endif // CMINOR_PARSING_COMPOSITE_INCLUDED
