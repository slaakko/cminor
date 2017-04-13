#ifndef Element_hpp_31289
#define Element_hpp_31289

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/pl/Scope.hpp>
#include <cminor/pl/Grammar.hpp>

namespace cpg { namespace syntax {

class ElementGrammar : public cminor::parsing::Grammar
{
public:
    static ElementGrammar* Create();
    static ElementGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    void Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, cminor::parsing::Grammar* grammar);
private:
    std::vector<std::string> keywords0;
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

} } // namespace cpg.syntax

#endif // Element_hpp_31289
