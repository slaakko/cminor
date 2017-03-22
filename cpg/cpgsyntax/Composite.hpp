#ifndef Composite_hpp_5596
#define Composite_hpp_5596

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/pl/Scope.hpp>
#include <cminor/pl/Parser.hpp>

namespace cpg { namespace syntax {

class CompositeGrammar : public cminor::parsing::Grammar
{
public:
    static CompositeGrammar* Create();
    static CompositeGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    cminor::parsing::Parser* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, cminor::parsing::Scope* enclosingScope);
private:
    CompositeGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class AlternativeRule;
    class SequenceRule;
    class DifferenceRule;
    class ExclusiveOrRule;
    class IntersectionRule;
    class ListRule;
    class PostfixRule;
};

} } // namespace cpg.syntax

#endif // Composite_hpp_5596
