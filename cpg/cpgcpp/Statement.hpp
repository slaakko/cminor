#ifndef Statement_hpp_5397
#define Statement_hpp_5397

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/pom/Statement.hpp>

namespace cpg { namespace cpp {

class StatementGrammar : public cminor::parsing::Grammar
{
public:
    static StatementGrammar* Create();
    static StatementGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    cminor::pom::CompoundStatement* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName);
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

} } // namespace cpg.cpp

#endif // Statement_hpp_5397
