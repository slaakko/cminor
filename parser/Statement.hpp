#ifndef Statement_hpp_23654
#define Statement_hpp_23654

#include <Cm.Parsing/Grammar.hpp>
#include <Cm.Parsing/Keyword.hpp>
#include <cminor/ast/Statement.hpp>
#include <cminor/parser/ParsingContext.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class StatementGrammar : public Cm::Parsing::Grammar
{
public:
    static StatementGrammar* Create();
    static StatementGrammar* Create(Cm::Parsing::ParsingDomain* parsingDomain);
    StatementNode* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    StatementGrammar(Cm::Parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class StatementRule;
    class LabelIdRule;
    class LabelRule;
    class LabeledStatementRule;
    class ControlStatementRule;
    class CompoundStatementRule;
    class ReturnStatementRule;
    class IfStatementRule;
    class WhileStatementRule;
    class DoStatementRule;
    class ForStatementRule;
    class ForInitStatementRule;
    class ForLoopExpressionStatementRule;
    class ForEachStatementRule;
    class BreakStatementRule;
    class ContinueStatementRule;
    class SwitchStatementRule;
    class CaseStatementRule;
    class DefaultStatementRule;
    class GotoCaseStatementRule;
    class GotoDefaultStatementRule;
    class AssignmentExpressionStatementRule;
    class AssignmentStatementRule;
    class ConstructionStatementRule;
    class IncrementExpressionStatementRule;
    class IncrementStatementRule;
    class DecrementExpressionStatementRule;
    class DecrementStatementRule;
    class ExpressionStatementRule;
    class EmptyStatementRule;
    class ThrowStatementRule;
    class TryStatementRule;
    class CatchRule;
    class FinallyRule;
};

} } // namespace cminor.parser

#endif // Statement_hpp_23654
