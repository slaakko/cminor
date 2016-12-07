#ifndef Declarator_hpp_20441
#define Declarator_hpp_20441

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/pom/Declaration.hpp>

namespace cpg { namespace cpp {

class DeclaratorGrammar : public cminor::parsing::Grammar
{
public:
    static DeclaratorGrammar* Create();
    static DeclaratorGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    cminor::pom::InitDeclaratorList* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName);
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

} } // namespace cpg.cpp

#endif // Declarator_hpp_20441
