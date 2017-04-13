#ifndef StdLib_hpp_1319
#define StdLib_hpp_1319

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
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
    class hexRule;
    class hex_literalRule;
    class realRule;
    class urealRule;
    class numberRule;
    class boolRule;
    class identifierRule;
    class qualified_idRule;
    class escapeRule;
    class charRule;
    class stringRule;
};

} } // namespace cminor.parsing

#endif // StdLib_hpp_1319
