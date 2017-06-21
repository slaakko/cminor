#ifndef Literal_hpp_19453
#define Literal_hpp_19453

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/codedom/Literal.hpp>

namespace cminor { namespace code {

class LiteralGrammar : public cminor::parsing::Grammar
{
public:
    static LiteralGrammar* Create();
    static LiteralGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    cminor::codedom::Literal* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName);
private:
    LiteralGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class LiteralRule;
    class IntegerLiteralRule;
    class CharacterLiteralRule;
    class CCharSequenceRule;
    class FloatingLiteralRule;
    class StringLiteralRule;
    class BooleanLiteralRule;
    class PointerLiteralRule;
};

} } // namespace cminor.code

#endif // Literal_hpp_19453
