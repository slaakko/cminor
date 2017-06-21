#ifndef StdLib_hpp_18303
#define StdLib_hpp_18303

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <stdint.h>

namespace cminor { namespace parsing {

class stdlib : public cminor::parsing::Grammar
{
public:
    static stdlib* Create();
    static stdlib* Create(cminor::parsing::ParsingDomain* parsingDomain);
private:
    stdlib(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class intRule;
    class uintRule;
    class longRule;
    class ulongRule;
    class hexuintRule;
    class hexRule;
    class hex_literalRule;
    class realRule;
    class urealRule;
    class numRule;
    class boolRule;
    class identifierRule;
    class qualified_idRule;
    class escapeRule;
    class charRule;
    class stringRule;
};

} } // namespace cminor.parsing

#endif // StdLib_hpp_18303
