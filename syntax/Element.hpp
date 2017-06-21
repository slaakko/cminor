#ifndef Element_hpp_20364
#define Element_hpp_20364

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/parsing/Scope.hpp>
#include <cminor/parsing/Grammar.hpp>

namespace cminor { namespace syntax {

class ElementGrammar : public cminor::parsing::Grammar
{
public:
    static ElementGrammar* Create();
    static ElementGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    void Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, cminor::parsing::Grammar* grammar);
private:
    std::vector<std::u32string> keywords0;
    ElementGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class RuleLinkRule;
    class SignatureRule;
    class ParameterListRule;
    class VariableRule;
    class ParameterRule;
    class ReturnTypeRule;
    class IdentifierRule;
    class QualifiedIdRule;
    class StringArrayRule;
};

} } // namespace cminor.syntax

#endif // Element_hpp_20364
