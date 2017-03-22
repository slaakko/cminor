#ifndef CompileUnit_hpp_7033
#define CompileUnit_hpp_7033

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/ast/CompileUnit.hpp>
#include <cminor/parser/ParsingContext.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class CompileUnitGrammar : public cminor::parsing::Grammar
{
public:
    static CompileUnitGrammar* Create();
    static CompileUnitGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    CompileUnitNode* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    CompileUnitGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
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
    class EnumTypeDefinitionRule;
    class ConstantDefinitionRule;
    class DelegateDefinitionRule;
    class ClassDelegateDefinitionRule;
};

} } // namespace cminor.parser

#endif // CompileUnit_hpp_7033
