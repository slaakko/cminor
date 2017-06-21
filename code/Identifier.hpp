#ifndef Identifier_hpp_19453
#define Identifier_hpp_19453

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>

namespace cminor { namespace code {

class IdentifierGrammar : public cminor::parsing::Grammar
{
public:
    static IdentifierGrammar* Create();
    static IdentifierGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    std::u32string Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName);
private:
    IdentifierGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class IdentifierRule;
    class QualifiedIdRule;
};

} } // namespace cminor.code

#endif // Identifier_hpp_19453
