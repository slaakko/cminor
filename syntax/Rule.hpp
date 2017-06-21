#ifndef Rule_hpp_20364
#define Rule_hpp_20364

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/parsing/Rule.hpp>

namespace cminor { namespace syntax {

class RuleGrammar : public cminor::parsing::Grammar
{
public:
    static RuleGrammar* Create();
    static RuleGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    cminor::parsing::Rule* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, cminor::parsing::Scope* enclosingScope);
private:
    RuleGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class RuleRule;
    class RuleHeaderRule;
    class RuleBodyRule;
};

} } // namespace cminor.syntax

#endif // Rule_hpp_20364
