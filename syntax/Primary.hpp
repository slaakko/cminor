#ifndef Primary_hpp_20364
#define Primary_hpp_20364

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/parsing/Parser.hpp>
#include <cminor/parsing/Scope.hpp>

namespace cminor { namespace syntax {

class PrimaryGrammar : public cminor::parsing::Grammar
{
public:
    static PrimaryGrammar* Create();
    static PrimaryGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    cminor::parsing::Parser* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, cminor::parsing::Scope* enclosingScope);
private:
    PrimaryGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class PrimaryRule;
    class RuleCallRule;
    class NonterminalRule;
    class AliasRule;
    class GroupingRule;
    class TokenRule;
    class ExpectationRule;
    class ActionRule;
};

} } // namespace cminor.syntax

#endif // Primary_hpp_20364
