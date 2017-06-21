#ifndef CompileUnit_hpp_21399
#define CompileUnit_hpp_21399

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/ast/CompileUnit.hpp>
#include <cminor/parser/ParsingContext.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class CompileUnitGrammar : public cminor::parsing::Grammar
{
public:
    static CompileUnitGrammar* Create();
    static CompileUnitGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    CompileUnitNode* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
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

#endif // CompileUnit_hpp_21399
