#ifndef Literal_hpp_21399
#define Literal_hpp_21399

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/ast/Literal.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class LiteralGrammar : public cminor::parsing::Grammar
{
public:
    static LiteralGrammar* Create();
    static LiteralGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    Node* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName);
private:
    LiteralGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class LiteralRule;
    class BooleanLiteralRule;
    class FloatingLiteralRule;
    class FloatingLiteralValueRule;
    class IntegerLiteralRule;
    class IntegerLiteralValueRule;
    class HexIntegerLiteralRule;
    class DecIntegerLiteralRule;
    class CharLiteralRule;
    class StringLiteralRule;
    class NullLiteralRule;
    class CharEscapeRule;
    class DecDigitSequenceRule;
    class HexDigitSequenceRule;
    class HexDigit4Rule;
    class HexDigit8Rule;
    class OctalDigitSequenceRule;
};

} } // namespace cminor.parser

#endif // Literal_hpp_21399
