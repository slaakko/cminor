#ifndef Statement_hpp_19453
#define Statement_hpp_19453

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/codedom/Statement.hpp>

namespace cminor { namespace code {

class StatementGrammar : public cminor::parsing::Grammar
{
public:
    static StatementGrammar* Create();
    static StatementGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    cminor::codedom::CompoundStatement* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName);
private:
    StatementGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class StatementRule;
    class LabeledStatementRule;
    class LabelRule;
    class EmptyStatementRule;
    class ExpressionStatementRule;
    class CompoundStatementRule;
    class SelectionStatementRule;
    class IfStatementRule;
    class SwitchStatementRule;
    class IterationStatementRule;
    class WhileStatementRule;
    class DoStatementRule;
    class ForStatementRule;
    class ForInitStatementRule;
    class JumpStatementRule;
    class BreakStatementRule;
    class ContinueStatementRule;
    class ReturnStatementRule;
    class GotoStatementRule;
    class GotoTargetRule;
    class DeclarationStatementRule;
    class ConditionRule;
    class TryStatementRule;
    class HandlerSeqRule;
    class HandlerRule;
    class ExceptionDeclarationRule;
};

} } // namespace cminor.code

#endif // Statement_hpp_19453
