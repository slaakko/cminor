#ifndef Grammar_hpp_20364
#define Grammar_hpp_20364

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/parsing/Grammar.hpp>

namespace cminor { namespace syntax {

class GrammarGrammar : public cminor::parsing::Grammar
{
public:
    static GrammarGrammar* Create();
    static GrammarGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    cminor::parsing::Grammar* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, cminor::parsing::Scope* enclosingScope);
private:
    GrammarGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class GrammarRule;
    class GrammarContentRule;
    class StartClauseRule;
    class SkipClauseRule;
};

} } // namespace cminor.syntax

#endif // Grammar_hpp_20364
