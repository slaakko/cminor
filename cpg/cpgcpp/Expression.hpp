#ifndef Expression_hpp_5397
#define Expression_hpp_5397

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/pom/Expression.hpp>

namespace cpg { namespace cpp {

class ExpressionGrammar : public cminor::parsing::Grammar
{
public:
    static ExpressionGrammar* Create();
    static ExpressionGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    cminor::pom::CppObject* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName);
private:
    ExpressionGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class ExpressionRule;
    class ConstantExpressionRule;
    class AssignmentExpressionRule;
    class AssingmentOpRule;
    class ThrowExpressionRule;
    class ConditionalExpressionRule;
    class LogicalOrExpressionRule;
    class LogicalAndExpressionRule;
    class InclusiveOrExpressionRule;
    class ExclusiveOrExpressionRule;
    class AndExpressionRule;
    class EqualityExpressionRule;
    class EqOpRule;
    class RelationalExpressionRule;
    class RelOpRule;
    class ShiftExpressionRule;
    class ShiftOpRule;
    class AdditiveExpressionRule;
    class AddOpRule;
    class MultiplicativeExpressionRule;
    class MulOpRule;
    class PmExpressionRule;
    class PmOpRule;
    class CastExpressionRule;
    class UnaryExpressionRule;
    class UnaryOperatorRule;
    class NewExpressionRule;
    class NewTypeIdRule;
    class NewDeclaratorRule;
    class DirectNewDeclaratorRule;
    class NewPlacementRule;
    class NewInitializerRule;
    class DeleteExpressionRule;
    class PostfixExpressionRule;
    class TypeSpecifierOrTypeNameRule;
    class PostCastExpressionRule;
    class ExpressionListRule;
    class PrimaryExpressionRule;
    class IdExpressionRule;
};

} } // namespace cpg.cpp

#endif // Expression_hpp_5397
