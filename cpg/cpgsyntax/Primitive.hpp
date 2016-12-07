#ifndef Primitive_hpp_20689
#define Primitive_hpp_20689

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Keyword.hpp>

namespace cpg { namespace syntax {

class PrimitiveGrammar : public cminor::parsing::Grammar
{
public:
    static PrimitiveGrammar* Create();
    static PrimitiveGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    cminor::parsing::Parser* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName);
private:
    PrimitiveGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class PrimitiveRule;
    class CharRule;
    class StringRule;
    class CharSetRule;
    class CharSetRangeRule;
    class CharSetCharRule;
    class KeywordRule;
    class KeywordBodyRule;
    class KeywordListRule;
    class KeywordListBodyRule;
    class EmptyRule;
    class SpaceRule;
    class AnyCharRule;
    class LetterRule;
    class DigitRule;
    class HexDigitRule;
    class PunctuationRule;
};

} } // namespace cpg.syntax

#endif // Primitive_hpp_20689
