#ifndef Expression_hpp_21399
#define Expression_hpp_21399

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/ast/Expression.hpp>
#include <cminor/parser/ParsingContext.hpp>
#include <cminor/parser/TypeExpr.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class ExpressionGrammar : public cminor::parsing::Grammar
{
public:
    static ExpressionGrammar* Create();
    static ExpressionGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    Node* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    ExpressionGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class ExpressionRule;
    class DisjunctionRule;
    class ConjunctionRule;
    class BitOrRule;
    class BitXorRule;
    class BitAndRule;
    class EqualityRule;
    class RelationalRule;
    class ShiftRule;
    class AdditiveRule;
    class MultiplicativeRule;
    class PrefixRule;
    class PostfixRule;
    class PrimaryRule;
    class CastExprRule;
    class NewExprRule;
    class ArgumentListRule;
    class ExpressionListRule;
};

} } // namespace cminor.parser

#endif // Expression_hpp_21399
