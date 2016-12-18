#ifndef Specifier_hpp_345
#define Specifier_hpp_345

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/ast/Specifier.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class SpecifierGrammar : public cminor::parsing::Grammar
{
public:
    static SpecifierGrammar* Create();
    static SpecifierGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    Specifiers Parse(const char* start, const char* end, int fileIndex, const std::string& fileName);
private:
    SpecifierGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class SpecifiersRule;
    class SpecifierRule;
};

} } // namespace cminor.parser

#endif // Specifier_hpp_345
