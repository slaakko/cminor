#ifndef Parameter_hpp_1192
#define Parameter_hpp_1192

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/ast/Parameter.hpp>
#include <cminor/parser/ParsingContext.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class ParameterGrammar : public cminor::parsing::Grammar
{
public:
    static ParameterGrammar* Create();
    static ParameterGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    void Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx, Node* owner);
private:
    ParameterGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class ParameterListRule;
    class ParameterRule;
};

} } // namespace cminor.parser

#endif // Parameter_hpp_1192
