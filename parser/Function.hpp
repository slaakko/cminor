#ifndef Function_hpp_3168
#define Function_hpp_3168

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/ast/Function.hpp>
#include <cminor/parser/ParsingContext.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class FunctionGrammar : public cminor::parsing::Grammar
{
public:
    static FunctionGrammar* Create();
    static FunctionGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    FunctionNode* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    FunctionGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
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

#endif // Function_hpp_3168
