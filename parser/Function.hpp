#ifndef Function_hpp_28726
#define Function_hpp_28726

#include <Cm.Parsing/Grammar.hpp>
#include <Cm.Parsing/Keyword.hpp>
#include <cminor/ast/Function.hpp>
#include <cminor/parser/ParsingContext.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class FunctionGrammar : public Cm::Parsing::Grammar
{
public:
    static FunctionGrammar* Create();
    static FunctionGrammar* Create(Cm::Parsing::ParsingDomain* parsingDomain);
    FunctionNode* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    FunctionGrammar(Cm::Parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class FunctionRule;
    class FunctionGroupIdRule;
    class OperatorFunctionGroupIdRule;
    class AttributesRule;
    class NameValuePairRule;
    class NameRule;
    class ValueRule;
};

} } // namespace cminor.parser

#endif // Function_hpp_28726
