#ifndef Class_hpp_11865
#define Class_hpp_11865

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/ast/Class.hpp>
#include <cminor/parser/ParsingContext.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class ClassGrammar : public cminor::parsing::Grammar
{
public:
    static ClassGrammar* Create();
    static ClassGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    ClassNode* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    ClassGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class ClassRule;
    class InheritanceAndInterfacesRule;
    class BaseClassOrInterfaceRule;
    class ClassContentRule;
    class ClassMemberRule;
    class StaticConstructorRule;
    class ConstructorRule;
    class InitializerRule;
    class MemberFunctionRule;
    class MemberVariableRule;
    class PropertyRule;
    class IndexerRule;
    class GetterOrSetterRule;
};

} } // namespace cminor.parser

#endif // Class_hpp_11865
