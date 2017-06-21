#ifndef Declaration_hpp_19449
#define Declaration_hpp_19449

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/codedom/Declaration.hpp>

namespace cminor { namespace code {

class DeclarationGrammar : public cminor::parsing::Grammar
{
public:
    static DeclarationGrammar* Create();
    static DeclarationGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    cminor::codedom::CppObject* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName);
private:
    std::vector<std::u32string> keywords0;
    std::vector<std::u32string> keywords1;
    DeclarationGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class BlockDeclarationRule;
    class SimpleDeclarationRule;
    class DeclSpecifierSeqRule;
    class DeclSpecifierRule;
    class StorageClassSpecifierRule;
    class TypeSpecifierRule;
    class SimpleTypeSpecifierRule;
    class TypeNameRule;
    class TemplateArgumentListRule;
    class TemplateArgumentRule;
    class TypedefRule;
    class CVQualifierRule;
    class NamespaceAliasDefinitionRule;
    class UsingDeclarationRule;
    class UsingDirectiveRule;
};

} } // namespace cminor.code

#endif // Declaration_hpp_19449
