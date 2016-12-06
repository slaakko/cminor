#ifndef Enumeration_hpp_19131
#define Enumeration_hpp_19131

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/ast/Enumeration.hpp>
#include <cminor/parser/ParsingContext.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class EnumerationGrammar : public cminor::parsing::Grammar
{
public:
    static EnumerationGrammar* Create();
    static EnumerationGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    EnumTypeNode* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    EnumerationGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class EnumTypeRule;
    class UnderlyingTypeRule;
    class EnumConstantsRule;
    class EnumConstantRule;
};

} } // namespace cminor.parser

#endif // Enumeration_hpp_19131
