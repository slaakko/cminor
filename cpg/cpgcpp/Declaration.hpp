#ifndef Declaration_hpp_20441
#define Declaration_hpp_20441

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/pom/Declaration.hpp>

namespace cpg { namespace cpp {

class DeclarationGrammar : public cminor::parsing::Grammar
{
public:
    static DeclarationGrammar* Create();
    static DeclarationGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    cminor::pom::CppObject* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName);
private:
    std::vector<std::string> keywords0;
    std::vector<std::string> keywords1;
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

} } // namespace cpg.cpp

#endif // Declaration_hpp_20441
