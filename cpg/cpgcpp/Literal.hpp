#ifndef Literal_hpp_5397
#define Literal_hpp_5397

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/pom/Literal.hpp>

namespace cpg { namespace cpp {

class LiteralGrammar : public cminor::parsing::Grammar
{
public:
    static LiteralGrammar* Create();
    static LiteralGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    cminor::pom::Literal* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName);
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

} } // namespace cpg.cpp

#endif // Literal_hpp_5397
