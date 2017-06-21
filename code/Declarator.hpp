#ifndef Declarator_hpp_19449
#define Declarator_hpp_19449

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/codedom/Declaration.hpp>

namespace cminor { namespace code {

class DeclaratorGrammar : public cminor::parsing::Grammar
{
public:
    static DeclaratorGrammar* Create();
    static DeclaratorGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    cminor::codedom::InitDeclaratorList* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName);
private:
    DeclaratorGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class InitDeclaratorListRule;
    class InitDeclaratorRule;
    class DeclaratorRule;
    class DirectDeclaratorRule;
    class DeclaratorIdRule;
    class TypeIdRule;
    class TypeRule;
    class TypeSpecifierSeqRule;
    class AbstractDeclaratorRule;
    class DirectAbstractDeclaratorRule;
    class CVQualifierSeqRule;
    class InitializerRule;
    class InitializerClauseRule;
    class InitializerListRule;
};

} } // namespace cminor.code

#endif // Declarator_hpp_19449
