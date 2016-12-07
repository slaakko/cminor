#ifndef Keyword_hpp_20441
#define Keyword_hpp_20441

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>

namespace cpg { namespace cpp {

class KeywordGrammar : public cminor::parsing::Grammar
{
public:
    static KeywordGrammar* Create();
    static KeywordGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
private:
    std::vector<std::string> keywords0;
    KeywordGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
};

} } // namespace cpg.cpp

#endif // Keyword_hpp_20441
