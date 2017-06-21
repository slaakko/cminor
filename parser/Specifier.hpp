#ifndef Specifier_hpp_21399
#define Specifier_hpp_21399

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/ast/Specifier.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class SpecifierGrammar : public cminor::parsing::Grammar
{
public:
    static SpecifierGrammar* Create();
    static SpecifierGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    Specifiers Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName);
private:
    SpecifierGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class SpecifiersRule;
    class SpecifierRule;
};

} } // namespace cminor.parser

#endif // Specifier_hpp_21399
