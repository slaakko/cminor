#ifndef Template_hpp_7033
#define Template_hpp_7033

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/parser/ParsingContext.hpp>
#include <cminor/ast/Template.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class TemplateGrammar : public cminor::parsing::Grammar
{
public:
    static TemplateGrammar* Create();
    static TemplateGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    Node* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    TemplateGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class TemplateIdRule;
    class TemplateParameterRule;
    class TemplateParameterListRule;
};

} } // namespace cminor.parser

#endif // Template_hpp_7033
