#ifndef Interface_hpp_21399
#define Interface_hpp_21399

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/ast/Interface.hpp>
#include <cminor/parser/ParsingContext.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class InterfaceGrammar : public cminor::parsing::Grammar
{
public:
    static InterfaceGrammar* Create();
    static InterfaceGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    InterfaceNode* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    InterfaceGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class InterfaceRule;
    class InterfaceContentRule;
    class InterfaceMemFunRule;
    class InterfaceFunctionGroupIdRule;
};

} } // namespace cminor.parser

#endif // Interface_hpp_21399
