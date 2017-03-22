#ifndef Rule_hpp_5596
#define Rule_hpp_5596

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/pl/Rule.hpp>

namespace cpg { namespace syntax {

class RuleGrammar : public cminor::parsing::Grammar
{
public:
    static RuleGrammar* Create();
    static RuleGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    cminor::parsing::Rule* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, cminor::parsing::Scope* enclosingScope);
private:
    RuleGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class RuleRule;
    class RuleHeaderRule;
    class RuleBodyRule;
};

} } // namespace cpg.syntax

#endif // Rule_hpp_5596
