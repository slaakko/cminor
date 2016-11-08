#ifndef Template_hpp_641
#define Template_hpp_641

#include <Cm.Parsing/Grammar.hpp>
#include <Cm.Parsing/Keyword.hpp>
#include <cminor/parser/ParsingContext.hpp>
#include <cminor/ast/Template.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class TemplateGrammar : public Cm::Parsing::Grammar
{
public:
    static TemplateGrammar* Create();
    static TemplateGrammar* Create(Cm::Parsing::ParsingDomain* parsingDomain);
    Node* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    TemplateGrammar(Cm::Parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class TemplateIdRule;
    class TemplateParameterRule;
    class TemplateParameterListRule;
};

} } // namespace cminor.parser

#endif // Template_hpp_641
