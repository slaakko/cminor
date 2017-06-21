#ifndef Keyword_hpp_21399
#define Keyword_hpp_21399

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>

namespace cminor { namespace parser {

class KeywordGrammar : public cminor::parsing::Grammar
{
public:
    static KeywordGrammar* Create();
    static KeywordGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
private:
    std::vector<std::u32string> keywords0;
    KeywordGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
};

} } // namespace cminor.parser

#endif // Keyword_hpp_21399
