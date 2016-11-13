#ifndef CompileUnit_hpp_20574
#define CompileUnit_hpp_20574

#include <Cm.Parsing/Grammar.hpp>
#include <Cm.Parsing/Keyword.hpp>
#include <cminor/ast/CompileUnit.hpp>
#include <cminor/parser/ParsingContext.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class CompileUnitGrammar : public Cm::Parsing::Grammar
{
public:
    static CompileUnitGrammar* Create();
    static CompileUnitGrammar* Create(Cm::Parsing::ParsingDomain* parsingDomain);
    CompileUnitNode* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    CompileUnitGrammar(Cm::Parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class CompileUnitRule;
    class NamespaceContentRule;
    class UsingDirectivesRule;
    class UsingDirectiveRule;
    class UsingAliasDirectiveRule;
    class UsingNamespaceDirectiveRule;
    class DefinitionsRule;
    class DefinitionRule;
    class NamespaceDefinitionRule;
    class FunctionDefinitionRule;
    class ClassDefinitionRule;
    class InterfaceDefinitionRule;
};

} } // namespace cminor.parser

#endif // CompileUnit_hpp_20574
