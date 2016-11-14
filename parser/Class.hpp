#ifndef Class_hpp_21394
#define Class_hpp_21394

#include <Cm.Parsing/Grammar.hpp>
#include <Cm.Parsing/Keyword.hpp>
#include <cminor/ast/Class.hpp>
#include <cminor/parser/ParsingContext.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class ClassGrammar : public Cm::Parsing::Grammar
{
public:
    static ClassGrammar* Create();
    static ClassGrammar* Create(Cm::Parsing::ParsingDomain* parsingDomain);
    ClassNode* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    ClassGrammar(Cm::Parsing::ParsingDomain* parsingDomain_);
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

#endif // Class_hpp_21394
