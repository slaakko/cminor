#ifndef Interface_hpp_30906
#define Interface_hpp_30906

#include <Cm.Parsing/Grammar.hpp>
#include <Cm.Parsing/Keyword.hpp>
#include <cminor/ast/Interface.hpp>
#include <cminor/parser/ParsingContext.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class InterfaceGrammar : public Cm::Parsing::Grammar
{
public:
    static InterfaceGrammar* Create();
    static InterfaceGrammar* Create(Cm::Parsing::ParsingDomain* parsingDomain);
    InterfaceNode* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    InterfaceGrammar(Cm::Parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class InterfaceRule;
    class InterfaceContentRule;
    class InterfaceMemFunRule;
    class InterfaceFunctionGroupIdRule;
};

} } // namespace cminor.parser

#endif // Interface_hpp_30906
