/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#ifndef CMINOR_PARSING_COMPOSITE_INCLUDED
#define CMINOR_PARSING_COMPOSITE_INCLUDED
#include <cminor/pl/Parser.hpp>

namespace cminor { namespace parsing {

class Visitor;

class UnaryParser : public Parser
{
public:
    UnaryParser(const std::string& name_, Parser* child_, const std::string& info_);
    Parser* Child() const { return child; }
private:
    Parser* child;
};

class OptionalParser : public UnaryParser
{
public:
    OptionalParser(Parser* child_);
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
};

class PositiveParser : public UnaryParser
{
public:
    PositiveParser(Parser* child_);
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
};

class KleeneStarParser : public UnaryParser
{
public:
    KleeneStarParser(Parser* child_);
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
};

class ExpectationParser : public UnaryParser
{
public:
    ExpectationParser(Parser* child_);
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
};

class CCOptParser : public UnaryParser
{
public:
    CCOptParser(Parser* child_);
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
};

class TokenParser : public UnaryParser
{
public:
    TokenParser(Parser* child_);
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
};

class BinaryParser : public Parser
{
public:
    BinaryParser(const std::string& name_, Parser* left_, Parser* right_, const std::string& info_);
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
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
};

class AlternativeParser : public BinaryParser
{
public:
    AlternativeParser(Parser* left_, Parser* right_);
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
};

class DifferenceParser : public BinaryParser
{
public:
    DifferenceParser(Parser* left_, Parser* right_);
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
};

class ExclusiveOrParser : public BinaryParser
{
public:
    ExclusiveOrParser(Parser* left_, Parser* right_);
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
};

class IntersectionParser : public BinaryParser
{
public:
    IntersectionParser(Parser* left_, Parser* right_);
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
};

class ListParser : public UnaryParser
{
public:
    ListParser(Parser* left_, Parser* right_);
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
private:
    Parser* left;
    Parser* right;
};

} } // namespace Cm::Parsing

#endif // CMINOR_PARSING_COMPOSITE_INCLUDED
