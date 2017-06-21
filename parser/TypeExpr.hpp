#ifndef TypeExpr_hpp_21399
#define TypeExpr_hpp_21399

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/ast/Expression.hpp>
#include <cminor/parser/ParsingContext.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class TypeExprGrammar : public cminor::parsing::Grammar
{
public:
    static TypeExprGrammar* Create();
    static TypeExprGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    Node* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    TypeExprGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class TypeExprRule;
    class PrefixTypeExprRule;
    class PostfixTypeExprRule;
    class PrimaryTypeExprRule;
};

} } // namespace cminor.parser

#endif // TypeExpr_hpp_21399
