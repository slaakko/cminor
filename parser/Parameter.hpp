#ifndef Parameter_hpp_18908
#define Parameter_hpp_18908

#include <Cm.Parsing/Grammar.hpp>
#include <Cm.Parsing/Keyword.hpp>
#include <cminor/ast/Parameter.hpp>
#include <cminor/parser/ParsingContext.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class ParameterGrammar : public Cm::Parsing::Grammar
{
public:
    static ParameterGrammar* Create();
    static ParameterGrammar* Create(Cm::Parsing::ParsingDomain* parsingDomain);
    void Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx, Node* owner);
private:
    ParameterGrammar(Cm::Parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class ParameterListRule;
    class ParameterRule;
};

} } // namespace cminor.parser

#endif // Parameter_hpp_18908
