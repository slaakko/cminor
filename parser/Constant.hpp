#ifndef Constant_hpp_21399
#define Constant_hpp_21399

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/ast/Constant.hpp>
#include <cminor/parser/ParsingContext.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class ConstantGrammar : public cminor::parsing::Grammar
{
public:
    static ConstantGrammar* Create();
    static ConstantGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    ConstantNode* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    ConstantGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class ConstantRule;
};

} } // namespace cminor.parser

#endif // Constant_hpp_21399
