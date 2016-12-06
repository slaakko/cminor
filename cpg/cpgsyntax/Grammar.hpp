#ifndef Grammar_hpp_8022
#define Grammar_hpp_8022

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/pl/Grammar.hpp>

namespace cpg { namespace syntax {

class GrammarGrammar : public cminor::parsing::Grammar
{
public:
    static GrammarGrammar* Create();
    static GrammarGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    cminor::parsing::Grammar* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, cminor::parsing::Scope* enclosingScope);
private:
    GrammarGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class GrammarRule;
    class GrammarContentRule;
    class StartClauseRule;
    class SkipClauseRule;
    class CCClauseRule;
};

} } // namespace cpg.syntax

#endif // Grammar_hpp_8022
