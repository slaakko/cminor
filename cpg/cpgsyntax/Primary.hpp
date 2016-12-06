#ifndef Primary_hpp_22788
#define Primary_hpp_22788

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/pl/Parser.hpp>
#include <cminor/pl/Scope.hpp>

namespace cpg { namespace syntax {

class PrimaryGrammar : public cminor::parsing::Grammar
{
public:
    static PrimaryGrammar* Create();
    static PrimaryGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    cminor::parsing::Parser* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, cminor::parsing::Scope* enclosingScope);
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
    class CCOptRule;
};

} } // namespace cpg.syntax

#endif // Primary_hpp_22788
