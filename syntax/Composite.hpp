#ifndef Composite_hpp_20364
#define Composite_hpp_20364

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/parsing/Scope.hpp>
#include <cminor/parsing/Parser.hpp>

namespace cminor { namespace syntax {

class CompositeGrammar : public cminor::parsing::Grammar
{
public:
    static CompositeGrammar* Create();
    static CompositeGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    cminor::parsing::Parser* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, cminor::parsing::Scope* enclosingScope);
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

} } // namespace cminor.syntax

#endif // Composite_hpp_20364
