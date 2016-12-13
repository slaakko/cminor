#ifndef Interface_hpp_1192
#define Interface_hpp_1192

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/ast/Interface.hpp>
#include <cminor/parser/ParsingContext.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class InterfaceGrammar : public cminor::parsing::Grammar
{
public:
    static InterfaceGrammar* Create();
    static InterfaceGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    InterfaceNode* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
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

#endif // Interface_hpp_1192
