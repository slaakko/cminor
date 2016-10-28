#ifndef Specifier_hpp_9727
#define Specifier_hpp_9727

#include <Cm.Parsing/Grammar.hpp>
#include <Cm.Parsing/Keyword.hpp>
#include <cminor/ast/Specifier.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class SpecifierGrammar : public Cm::Parsing::Grammar
{
public:
    static SpecifierGrammar* Create();
    static SpecifierGrammar* Create(Cm::Parsing::ParsingDomain* parsingDomain);
    Specifiers Parse(const char* start, const char* end, int fileIndex, const std::string& fileName);
private:
    SpecifierGrammar(Cm::Parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class SpecifiersRule;
    class SpecifierRule;
};

} } // namespace cminor.parser

#endif // Specifier_hpp_9727
