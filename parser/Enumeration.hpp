#ifndef Enumeration_hpp_21399
#define Enumeration_hpp_21399

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/ast/Enumeration.hpp>
#include <cminor/parser/ParsingContext.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class EnumerationGrammar : public cminor::parsing::Grammar
{
public:
    static EnumerationGrammar* Create();
    static EnumerationGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    EnumTypeNode* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
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

#endif // Enumeration_hpp_21399
