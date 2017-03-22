#ifndef Function_hpp_9318
#define Function_hpp_9318

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/ast/Function.hpp>
#include <cminor/parser/ParsingContext.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class FunctionGrammar : public cminor::parsing::Grammar
{
public:
    static FunctionGrammar* Create();
    static FunctionGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    FunctionNode* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
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

#endif // Function_hpp_9318
