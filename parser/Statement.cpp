#include "Statement.hpp"
#include <cminor/pl/Action.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/StdLib.hpp>
#include <cminor/pl/XmlLog.hpp>
#include <cminor/parser/Keyword.hpp>
#include <cminor/parser/Expression.hpp>
#include <cminor/parser/TypeExpr.hpp>
#include <cminor/parser/Identifier.hpp>

namespace cminor { namespace parser {

using namespace cminor::parsing;

StatementGrammar* StatementGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

StatementGrammar* StatementGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    StatementGrammar* grammar(new StatementGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

StatementGrammar::StatementGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("StatementGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

StatementNode* StatementGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
{
    cminor::parsing::Scanner scanner(start, end, fileName, fileIndex, SkipRule());
    std::unique_ptr<cminor::parsing::XmlLog> xmlLog;
    if (Log())
    {
        xmlLog.reset(new cminor::parsing::XmlLog(*Log(), MaxLogLineLength()));
        scanner.SetLog(xmlLog.get());
        xmlLog->WriteBeginRule("parse");
    }
    cminor::parsing::ObjectStack stack;
    stack.push(std::unique_ptr<cminor::parsing::Object>(new ValueObject<ParsingContext*>(ctx)));
    cminor::parsing::Match match = cminor::parsing::Grammar::Parse(scanner, stack);
    cminor::parsing::Span stop = scanner.GetSpan();
    if (Log())
    {
        xmlLog->WriteEndRule("parse");
    }
    if (!match.Hit() || !CC() && stop.Start() != int(end - start))
    {
        if (StartRule())
        {
            throw cminor::parsing::ExpectationFailure(StartRule()->Info(), fileName, stop, start, end);
        }
        else
        {
            throw cminor::parsing::ParsingException("grammar '" + Name() + "' has no start rule", fileName, scanner.GetSpan(), start, end);
        }
    }
    std::unique_ptr<cminor::parsing::Object> value = std::move(stack.top());
    StatementNode* result = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(value.get());
    stack.pop();
    return result;
}

class StatementGrammar::StatementRule : public cminor::parsing::Rule
{
public:
    StatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction("A6");
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A6Action));
        cminor::parsing::ActionParser* a7ActionParser = GetAction("A7");
        a7ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A7Action));
        cminor::parsing::ActionParser* a8ActionParser = GetAction("A8");
        a8ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A8Action));
        cminor::parsing::ActionParser* a9ActionParser = GetAction("A9");
        a9ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A9Action));
        cminor::parsing::ActionParser* a10ActionParser = GetAction("A10");
        a10ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A10Action));
        cminor::parsing::ActionParser* a11ActionParser = GetAction("A11");
        a11ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A11Action));
        cminor::parsing::NonterminalParser* labeledStatementNonterminalParser = GetNonterminal("LabeledStatement");
        labeledStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreLabeledStatement));
        labeledStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostLabeledStatement));
        cminor::parsing::NonterminalParser* controlStatementNonterminalParser = GetNonterminal("ControlStatement");
        controlStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreControlStatement));
        controlStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostControlStatement));
        cminor::parsing::NonterminalParser* incrementStatementNonterminalParser = GetNonterminal("IncrementStatement");
        incrementStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreIncrementStatement));
        incrementStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostIncrementStatement));
        cminor::parsing::NonterminalParser* decrementStatementNonterminalParser = GetNonterminal("DecrementStatement");
        decrementStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreDecrementStatement));
        decrementStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostDecrementStatement));
        cminor::parsing::NonterminalParser* expressionStatementNonterminalParser = GetNonterminal("ExpressionStatement");
        expressionStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreExpressionStatement));
        expressionStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostExpressionStatement));
        cminor::parsing::NonterminalParser* assignmentStatementNonterminalParser = GetNonterminal("AssignmentStatement");
        assignmentStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreAssignmentStatement));
        assignmentStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostAssignmentStatement));
        cminor::parsing::NonterminalParser* constructionStatementNonterminalParser = GetNonterminal("ConstructionStatement");
        constructionStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreConstructionStatement));
        constructionStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostConstructionStatement));
        cminor::parsing::NonterminalParser* emptyStatementNonterminalParser = GetNonterminal("EmptyStatement");
        emptyStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreEmptyStatement));
        emptyStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostEmptyStatement));
        cminor::parsing::NonterminalParser* throwStatementNonterminalParser = GetNonterminal("ThrowStatement");
        throwStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreThrowStatement));
        throwStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostThrowStatement));
        cminor::parsing::NonterminalParser* tryStatementNonterminalParser = GetNonterminal("TryStatement");
        tryStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreTryStatement));
        tryStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostTryStatement));
        cminor::parsing::NonterminalParser* usingStatementNonterminalParser = GetNonterminal("UsingStatement");
        usingStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreUsingStatement));
        usingStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostUsingStatement));
        cminor::parsing::NonterminalParser* lockStatementNonterminalParser = GetNonterminal("LockStatement");
        lockStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreLockStatement));
        lockStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostLockStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromLabeledStatement;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromControlStatement;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromIncrementStatement;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromDecrementStatement;
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromExpressionStatement;
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromAssignmentStatement;
    }
    void A6Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromConstructionStatement;
    }
    void A7Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromEmptyStatement;
    }
    void A8Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromThrowStatement;
    }
    void A9Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromTryStatement;
    }
    void A10Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromUsingStatement;
    }
    void A11Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromLockStatement;
    }
    void PreLabeledStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostLabeledStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromLabeledStatement_value = std::move(stack.top());
            context.fromLabeledStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromLabeledStatement_value.get());
            stack.pop();
        }
    }
    void PreControlStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostControlStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromControlStatement_value = std::move(stack.top());
            context.fromControlStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromControlStatement_value.get());
            stack.pop();
        }
    }
    void PreIncrementStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostIncrementStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIncrementStatement_value = std::move(stack.top());
            context.fromIncrementStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromIncrementStatement_value.get());
            stack.pop();
        }
    }
    void PreDecrementStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostDecrementStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDecrementStatement_value = std::move(stack.top());
            context.fromDecrementStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromDecrementStatement_value.get());
            stack.pop();
        }
    }
    void PreExpressionStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpressionStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpressionStatement_value = std::move(stack.top());
            context.fromExpressionStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromExpressionStatement_value.get());
            stack.pop();
        }
    }
    void PreAssignmentStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostAssignmentStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAssignmentStatement_value = std::move(stack.top());
            context.fromAssignmentStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromAssignmentStatement_value.get());
            stack.pop();
        }
    }
    void PreConstructionStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostConstructionStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromConstructionStatement_value = std::move(stack.top());
            context.fromConstructionStatement = *static_cast<cminor::parsing::ValueObject<ConstructionStatementNode*>*>(fromConstructionStatement_value.get());
            stack.pop();
        }
    }
    void PreEmptyStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostEmptyStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromEmptyStatement_value = std::move(stack.top());
            context.fromEmptyStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromEmptyStatement_value.get());
            stack.pop();
        }
    }
    void PreThrowStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostThrowStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromThrowStatement_value = std::move(stack.top());
            context.fromThrowStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromThrowStatement_value.get());
            stack.pop();
        }
    }
    void PreTryStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostTryStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTryStatement_value = std::move(stack.top());
            context.fromTryStatement = *static_cast<cminor::parsing::ValueObject<TryStatementNode*>*>(fromTryStatement_value.get());
            stack.pop();
        }
    }
    void PreUsingStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostUsingStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromUsingStatement_value = std::move(stack.top());
            context.fromUsingStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromUsingStatement_value.get());
            stack.pop();
        }
    }
    void PreLockStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostLockStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromLockStatement_value = std::move(stack.top());
            context.fromLockStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromLockStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromLabeledStatement(), fromControlStatement(), fromIncrementStatement(), fromDecrementStatement(), fromExpressionStatement(), fromAssignmentStatement(), fromConstructionStatement(), fromEmptyStatement(), fromThrowStatement(), fromTryStatement(), fromUsingStatement(), fromLockStatement() {}
        ParsingContext* ctx;
        StatementNode* value;
        StatementNode* fromLabeledStatement;
        StatementNode* fromControlStatement;
        StatementNode* fromIncrementStatement;
        StatementNode* fromDecrementStatement;
        StatementNode* fromExpressionStatement;
        StatementNode* fromAssignmentStatement;
        ConstructionStatementNode* fromConstructionStatement;
        StatementNode* fromEmptyStatement;
        StatementNode* fromThrowStatement;
        TryStatementNode* fromTryStatement;
        StatementNode* fromUsingStatement;
        StatementNode* fromLockStatement;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::LabelIdRule : public cminor::parsing::Rule
{
public:
    LabelIdRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("std::string");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<std::string>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LabelIdRule>(this, &LabelIdRule::A0Action));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LabelIdRule>(this, &LabelIdRule::Postidentifier));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromidentifier;
    }
    void Postidentifier(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromidentifier_value = std::move(stack.top());
            context.fromidentifier = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromidentifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromidentifier() {}
        std::string value;
        std::string fromidentifier;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::LabelRule : public cminor::parsing::Rule
{
public:
    LabelRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("LabelNode*");
        AddLocalVariable(AttrOrVariable("std::string", "label"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<LabelNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LabelRule>(this, &LabelRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LabelRule>(this, &LabelRule::A1Action));
        cminor::parsing::NonterminalParser* labelIdNonterminalParser = GetNonterminal("LabelId");
        labelIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LabelRule>(this, &LabelRule::PostLabelId));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new LabelNode(span, context.label);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.label = context.fromLabelId;
    }
    void PostLabelId(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromLabelId_value = std::move(stack.top());
            context.fromLabelId = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromLabelId_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), label(), fromLabelId() {}
        LabelNode* value;
        std::string label;
        std::string fromLabelId;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::LabeledStatementRule : public cminor::parsing::Rule
{
public:
    LabeledStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LabeledStatementRule>(this, &LabeledStatementRule::A0Action));
        cminor::parsing::NonterminalParser* lblNonterminalParser = GetNonterminal("lbl");
        lblNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LabeledStatementRule>(this, &LabeledStatementRule::Postlbl));
        cminor::parsing::NonterminalParser* stmtNonterminalParser = GetNonterminal("stmt");
        stmtNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<LabeledStatementRule>(this, &LabeledStatementRule::Prestmt));
        stmtNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LabeledStatementRule>(this, &LabeledStatementRule::Poststmt));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.fromstmt->SetLabelNode(context.fromlbl);
        context.value = context.fromstmt;
    }
    void Postlbl(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromlbl_value = std::move(stack.top());
            context.fromlbl = *static_cast<cminor::parsing::ValueObject<LabelNode*>*>(fromlbl_value.get());
            stack.pop();
        }
    }
    void Prestmt(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Poststmt(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromstmt_value = std::move(stack.top());
            context.fromstmt = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromstmt_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromlbl(), fromstmt() {}
        ParsingContext* ctx;
        StatementNode* value;
        LabelNode* fromlbl;
        StatementNode* fromstmt;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::ControlStatementRule : public cminor::parsing::Rule
{
public:
    ControlStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction("A6");
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A6Action));
        cminor::parsing::ActionParser* a7ActionParser = GetAction("A7");
        a7ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A7Action));
        cminor::parsing::ActionParser* a8ActionParser = GetAction("A8");
        a8ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A8Action));
        cminor::parsing::ActionParser* a9ActionParser = GetAction("A9");
        a9ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A9Action));
        cminor::parsing::ActionParser* a10ActionParser = GetAction("A10");
        a10ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A10Action));
        cminor::parsing::ActionParser* a11ActionParser = GetAction("A11");
        a11ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A11Action));
        cminor::parsing::ActionParser* a12ActionParser = GetAction("A12");
        a12ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A12Action));
        cminor::parsing::NonterminalParser* compoundStatementNonterminalParser = GetNonterminal("CompoundStatement");
        compoundStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreCompoundStatement));
        compoundStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostCompoundStatement));
        cminor::parsing::NonterminalParser* returnStatementNonterminalParser = GetNonterminal("ReturnStatement");
        returnStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreReturnStatement));
        returnStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostReturnStatement));
        cminor::parsing::NonterminalParser* ifStatementNonterminalParser = GetNonterminal("IfStatement");
        ifStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreIfStatement));
        ifStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostIfStatement));
        cminor::parsing::NonterminalParser* whileStatementNonterminalParser = GetNonterminal("WhileStatement");
        whileStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreWhileStatement));
        whileStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostWhileStatement));
        cminor::parsing::NonterminalParser* doStatementNonterminalParser = GetNonterminal("DoStatement");
        doStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreDoStatement));
        doStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostDoStatement));
        cminor::parsing::NonterminalParser* forStatementNonterminalParser = GetNonterminal("ForStatement");
        forStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreForStatement));
        forStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostForStatement));
        cminor::parsing::NonterminalParser* forEachStatementNonterminalParser = GetNonterminal("ForEachStatement");
        forEachStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreForEachStatement));
        forEachStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostForEachStatement));
        cminor::parsing::NonterminalParser* breakStatementNonterminalParser = GetNonterminal("BreakStatement");
        breakStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreBreakStatement));
        breakStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostBreakStatement));
        cminor::parsing::NonterminalParser* continueStatementNonterminalParser = GetNonterminal("ContinueStatement");
        continueStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreContinueStatement));
        continueStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostContinueStatement));
        cminor::parsing::NonterminalParser* gotoStatementNonterminalParser = GetNonterminal("GotoStatement");
        gotoStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreGotoStatement));
        gotoStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostGotoStatement));
        cminor::parsing::NonterminalParser* switchStatementNonterminalParser = GetNonterminal("SwitchStatement");
        switchStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreSwitchStatement));
        switchStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostSwitchStatement));
        cminor::parsing::NonterminalParser* gotoCaseStatementNonterminalParser = GetNonterminal("GotoCaseStatement");
        gotoCaseStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreGotoCaseStatement));
        gotoCaseStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostGotoCaseStatement));
        cminor::parsing::NonterminalParser* gotoDefaultStatementNonterminalParser = GetNonterminal("GotoDefaultStatement");
        gotoDefaultStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreGotoDefaultStatement));
        gotoDefaultStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostGotoDefaultStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromCompoundStatement;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromReturnStatement;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromIfStatement;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromWhileStatement;
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromDoStatement;
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromForStatement;
    }
    void A6Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromForEachStatement;
    }
    void A7Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromBreakStatement;
    }
    void A8Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromContinueStatement;
    }
    void A9Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromGotoStatement;
    }
    void A10Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromSwitchStatement;
    }
    void A11Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromGotoCaseStatement;
    }
    void A12Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromGotoDefaultStatement;
    }
    void PreCompoundStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostCompoundStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCompoundStatement_value = std::move(stack.top());
            context.fromCompoundStatement = *static_cast<cminor::parsing::ValueObject<CompoundStatementNode*>*>(fromCompoundStatement_value.get());
            stack.pop();
        }
    }
    void PreReturnStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostReturnStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromReturnStatement_value = std::move(stack.top());
            context.fromReturnStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromReturnStatement_value.get());
            stack.pop();
        }
    }
    void PreIfStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostIfStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIfStatement_value = std::move(stack.top());
            context.fromIfStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromIfStatement_value.get());
            stack.pop();
        }
    }
    void PreWhileStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostWhileStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromWhileStatement_value = std::move(stack.top());
            context.fromWhileStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromWhileStatement_value.get());
            stack.pop();
        }
    }
    void PreDoStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostDoStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDoStatement_value = std::move(stack.top());
            context.fromDoStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromDoStatement_value.get());
            stack.pop();
        }
    }
    void PreForStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostForStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromForStatement_value = std::move(stack.top());
            context.fromForStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromForStatement_value.get());
            stack.pop();
        }
    }
    void PreForEachStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostForEachStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromForEachStatement_value = std::move(stack.top());
            context.fromForEachStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromForEachStatement_value.get());
            stack.pop();
        }
    }
    void PreBreakStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostBreakStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromBreakStatement_value = std::move(stack.top());
            context.fromBreakStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromBreakStatement_value.get());
            stack.pop();
        }
    }
    void PreContinueStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostContinueStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromContinueStatement_value = std::move(stack.top());
            context.fromContinueStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromContinueStatement_value.get());
            stack.pop();
        }
    }
    void PreGotoStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostGotoStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromGotoStatement_value = std::move(stack.top());
            context.fromGotoStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromGotoStatement_value.get());
            stack.pop();
        }
    }
    void PreSwitchStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostSwitchStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSwitchStatement_value = std::move(stack.top());
            context.fromSwitchStatement = *static_cast<cminor::parsing::ValueObject<SwitchStatementNode*>*>(fromSwitchStatement_value.get());
            stack.pop();
        }
    }
    void PreGotoCaseStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostGotoCaseStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromGotoCaseStatement_value = std::move(stack.top());
            context.fromGotoCaseStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromGotoCaseStatement_value.get());
            stack.pop();
        }
    }
    void PreGotoDefaultStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostGotoDefaultStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromGotoDefaultStatement_value = std::move(stack.top());
            context.fromGotoDefaultStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromGotoDefaultStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromCompoundStatement(), fromReturnStatement(), fromIfStatement(), fromWhileStatement(), fromDoStatement(), fromForStatement(), fromForEachStatement(), fromBreakStatement(), fromContinueStatement(), fromGotoStatement(), fromSwitchStatement(), fromGotoCaseStatement(), fromGotoDefaultStatement() {}
        ParsingContext* ctx;
        StatementNode* value;
        CompoundStatementNode* fromCompoundStatement;
        StatementNode* fromReturnStatement;
        StatementNode* fromIfStatement;
        StatementNode* fromWhileStatement;
        StatementNode* fromDoStatement;
        StatementNode* fromForStatement;
        StatementNode* fromForEachStatement;
        StatementNode* fromBreakStatement;
        StatementNode* fromContinueStatement;
        StatementNode* fromGotoStatement;
        SwitchStatementNode* fromSwitchStatement;
        StatementNode* fromGotoCaseStatement;
        StatementNode* fromGotoDefaultStatement;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::CompoundStatementRule : public cminor::parsing::Rule
{
public:
    CompoundStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("CompoundStatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<CompoundStatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CompoundStatementRule>(this, &CompoundStatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CompoundStatementRule>(this, &CompoundStatementRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CompoundStatementRule>(this, &CompoundStatementRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CompoundStatementRule>(this, &CompoundStatementRule::A3Action));
        cminor::parsing::NonterminalParser* stmtNonterminalParser = GetNonterminal("stmt");
        stmtNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<CompoundStatementRule>(this, &CompoundStatementRule::Prestmt));
        stmtNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CompoundStatementRule>(this, &CompoundStatementRule::Poststmt));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value->GetSpan().SetEnd(span.End());
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new CompoundStatementNode(span);
        context.value->SetBeginBraceSpan(span);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value->AddStatement(context.fromstmt);
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value->SetEndBraceSpan(span);
    }
    void Prestmt(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Poststmt(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromstmt_value = std::move(stack.top());
            context.fromstmt = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromstmt_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromstmt() {}
        ParsingContext* ctx;
        CompoundStatementNode* value;
        StatementNode* fromstmt;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::ReturnStatementRule : public cminor::parsing::Rule
{
public:
    ReturnStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ReturnStatementRule>(this, &ReturnStatementRule::A0Action));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ReturnStatementRule>(this, &ReturnStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ReturnStatementRule>(this, &ReturnStatementRule::PostExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ReturnStatementNode(span, context.fromExpression);
    }
    void PreExpression(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromExpression() {}
        ParsingContext* ctx;
        StatementNode* value;
        Node* fromExpression;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::IfStatementRule : public cminor::parsing::Rule
{
public:
    IfStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IfStatementRule>(this, &IfStatementRule::A0Action));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<IfStatementRule>(this, &IfStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IfStatementRule>(this, &IfStatementRule::PostExpression));
        cminor::parsing::NonterminalParser* thensNonterminalParser = GetNonterminal("thens");
        thensNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<IfStatementRule>(this, &IfStatementRule::Prethens));
        thensNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IfStatementRule>(this, &IfStatementRule::Postthens));
        cminor::parsing::NonterminalParser* elsesNonterminalParser = GetNonterminal("elses");
        elsesNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<IfStatementRule>(this, &IfStatementRule::Preelses));
        elsesNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IfStatementRule>(this, &IfStatementRule::Postelses));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new IfStatementNode(span, context.fromExpression, context.fromthens, context.fromelses);
    }
    void PreExpression(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void Prethens(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postthens(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromthens_value = std::move(stack.top());
            context.fromthens = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromthens_value.get());
            stack.pop();
        }
    }
    void Preelses(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postelses(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromelses_value = std::move(stack.top());
            context.fromelses = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromelses_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromExpression(), fromthens(), fromelses() {}
        ParsingContext* ctx;
        StatementNode* value;
        Node* fromExpression;
        StatementNode* fromthens;
        StatementNode* fromelses;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::WhileStatementRule : public cminor::parsing::Rule
{
public:
    WhileStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<WhileStatementRule>(this, &WhileStatementRule::A0Action));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<WhileStatementRule>(this, &WhileStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<WhileStatementRule>(this, &WhileStatementRule::PostExpression));
        cminor::parsing::NonterminalParser* statementNonterminalParser = GetNonterminal("Statement");
        statementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<WhileStatementRule>(this, &WhileStatementRule::PreStatement));
        statementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<WhileStatementRule>(this, &WhileStatementRule::PostStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new WhileStatementNode(span, context.fromExpression, context.fromStatement);
    }
    void PreExpression(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PreStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromStatement_value = std::move(stack.top());
            context.fromStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromExpression(), fromStatement() {}
        ParsingContext* ctx;
        StatementNode* value;
        Node* fromExpression;
        StatementNode* fromStatement;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::DoStatementRule : public cminor::parsing::Rule
{
public:
    DoStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DoStatementRule>(this, &DoStatementRule::A0Action));
        cminor::parsing::NonterminalParser* statementNonterminalParser = GetNonterminal("Statement");
        statementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DoStatementRule>(this, &DoStatementRule::PreStatement));
        statementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DoStatementRule>(this, &DoStatementRule::PostStatement));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DoStatementRule>(this, &DoStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DoStatementRule>(this, &DoStatementRule::PostExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new DoStatementNode(span, context.fromStatement, context.fromExpression);
    }
    void PreStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromStatement_value = std::move(stack.top());
            context.fromStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromStatement_value.get());
            stack.pop();
        }
    }
    void PreExpression(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromStatement(), fromExpression() {}
        ParsingContext* ctx;
        StatementNode* value;
        StatementNode* fromStatement;
        Node* fromExpression;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::ForStatementRule : public cminor::parsing::Rule
{
public:
    ForStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ForStatementRule>(this, &ForStatementRule::A0Action));
        cminor::parsing::NonterminalParser* forInitStatementNonterminalParser = GetNonterminal("ForInitStatement");
        forInitStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ForStatementRule>(this, &ForStatementRule::PreForInitStatement));
        forInitStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForStatementRule>(this, &ForStatementRule::PostForInitStatement));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ForStatementRule>(this, &ForStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForStatementRule>(this, &ForStatementRule::PostExpression));
        cminor::parsing::NonterminalParser* forLoopExpressionStatementNonterminalParser = GetNonterminal("ForLoopExpressionStatement");
        forLoopExpressionStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ForStatementRule>(this, &ForStatementRule::PreForLoopExpressionStatement));
        forLoopExpressionStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForStatementRule>(this, &ForStatementRule::PostForLoopExpressionStatement));
        cminor::parsing::NonterminalParser* statementNonterminalParser = GetNonterminal("Statement");
        statementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ForStatementRule>(this, &ForStatementRule::PreStatement));
        statementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForStatementRule>(this, &ForStatementRule::PostStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ForStatementNode(span, context.fromForInitStatement, context.fromExpression, context.fromForLoopExpressionStatement, context.fromStatement);
    }
    void PreForInitStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostForInitStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromForInitStatement_value = std::move(stack.top());
            context.fromForInitStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromForInitStatement_value.get());
            stack.pop();
        }
    }
    void PreExpression(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PreForLoopExpressionStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostForLoopExpressionStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromForLoopExpressionStatement_value = std::move(stack.top());
            context.fromForLoopExpressionStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromForLoopExpressionStatement_value.get());
            stack.pop();
        }
    }
    void PreStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromStatement_value = std::move(stack.top());
            context.fromStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromForInitStatement(), fromExpression(), fromForLoopExpressionStatement(), fromStatement() {}
        ParsingContext* ctx;
        StatementNode* value;
        StatementNode* fromForInitStatement;
        Node* fromExpression;
        StatementNode* fromForLoopExpressionStatement;
        StatementNode* fromStatement;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::ForInitStatementRule : public cminor::parsing::Rule
{
public:
    ForInitStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ForInitStatementRule>(this, &ForInitStatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ForInitStatementRule>(this, &ForInitStatementRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ForInitStatementRule>(this, &ForInitStatementRule::A2Action));
        cminor::parsing::NonterminalParser* assignmentStatementNonterminalParser = GetNonterminal("AssignmentStatement");
        assignmentStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ForInitStatementRule>(this, &ForInitStatementRule::PreAssignmentStatement));
        assignmentStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForInitStatementRule>(this, &ForInitStatementRule::PostAssignmentStatement));
        cminor::parsing::NonterminalParser* constructionStatementNonterminalParser = GetNonterminal("ConstructionStatement");
        constructionStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ForInitStatementRule>(this, &ForInitStatementRule::PreConstructionStatement));
        constructionStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForInitStatementRule>(this, &ForInitStatementRule::PostConstructionStatement));
        cminor::parsing::NonterminalParser* emptyStatementNonterminalParser = GetNonterminal("EmptyStatement");
        emptyStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ForInitStatementRule>(this, &ForInitStatementRule::PreEmptyStatement));
        emptyStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForInitStatementRule>(this, &ForInitStatementRule::PostEmptyStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromAssignmentStatement;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromConstructionStatement;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromEmptyStatement;
    }
    void PreAssignmentStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostAssignmentStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAssignmentStatement_value = std::move(stack.top());
            context.fromAssignmentStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromAssignmentStatement_value.get());
            stack.pop();
        }
    }
    void PreConstructionStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostConstructionStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromConstructionStatement_value = std::move(stack.top());
            context.fromConstructionStatement = *static_cast<cminor::parsing::ValueObject<ConstructionStatementNode*>*>(fromConstructionStatement_value.get());
            stack.pop();
        }
    }
    void PreEmptyStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostEmptyStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromEmptyStatement_value = std::move(stack.top());
            context.fromEmptyStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromEmptyStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromAssignmentStatement(), fromConstructionStatement(), fromEmptyStatement() {}
        ParsingContext* ctx;
        StatementNode* value;
        StatementNode* fromAssignmentStatement;
        ConstructionStatementNode* fromConstructionStatement;
        StatementNode* fromEmptyStatement;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::ForLoopExpressionStatementRule : public cminor::parsing::Rule
{
public:
    ForLoopExpressionStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ForLoopExpressionStatementRule>(this, &ForLoopExpressionStatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ForLoopExpressionStatementRule>(this, &ForLoopExpressionStatementRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ForLoopExpressionStatementRule>(this, &ForLoopExpressionStatementRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ForLoopExpressionStatementRule>(this, &ForLoopExpressionStatementRule::A3Action));
        cminor::parsing::NonterminalParser* incrementExpressionStatementNonterminalParser = GetNonterminal("IncrementExpressionStatement");
        incrementExpressionStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ForLoopExpressionStatementRule>(this, &ForLoopExpressionStatementRule::PreIncrementExpressionStatement));
        incrementExpressionStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForLoopExpressionStatementRule>(this, &ForLoopExpressionStatementRule::PostIncrementExpressionStatement));
        cminor::parsing::NonterminalParser* decrementExpressionStatementNonterminalParser = GetNonterminal("DecrementExpressionStatement");
        decrementExpressionStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ForLoopExpressionStatementRule>(this, &ForLoopExpressionStatementRule::PreDecrementExpressionStatement));
        decrementExpressionStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForLoopExpressionStatementRule>(this, &ForLoopExpressionStatementRule::PostDecrementExpressionStatement));
        cminor::parsing::NonterminalParser* assignmentExpressionStatementNonterminalParser = GetNonterminal("AssignmentExpressionStatement");
        assignmentExpressionStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ForLoopExpressionStatementRule>(this, &ForLoopExpressionStatementRule::PreAssignmentExpressionStatement));
        assignmentExpressionStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForLoopExpressionStatementRule>(this, &ForLoopExpressionStatementRule::PostAssignmentExpressionStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromIncrementExpressionStatement;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromDecrementExpressionStatement;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromAssignmentExpressionStatement;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new EmptyStatementNode(span);
    }
    void PreIncrementExpressionStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostIncrementExpressionStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIncrementExpressionStatement_value = std::move(stack.top());
            context.fromIncrementExpressionStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromIncrementExpressionStatement_value.get());
            stack.pop();
        }
    }
    void PreDecrementExpressionStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostDecrementExpressionStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDecrementExpressionStatement_value = std::move(stack.top());
            context.fromDecrementExpressionStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromDecrementExpressionStatement_value.get());
            stack.pop();
        }
    }
    void PreAssignmentExpressionStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostAssignmentExpressionStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAssignmentExpressionStatement_value = std::move(stack.top());
            context.fromAssignmentExpressionStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromAssignmentExpressionStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromIncrementExpressionStatement(), fromDecrementExpressionStatement(), fromAssignmentExpressionStatement() {}
        ParsingContext* ctx;
        StatementNode* value;
        StatementNode* fromIncrementExpressionStatement;
        StatementNode* fromDecrementExpressionStatement;
        StatementNode* fromAssignmentExpressionStatement;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::ForEachStatementRule : public cminor::parsing::Rule
{
public:
    ForEachStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ForEachStatementRule>(this, &ForEachStatementRule::A0Action));
        cminor::parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal("TypeExpr");
        typeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ForEachStatementRule>(this, &ForEachStatementRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForEachStatementRule>(this, &ForEachStatementRule::PostTypeExpr));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForEachStatementRule>(this, &ForEachStatementRule::PostIdentifier));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ForEachStatementRule>(this, &ForEachStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForEachStatementRule>(this, &ForEachStatementRule::PostExpression));
        cminor::parsing::NonterminalParser* compoundStatementNonterminalParser = GetNonterminal("CompoundStatement");
        compoundStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ForEachStatementRule>(this, &ForEachStatementRule::PreCompoundStatement));
        compoundStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForEachStatementRule>(this, &ForEachStatementRule::PostCompoundStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ForEachStatementNode(span, context.fromTypeExpr, context.fromIdentifier, context.fromExpression, context.fromCompoundStatement);
    }
    void PreTypeExpr(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostTypeExpr(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeExpr_value = std::move(stack.top());
            context.fromTypeExpr = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromTypeExpr_value.get());
            stack.pop();
        }
    }
    void PostIdentifier(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIdentifier_value = std::move(stack.top());
            context.fromIdentifier = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
    void PreExpression(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PreCompoundStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostCompoundStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCompoundStatement_value = std::move(stack.top());
            context.fromCompoundStatement = *static_cast<cminor::parsing::ValueObject<CompoundStatementNode*>*>(fromCompoundStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromTypeExpr(), fromIdentifier(), fromExpression(), fromCompoundStatement() {}
        ParsingContext* ctx;
        StatementNode* value;
        Node* fromTypeExpr;
        IdentifierNode* fromIdentifier;
        Node* fromExpression;
        CompoundStatementNode* fromCompoundStatement;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::BreakStatementRule : public cminor::parsing::Rule
{
public:
    BreakStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BreakStatementRule>(this, &BreakStatementRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new BreakStatementNode(span);
    }
private:
    struct Context
    {
        Context(): ctx(), value() {}
        ParsingContext* ctx;
        StatementNode* value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::ContinueStatementRule : public cminor::parsing::Rule
{
public:
    ContinueStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ContinueStatementRule>(this, &ContinueStatementRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ContinueStatementNode(span);
    }
private:
    struct Context
    {
        Context(): ctx(), value() {}
        ParsingContext* ctx;
        StatementNode* value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::GotoStatementRule : public cminor::parsing::Rule
{
public:
    GotoStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<GotoStatementRule>(this, &GotoStatementRule::A0Action));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<GotoStatementRule>(this, &GotoStatementRule::PostIdentifier));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new GotoStatementNode(span, context.fromIdentifier->Str());
    }
    void PostIdentifier(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIdentifier_value = std::move(stack.top());
            context.fromIdentifier = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromIdentifier() {}
        ParsingContext* ctx;
        StatementNode* value;
        IdentifierNode* fromIdentifier;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::SwitchStatementRule : public cminor::parsing::Rule
{
public:
    SwitchStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("SwitchStatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<SwitchStatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SwitchStatementRule>(this, &SwitchStatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SwitchStatementRule>(this, &SwitchStatementRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SwitchStatementRule>(this, &SwitchStatementRule::A2Action));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<SwitchStatementRule>(this, &SwitchStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SwitchStatementRule>(this, &SwitchStatementRule::PostExpression));
        cminor::parsing::NonterminalParser* caseStatementNonterminalParser = GetNonterminal("CaseStatement");
        caseStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<SwitchStatementRule>(this, &SwitchStatementRule::PreCaseStatement));
        caseStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SwitchStatementRule>(this, &SwitchStatementRule::PostCaseStatement));
        cminor::parsing::NonterminalParser* defaultStatementNonterminalParser = GetNonterminal("DefaultStatement");
        defaultStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<SwitchStatementRule>(this, &SwitchStatementRule::PreDefaultStatement));
        defaultStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SwitchStatementRule>(this, &SwitchStatementRule::PostDefaultStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new SwitchStatementNode(span, context.fromExpression);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value->AddCase(context.fromCaseStatement);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value->SetDefault(context.fromDefaultStatement);
    }
    void PreExpression(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PreCaseStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostCaseStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCaseStatement_value = std::move(stack.top());
            context.fromCaseStatement = *static_cast<cminor::parsing::ValueObject<CaseStatementNode*>*>(fromCaseStatement_value.get());
            stack.pop();
        }
    }
    void PreDefaultStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostDefaultStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDefaultStatement_value = std::move(stack.top());
            context.fromDefaultStatement = *static_cast<cminor::parsing::ValueObject<DefaultStatementNode*>*>(fromDefaultStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromExpression(), fromCaseStatement(), fromDefaultStatement() {}
        ParsingContext* ctx;
        SwitchStatementNode* value;
        Node* fromExpression;
        CaseStatementNode* fromCaseStatement;
        DefaultStatementNode* fromDefaultStatement;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::CaseStatementRule : public cminor::parsing::Rule
{
public:
    CaseStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("CaseStatementNode*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<CaseStatementNode>", "caseS"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<CaseStatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CaseStatementRule>(this, &CaseStatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CaseStatementRule>(this, &CaseStatementRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CaseStatementRule>(this, &CaseStatementRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CaseStatementRule>(this, &CaseStatementRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CaseStatementRule>(this, &CaseStatementRule::A4Action));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<CaseStatementRule>(this, &CaseStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CaseStatementRule>(this, &CaseStatementRule::PostExpression));
        cminor::parsing::NonterminalParser* statementNonterminalParser = GetNonterminal("Statement");
        statementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<CaseStatementRule>(this, &CaseStatementRule::PreStatement));
        statementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CaseStatementRule>(this, &CaseStatementRule::PostStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.caseS.release();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.caseS->GetSpan().SetEnd(span.End());
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.caseS.reset(new CaseStatementNode(span));
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.caseS->AddCaseExpr(context.fromExpression);
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.caseS->AddStatement(context.fromStatement);
    }
    void PreExpression(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PreStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromStatement_value = std::move(stack.top());
            context.fromStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), caseS(), fromExpression(), fromStatement() {}
        ParsingContext* ctx;
        CaseStatementNode* value;
        std::unique_ptr<CaseStatementNode> caseS;
        Node* fromExpression;
        StatementNode* fromStatement;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::DefaultStatementRule : public cminor::parsing::Rule
{
public:
    DefaultStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("DefaultStatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<DefaultStatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DefaultStatementRule>(this, &DefaultStatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DefaultStatementRule>(this, &DefaultStatementRule::A1Action));
        cminor::parsing::NonterminalParser* statementNonterminalParser = GetNonterminal("Statement");
        statementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DefaultStatementRule>(this, &DefaultStatementRule::PreStatement));
        statementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DefaultStatementRule>(this, &DefaultStatementRule::PostStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new DefaultStatementNode(span);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value->AddStatement(context.fromStatement);
    }
    void PreStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromStatement_value = std::move(stack.top());
            context.fromStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromStatement() {}
        ParsingContext* ctx;
        DefaultStatementNode* value;
        StatementNode* fromStatement;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::GotoCaseStatementRule : public cminor::parsing::Rule
{
public:
    GotoCaseStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<GotoCaseStatementRule>(this, &GotoCaseStatementRule::A0Action));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<GotoCaseStatementRule>(this, &GotoCaseStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<GotoCaseStatementRule>(this, &GotoCaseStatementRule::PostExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new GotoCaseStatementNode(span, context.fromExpression);
    }
    void PreExpression(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromExpression() {}
        ParsingContext* ctx;
        StatementNode* value;
        Node* fromExpression;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::GotoDefaultStatementRule : public cminor::parsing::Rule
{
public:
    GotoDefaultStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<GotoDefaultStatementRule>(this, &GotoDefaultStatementRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new GotoDefaultStatementNode(span);
    }
private:
    struct Context
    {
        Context(): ctx(), value() {}
        ParsingContext* ctx;
        StatementNode* value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::AssignmentExpressionStatementRule : public cminor::parsing::Rule
{
public:
    AssignmentExpressionStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "targetExpr"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AssignmentExpressionStatementRule>(this, &AssignmentExpressionStatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AssignmentExpressionStatementRule>(this, &AssignmentExpressionStatementRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AssignmentExpressionStatementRule>(this, &AssignmentExpressionStatementRule::A2Action));
        a2ActionParser->SetFailureAction(new cminor::parsing::MemberFailureAction<AssignmentExpressionStatementRule>(this, &AssignmentExpressionStatementRule::A2ActionFail));
        cminor::parsing::NonterminalParser* targetNonterminalParser = GetNonterminal("target");
        targetNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<AssignmentExpressionStatementRule>(this, &AssignmentExpressionStatementRule::Pretarget));
        targetNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AssignmentExpressionStatementRule>(this, &AssignmentExpressionStatementRule::Posttarget));
        cminor::parsing::NonterminalParser* sourceNonterminalParser = GetNonterminal("source");
        sourceNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<AssignmentExpressionStatementRule>(this, &AssignmentExpressionStatementRule::Presource));
        sourceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AssignmentExpressionStatementRule>(this, &AssignmentExpressionStatementRule::Postsource));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ctx->PushParsingLvalue(true);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new AssignmentStatementNode(span, context.targetExpr.release(), context.fromsource);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ctx->PopParsingLvalue();
        context.targetExpr.reset(context.fromtarget);
    }
    void A2ActionFail()
    {
        context.ctx->PopParsingLvalue();
    }
    void Pretarget(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Posttarget(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromtarget_value = std::move(stack.top());
            context.fromtarget = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromtarget_value.get());
            stack.pop();
        }
    }
    void Presource(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postsource(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromsource_value = std::move(stack.top());
            context.fromsource = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromsource_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), targetExpr(), fromtarget(), fromsource() {}
        ParsingContext* ctx;
        StatementNode* value;
        std::unique_ptr<Node> targetExpr;
        Node* fromtarget;
        Node* fromsource;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::AssignmentStatementRule : public cminor::parsing::Rule
{
public:
    AssignmentStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AssignmentStatementRule>(this, &AssignmentStatementRule::A0Action));
        cminor::parsing::NonterminalParser* assignmentExpressionStatementNonterminalParser = GetNonterminal("AssignmentExpressionStatement");
        assignmentExpressionStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<AssignmentStatementRule>(this, &AssignmentStatementRule::PreAssignmentExpressionStatement));
        assignmentExpressionStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AssignmentStatementRule>(this, &AssignmentStatementRule::PostAssignmentExpressionStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromAssignmentExpressionStatement;
        context.value->GetSpan().SetEnd(span.End());
    }
    void PreAssignmentExpressionStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostAssignmentExpressionStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAssignmentExpressionStatement_value = std::move(stack.top());
            context.fromAssignmentExpressionStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromAssignmentExpressionStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromAssignmentExpressionStatement() {}
        ParsingContext* ctx;
        StatementNode* value;
        StatementNode* fromAssignmentExpressionStatement;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::ConstructionStatementRule : public cminor::parsing::Rule
{
public:
    ConstructionStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("ConstructionStatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ConstructionStatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConstructionStatementRule>(this, &ConstructionStatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConstructionStatementRule>(this, &ConstructionStatementRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConstructionStatementRule>(this, &ConstructionStatementRule::A2Action));
        cminor::parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal("TypeExpr");
        typeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ConstructionStatementRule>(this, &ConstructionStatementRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConstructionStatementRule>(this, &ConstructionStatementRule::PostTypeExpr));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConstructionStatementRule>(this, &ConstructionStatementRule::PostIdentifier));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ConstructionStatementRule>(this, &ConstructionStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConstructionStatementRule>(this, &ConstructionStatementRule::PostExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ConstructionStatementNode(span, context.fromTypeExpr, context.fromIdentifier);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value->SetInitializer(context.fromExpression);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value->GetSpan().SetEnd(span.End());
    }
    void PreTypeExpr(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostTypeExpr(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeExpr_value = std::move(stack.top());
            context.fromTypeExpr = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromTypeExpr_value.get());
            stack.pop();
        }
    }
    void PostIdentifier(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIdentifier_value = std::move(stack.top());
            context.fromIdentifier = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
    void PreExpression(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromTypeExpr(), fromIdentifier(), fromExpression() {}
        ParsingContext* ctx;
        ConstructionStatementNode* value;
        Node* fromTypeExpr;
        IdentifierNode* fromIdentifier;
        Node* fromExpression;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::ConstructionStatementExpressionRule : public cminor::parsing::Rule
{
public:
    ConstructionStatementExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("ConstructionStatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ConstructionStatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConstructionStatementExpressionRule>(this, &ConstructionStatementExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConstructionStatementExpressionRule>(this, &ConstructionStatementExpressionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConstructionStatementExpressionRule>(this, &ConstructionStatementExpressionRule::A2Action));
        cminor::parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal("TypeExpr");
        typeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ConstructionStatementExpressionRule>(this, &ConstructionStatementExpressionRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConstructionStatementExpressionRule>(this, &ConstructionStatementExpressionRule::PostTypeExpr));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConstructionStatementExpressionRule>(this, &ConstructionStatementExpressionRule::PostIdentifier));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ConstructionStatementExpressionRule>(this, &ConstructionStatementExpressionRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConstructionStatementExpressionRule>(this, &ConstructionStatementExpressionRule::PostExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value->GetSpan().SetEnd(span.End());
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ConstructionStatementNode(span, context.fromTypeExpr, context.fromIdentifier);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value->SetInitializer(context.fromExpression);
    }
    void PreTypeExpr(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostTypeExpr(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeExpr_value = std::move(stack.top());
            context.fromTypeExpr = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromTypeExpr_value.get());
            stack.pop();
        }
    }
    void PostIdentifier(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIdentifier_value = std::move(stack.top());
            context.fromIdentifier = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
    void PreExpression(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromTypeExpr(), fromIdentifier(), fromExpression() {}
        ParsingContext* ctx;
        ConstructionStatementNode* value;
        Node* fromTypeExpr;
        IdentifierNode* fromIdentifier;
        Node* fromExpression;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::IncrementExpressionStatementRule : public cminor::parsing::Rule
{
public:
    IncrementExpressionStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IncrementExpressionStatementRule>(this, &IncrementExpressionStatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IncrementExpressionStatementRule>(this, &IncrementExpressionStatementRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IncrementExpressionStatementRule>(this, &IncrementExpressionStatementRule::A2Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<IncrementExpressionStatementRule>(this, &IncrementExpressionStatementRule::Preleft));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IncrementExpressionStatementRule>(this, &IncrementExpressionStatementRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<IncrementExpressionStatementRule>(this, &IncrementExpressionStatementRule::Preright));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IncrementExpressionStatementRule>(this, &IncrementExpressionStatementRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new IncrementStatementNode(span, context.fromleft);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new IncrementStatementNode(span, context.expr.release());
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.expr.reset(context.fromright);
    }
    void Preleft(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postleft(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postright(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), expr(), fromleft(), fromright() {}
        ParsingContext* ctx;
        StatementNode* value;
        std::unique_ptr<Node> expr;
        Node* fromleft;
        Node* fromright;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::IncrementStatementRule : public cminor::parsing::Rule
{
public:
    IncrementStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IncrementStatementRule>(this, &IncrementStatementRule::A0Action));
        cminor::parsing::NonterminalParser* incrementExpressionStatementNonterminalParser = GetNonterminal("IncrementExpressionStatement");
        incrementExpressionStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<IncrementStatementRule>(this, &IncrementStatementRule::PreIncrementExpressionStatement));
        incrementExpressionStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IncrementStatementRule>(this, &IncrementStatementRule::PostIncrementExpressionStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromIncrementExpressionStatement;
        context.value->GetSpan().SetEnd(span.End());
    }
    void PreIncrementExpressionStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostIncrementExpressionStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIncrementExpressionStatement_value = std::move(stack.top());
            context.fromIncrementExpressionStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromIncrementExpressionStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromIncrementExpressionStatement() {}
        ParsingContext* ctx;
        StatementNode* value;
        StatementNode* fromIncrementExpressionStatement;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::DecrementExpressionStatementRule : public cminor::parsing::Rule
{
public:
    DecrementExpressionStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DecrementExpressionStatementRule>(this, &DecrementExpressionStatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DecrementExpressionStatementRule>(this, &DecrementExpressionStatementRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DecrementExpressionStatementRule>(this, &DecrementExpressionStatementRule::A2Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DecrementExpressionStatementRule>(this, &DecrementExpressionStatementRule::Preleft));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DecrementExpressionStatementRule>(this, &DecrementExpressionStatementRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DecrementExpressionStatementRule>(this, &DecrementExpressionStatementRule::Preright));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DecrementExpressionStatementRule>(this, &DecrementExpressionStatementRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new DecrementStatementNode(span, context.fromleft);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new DecrementStatementNode(span, context.expr.release());
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.expr.reset(context.fromright);
    }
    void Preleft(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postleft(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postright(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), expr(), fromleft(), fromright() {}
        ParsingContext* ctx;
        StatementNode* value;
        std::unique_ptr<Node> expr;
        Node* fromleft;
        Node* fromright;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::DecrementStatementRule : public cminor::parsing::Rule
{
public:
    DecrementStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DecrementStatementRule>(this, &DecrementStatementRule::A0Action));
        cminor::parsing::NonterminalParser* decrementExpressionStatementNonterminalParser = GetNonterminal("DecrementExpressionStatement");
        decrementExpressionStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DecrementStatementRule>(this, &DecrementStatementRule::PreDecrementExpressionStatement));
        decrementExpressionStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DecrementStatementRule>(this, &DecrementStatementRule::PostDecrementExpressionStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromDecrementExpressionStatement;
        context.value->GetSpan().SetEnd(span.End());
    }
    void PreDecrementExpressionStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostDecrementExpressionStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDecrementExpressionStatement_value = std::move(stack.top());
            context.fromDecrementExpressionStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromDecrementExpressionStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromDecrementExpressionStatement() {}
        ParsingContext* ctx;
        StatementNode* value;
        StatementNode* fromDecrementExpressionStatement;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::ExpressionStatementRule : public cminor::parsing::Rule
{
public:
    ExpressionStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExpressionStatementRule>(this, &ExpressionStatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExpressionStatementRule>(this, &ExpressionStatementRule::A1Action));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ExpressionStatementRule>(this, &ExpressionStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ExpressionStatementRule>(this, &ExpressionStatementRule::PostExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ExpressionStatementNode(span, context.expr.release());
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.expr.reset(context.fromExpression);
    }
    void PreExpression(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), expr(), fromExpression() {}
        ParsingContext* ctx;
        StatementNode* value;
        std::unique_ptr<Node> expr;
        Node* fromExpression;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::EmptyStatementRule : public cminor::parsing::Rule
{
public:
    EmptyStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EmptyStatementRule>(this, &EmptyStatementRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new EmptyStatementNode(span);
    }
private:
    struct Context
    {
        Context(): ctx(), value() {}
        ParsingContext* ctx;
        StatementNode* value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::ThrowStatementRule : public cminor::parsing::Rule
{
public:
    ThrowStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ThrowStatementRule>(this, &ThrowStatementRule::A0Action));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ThrowStatementRule>(this, &ThrowStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ThrowStatementRule>(this, &ThrowStatementRule::PostExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ThrowStatementNode(span, context.fromExpression);
    }
    void PreExpression(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromExpression() {}
        ParsingContext* ctx;
        StatementNode* value;
        Node* fromExpression;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::TryStatementRule : public cminor::parsing::Rule
{
public:
    TryStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("TryStatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<TryStatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TryStatementRule>(this, &TryStatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TryStatementRule>(this, &TryStatementRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TryStatementRule>(this, &TryStatementRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TryStatementRule>(this, &TryStatementRule::A3Action));
        cminor::parsing::NonterminalParser* tryBlockNonterminalParser = GetNonterminal("tryBlock");
        tryBlockNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<TryStatementRule>(this, &TryStatementRule::PretryBlock));
        tryBlockNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TryStatementRule>(this, &TryStatementRule::PosttryBlock));
        cminor::parsing::NonterminalParser* catchNonterminalParser = GetNonterminal("Catch");
        catchNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<TryStatementRule>(this, &TryStatementRule::PreCatch));
        catchNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TryStatementRule>(this, &TryStatementRule::PostCatch));
        cminor::parsing::NonterminalParser* finallyNonterminalParser = GetNonterminal("Finally");
        finallyNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<TryStatementRule>(this, &TryStatementRule::PreFinally));
        finallyNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TryStatementRule>(this, &TryStatementRule::PostFinally));
        cminor::parsing::NonterminalParser* finNonterminalParser = GetNonterminal("fin");
        finNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<TryStatementRule>(this, &TryStatementRule::Prefin));
        finNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TryStatementRule>(this, &TryStatementRule::Postfin));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new TryStatementNode(span, context.fromtryBlock);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value->AddCatch(context.fromCatch);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value->SetFinally(context.fromFinally);
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value->SetFinally(context.fromfin);
    }
    void PretryBlock(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PosttryBlock(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromtryBlock_value = std::move(stack.top());
            context.fromtryBlock = *static_cast<cminor::parsing::ValueObject<CompoundStatementNode*>*>(fromtryBlock_value.get());
            stack.pop();
        }
    }
    void PreCatch(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostCatch(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCatch_value = std::move(stack.top());
            context.fromCatch = *static_cast<cminor::parsing::ValueObject<CatchNode*>*>(fromCatch_value.get());
            stack.pop();
        }
    }
    void PreFinally(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostFinally(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromFinally_value = std::move(stack.top());
            context.fromFinally = *static_cast<cminor::parsing::ValueObject<CompoundStatementNode*>*>(fromFinally_value.get());
            stack.pop();
        }
    }
    void Prefin(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postfin(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromfin_value = std::move(stack.top());
            context.fromfin = *static_cast<cminor::parsing::ValueObject<CompoundStatementNode*>*>(fromfin_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromtryBlock(), fromCatch(), fromFinally(), fromfin() {}
        ParsingContext* ctx;
        TryStatementNode* value;
        CompoundStatementNode* fromtryBlock;
        CatchNode* fromCatch;
        CompoundStatementNode* fromFinally;
        CompoundStatementNode* fromfin;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::CatchRule : public cminor::parsing::Rule
{
public:
    CatchRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("CatchNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<CatchNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CatchRule>(this, &CatchRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CatchRule>(this, &CatchRule::A1Action));
        cminor::parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal("TypeExpr");
        typeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<CatchRule>(this, &CatchRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CatchRule>(this, &CatchRule::PostTypeExpr));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CatchRule>(this, &CatchRule::PostIdentifier));
        cminor::parsing::NonterminalParser* catchBlock1NonterminalParser = GetNonterminal("catchBlock1");
        catchBlock1NonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<CatchRule>(this, &CatchRule::PrecatchBlock1));
        catchBlock1NonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CatchRule>(this, &CatchRule::PostcatchBlock1));
        cminor::parsing::NonterminalParser* catchBlock2NonterminalParser = GetNonterminal("catchBlock2");
        catchBlock2NonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<CatchRule>(this, &CatchRule::PrecatchBlock2));
        catchBlock2NonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CatchRule>(this, &CatchRule::PostcatchBlock2));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new CatchNode(span, context.fromTypeExpr, context.fromIdentifier, context.fromcatchBlock1);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new CatchNode(span, context.fromcatchBlock2);
    }
    void PreTypeExpr(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostTypeExpr(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeExpr_value = std::move(stack.top());
            context.fromTypeExpr = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromTypeExpr_value.get());
            stack.pop();
        }
    }
    void PostIdentifier(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIdentifier_value = std::move(stack.top());
            context.fromIdentifier = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
    void PrecatchBlock1(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostcatchBlock1(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromcatchBlock1_value = std::move(stack.top());
            context.fromcatchBlock1 = *static_cast<cminor::parsing::ValueObject<CompoundStatementNode*>*>(fromcatchBlock1_value.get());
            stack.pop();
        }
    }
    void PrecatchBlock2(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostcatchBlock2(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromcatchBlock2_value = std::move(stack.top());
            context.fromcatchBlock2 = *static_cast<cminor::parsing::ValueObject<CompoundStatementNode*>*>(fromcatchBlock2_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromTypeExpr(), fromIdentifier(), fromcatchBlock1(), fromcatchBlock2() {}
        ParsingContext* ctx;
        CatchNode* value;
        Node* fromTypeExpr;
        IdentifierNode* fromIdentifier;
        CompoundStatementNode* fromcatchBlock1;
        CompoundStatementNode* fromcatchBlock2;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::FinallyRule : public cminor::parsing::Rule
{
public:
    FinallyRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("CompoundStatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<CompoundStatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FinallyRule>(this, &FinallyRule::A0Action));
        cminor::parsing::NonterminalParser* compoundStatementNonterminalParser = GetNonterminal("CompoundStatement");
        compoundStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<FinallyRule>(this, &FinallyRule::PreCompoundStatement));
        compoundStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<FinallyRule>(this, &FinallyRule::PostCompoundStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromCompoundStatement;
    }
    void PreCompoundStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostCompoundStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCompoundStatement_value = std::move(stack.top());
            context.fromCompoundStatement = *static_cast<cminor::parsing::ValueObject<CompoundStatementNode*>*>(fromCompoundStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromCompoundStatement() {}
        ParsingContext* ctx;
        CompoundStatementNode* value;
        CompoundStatementNode* fromCompoundStatement;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::UsingStatementRule : public cminor::parsing::Rule
{
public:
    UsingStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UsingStatementRule>(this, &UsingStatementRule::A0Action));
        cminor::parsing::NonterminalParser* constructionStatementExpressionNonterminalParser = GetNonterminal("ConstructionStatementExpression");
        constructionStatementExpressionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<UsingStatementRule>(this, &UsingStatementRule::PreConstructionStatementExpression));
        constructionStatementExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UsingStatementRule>(this, &UsingStatementRule::PostConstructionStatementExpression));
        cminor::parsing::NonterminalParser* statementNonterminalParser = GetNonterminal("Statement");
        statementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<UsingStatementRule>(this, &UsingStatementRule::PreStatement));
        statementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UsingStatementRule>(this, &UsingStatementRule::PostStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new UsingStatementNode(span, context.fromConstructionStatementExpression, context.fromStatement);
    }
    void PreConstructionStatementExpression(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostConstructionStatementExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromConstructionStatementExpression_value = std::move(stack.top());
            context.fromConstructionStatementExpression = *static_cast<cminor::parsing::ValueObject<ConstructionStatementNode*>*>(fromConstructionStatementExpression_value.get());
            stack.pop();
        }
    }
    void PreStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromStatement_value = std::move(stack.top());
            context.fromStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromConstructionStatementExpression(), fromStatement() {}
        ParsingContext* ctx;
        StatementNode* value;
        ConstructionStatementNode* fromConstructionStatementExpression;
        StatementNode* fromStatement;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::LockStatementRule : public cminor::parsing::Rule
{
public:
    LockStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LockStatementRule>(this, &LockStatementRule::A0Action));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<LockStatementRule>(this, &LockStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LockStatementRule>(this, &LockStatementRule::PostExpression));
        cminor::parsing::NonterminalParser* statementNonterminalParser = GetNonterminal("Statement");
        statementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<LockStatementRule>(this, &LockStatementRule::PreStatement));
        statementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LockStatementRule>(this, &LockStatementRule::PostStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new LockStatementNode(span, context.fromExpression, context.fromStatement);
    }
    void PreExpression(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PreStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromStatement_value = std::move(stack.top());
            context.fromStatement = *static_cast<cminor::parsing::ValueObject<StatementNode*>*>(fromStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromExpression(), fromStatement() {}
        ParsingContext* ctx;
        StatementNode* value;
        Node* fromExpression;
        StatementNode* fromStatement;
    };
    std::stack<Context> contextStack;
    Context context;
};

void StatementGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("cminor.parser.TypeExprGrammar");
    if (!grammar0)
    {
        grammar0 = cminor::parser::TypeExprGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar("cminor.parser.ExpressionGrammar");
    if (!grammar1)
    {
        grammar1 = cminor::parser::ExpressionGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar("cminor.parsing.stdlib");
    if (!grammar2)
    {
        grammar2 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar2);
    cminor::parsing::Grammar* grammar3 = pd->GetGrammar("cminor.parser.KeywordGrammar");
    if (!grammar3)
    {
        grammar3 = cminor::parser::KeywordGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
    cminor::parsing::Grammar* grammar4 = pd->GetGrammar("cminor.parser.IdentifierGrammar");
    if (!grammar4)
    {
        grammar4 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar4);
}

void StatementGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("identifier", this, "cminor.parsing.stdlib.identifier"));
    AddRuleLink(new cminor::parsing::RuleLink("Keyword", this, "KeywordGrammar.Keyword"));
    AddRuleLink(new cminor::parsing::RuleLink("ArgumentList", this, "ExpressionGrammar.ArgumentList"));
    AddRuleLink(new cminor::parsing::RuleLink("Expression", this, "ExpressionGrammar.Expression"));
    AddRuleLink(new cminor::parsing::RuleLink("TypeExpr", this, "TypeExprGrammar.TypeExpr"));
    AddRuleLink(new cminor::parsing::RuleLink("Identifier", this, "IdentifierGrammar.Identifier"));
    AddRule(new StatementRule("Statement", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::AlternativeParser(
                                    new cminor::parsing::AlternativeParser(
                                        new cminor::parsing::AlternativeParser(
                                            new cminor::parsing::AlternativeParser(
                                                new cminor::parsing::AlternativeParser(
                                                    new cminor::parsing::ActionParser("A0",
                                                        new cminor::parsing::NonterminalParser("LabeledStatement", "LabeledStatement", 1)),
                                                    new cminor::parsing::ActionParser("A1",
                                                        new cminor::parsing::NonterminalParser("ControlStatement", "ControlStatement", 1))),
                                                new cminor::parsing::ActionParser("A2",
                                                    new cminor::parsing::NonterminalParser("IncrementStatement", "IncrementStatement", 1))),
                                            new cminor::parsing::ActionParser("A3",
                                                new cminor::parsing::NonterminalParser("DecrementStatement", "DecrementStatement", 1))),
                                        new cminor::parsing::ActionParser("A4",
                                            new cminor::parsing::NonterminalParser("ExpressionStatement", "ExpressionStatement", 1))),
                                    new cminor::parsing::ActionParser("A5",
                                        new cminor::parsing::NonterminalParser("AssignmentStatement", "AssignmentStatement", 1))),
                                new cminor::parsing::ActionParser("A6",
                                    new cminor::parsing::NonterminalParser("ConstructionStatement", "ConstructionStatement", 1))),
                            new cminor::parsing::ActionParser("A7",
                                new cminor::parsing::NonterminalParser("EmptyStatement", "EmptyStatement", 1))),
                        new cminor::parsing::ActionParser("A8",
                            new cminor::parsing::NonterminalParser("ThrowStatement", "ThrowStatement", 1))),
                    new cminor::parsing::ActionParser("A9",
                        new cminor::parsing::NonterminalParser("TryStatement", "TryStatement", 1))),
                new cminor::parsing::ActionParser("A10",
                    new cminor::parsing::NonterminalParser("UsingStatement", "UsingStatement", 1))),
            new cminor::parsing::ActionParser("A11",
                new cminor::parsing::NonterminalParser("LockStatement", "LockStatement", 1)))));
    AddRule(new LabelIdRule("LabelId", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::DifferenceParser(
                new cminor::parsing::NonterminalParser("identifier", "identifier", 0),
                new cminor::parsing::NonterminalParser("Keyword", "Keyword", 0)))));
    AddRule(new LabelRule("Label", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("LabelId", "LabelId", 0)),
                new cminor::parsing::CharParser(':')))));
    AddRule(new LabeledStatementRule("LabeledStatement", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::NonterminalParser("lbl", "Label", 0),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("stmt", "Statement", 1)))));
    AddRule(new ControlStatementRule("ControlStatement", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::AlternativeParser(
                                    new cminor::parsing::AlternativeParser(
                                        new cminor::parsing::AlternativeParser(
                                            new cminor::parsing::AlternativeParser(
                                                new cminor::parsing::AlternativeParser(
                                                    new cminor::parsing::AlternativeParser(
                                                        new cminor::parsing::ActionParser("A0",
                                                            new cminor::parsing::NonterminalParser("CompoundStatement", "CompoundStatement", 1)),
                                                        new cminor::parsing::ActionParser("A1",
                                                            new cminor::parsing::NonterminalParser("ReturnStatement", "ReturnStatement", 1))),
                                                    new cminor::parsing::ActionParser("A2",
                                                        new cminor::parsing::NonterminalParser("IfStatement", "IfStatement", 1))),
                                                new cminor::parsing::ActionParser("A3",
                                                    new cminor::parsing::NonterminalParser("WhileStatement", "WhileStatement", 1))),
                                            new cminor::parsing::ActionParser("A4",
                                                new cminor::parsing::NonterminalParser("DoStatement", "DoStatement", 1))),
                                        new cminor::parsing::ActionParser("A5",
                                            new cminor::parsing::NonterminalParser("ForStatement", "ForStatement", 1))),
                                    new cminor::parsing::ActionParser("A6",
                                        new cminor::parsing::NonterminalParser("ForEachStatement", "ForEachStatement", 1))),
                                new cminor::parsing::ActionParser("A7",
                                    new cminor::parsing::NonterminalParser("BreakStatement", "BreakStatement", 1))),
                            new cminor::parsing::ActionParser("A8",
                                new cminor::parsing::NonterminalParser("ContinueStatement", "ContinueStatement", 1))),
                        new cminor::parsing::ActionParser("A9",
                            new cminor::parsing::NonterminalParser("GotoStatement", "GotoStatement", 1))),
                    new cminor::parsing::ActionParser("A10",
                        new cminor::parsing::NonterminalParser("SwitchStatement", "SwitchStatement", 1))),
                new cminor::parsing::ActionParser("A11",
                    new cminor::parsing::NonterminalParser("GotoCaseStatement", "GotoCaseStatement", 1))),
            new cminor::parsing::ActionParser("A12",
                new cminor::parsing::NonterminalParser("GotoDefaultStatement", "GotoDefaultStatement", 1)))));
    AddRule(new CompoundStatementRule("CompoundStatement", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::CharParser('{')),
                    new cminor::parsing::KleeneStarParser(
                        new cminor::parsing::ActionParser("A2",
                            new cminor::parsing::NonterminalParser("stmt", "Statement", 1)))),
                new cminor::parsing::ActionParser("A3",
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('}')))))));
    AddRule(new ReturnStatementRule("ReturnStatement", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser("return"),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::NonterminalParser("Expression", "Expression", 1))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new IfStatementRule("IfStatement", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::KeywordParser("if"),
                                new cminor::parsing::CharParser('(')),
                            new cminor::parsing::NonterminalParser("Expression", "Expression", 1)),
                        new cminor::parsing::CharParser(')')),
                    new cminor::parsing::NonterminalParser("thens", "Statement", 1)),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::KeywordParser("else"),
                        new cminor::parsing::NonterminalParser("elses", "Statement", 1)))))));
    AddRule(new WhileStatementRule("WhileStatement", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::KeywordParser("while"),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::CharParser('('))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("Expression", "Expression", 1))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser(')'))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser("Statement", "Statement", 1))))));
    AddRule(new DoStatementRule("DoStatement", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::KeywordParser("do"),
                                    new cminor::parsing::ExpectationParser(
                                        new cminor::parsing::NonterminalParser("Statement", "Statement", 1))),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::KeywordParser("while"))),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::CharParser('('))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("Expression", "Expression", 1))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser(')'))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new ForStatementRule("ForStatement", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::SequenceParser(
                                        new cminor::parsing::KeywordParser("for"),
                                        new cminor::parsing::CharParser('(')),
                                    new cminor::parsing::ExpectationParser(
                                        new cminor::parsing::NonterminalParser("ForInitStatement", "ForInitStatement", 1))),
                                new cminor::parsing::OptionalParser(
                                    new cminor::parsing::NonterminalParser("Expression", "Expression", 1))),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::CharParser(';'))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("ForLoopExpressionStatement", "ForLoopExpressionStatement", 1))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser(')'))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser("Statement", "Statement", 1))))));
    AddRule(new ForInitStatementRule("ForInitStatement", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::NonterminalParser("AssignmentStatement", "AssignmentStatement", 1)),
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("ConstructionStatement", "ConstructionStatement", 1))),
            new cminor::parsing::ActionParser("A2",
                new cminor::parsing::NonterminalParser("EmptyStatement", "EmptyStatement", 1)))));
    AddRule(new ForLoopExpressionStatementRule("ForLoopExpressionStatement", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::ActionParser("A0",
                        new cminor::parsing::NonterminalParser("IncrementExpressionStatement", "IncrementExpressionStatement", 1)),
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::NonterminalParser("DecrementExpressionStatement", "DecrementExpressionStatement", 1))),
                new cminor::parsing::ActionParser("A2",
                    new cminor::parsing::NonterminalParser("AssignmentExpressionStatement", "AssignmentExpressionStatement", 1))),
            new cminor::parsing::ActionParser("A3",
                new cminor::parsing::EmptyParser()))));
    AddRule(new ForEachStatementRule("ForEachStatement", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::SequenceParser(
                                        new cminor::parsing::KeywordParser("foreach"),
                                        new cminor::parsing::ExpectationParser(
                                            new cminor::parsing::CharParser('('))),
                                    new cminor::parsing::ExpectationParser(
                                        new cminor::parsing::NonterminalParser("TypeExpr", "TypeExpr", 1))),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::NonterminalParser("Identifier", "Identifier", 0))),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::KeywordParser("in"))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("Expression", "Expression", 1))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser(')'))),
                new cminor::parsing::NonterminalParser("CompoundStatement", "CompoundStatement", 1)))));
    AddRule(new BreakStatementRule("BreakStatement", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::KeywordParser("break"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new ContinueStatementRule("ContinueStatement", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::KeywordParser("continue"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new GotoStatementRule("GotoStatement", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser("goto"),
                    new cminor::parsing::NonterminalParser("Identifier", "Identifier", 0)),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new SwitchStatementRule("SwitchStatement", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::ActionParser("A0",
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::KeywordParser("switch"),
                                    new cminor::parsing::ExpectationParser(
                                        new cminor::parsing::CharParser('('))),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::NonterminalParser("Expression", "Expression", 1))),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::CharParser(')')))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('{'))),
                new cminor::parsing::KleeneStarParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::ActionParser("A1",
                            new cminor::parsing::NonterminalParser("CaseStatement", "CaseStatement", 1)),
                        new cminor::parsing::ActionParser("A2",
                            new cminor::parsing::NonterminalParser("DefaultStatement", "DefaultStatement", 1))))),
            new cminor::parsing::ExpectationParser(
                new cminor::parsing::CharParser('}')))));
    AddRule(new CaseStatementRule("CaseStatement", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::ActionParser("A2",
                            new cminor::parsing::EmptyParser()),
                        new cminor::parsing::PositiveParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::KeywordParser("case"),
                                    new cminor::parsing::ActionParser("A3",
                                        new cminor::parsing::NonterminalParser("Expression", "Expression", 1))),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::CharParser(':')))))),
                new cminor::parsing::KleeneStarParser(
                    new cminor::parsing::ActionParser("A4",
                        new cminor::parsing::NonterminalParser("Statement", "Statement", 1)))))));
    AddRule(new DefaultStatementRule("DefaultStatement", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser("default"),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser(':')))),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("Statement", "Statement", 1))))));
    AddRule(new GotoCaseStatementRule("GotoCaseStatement", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::KeywordParser("goto"),
                        new cminor::parsing::KeywordParser("case")),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser("Expression", "Expression", 1))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new GotoDefaultStatementRule("GotoDefaultStatement", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser("goto"),
                    new cminor::parsing::KeywordParser("default")),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new AssignmentExpressionStatementRule("AssignmentExpressionStatement", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::EmptyParser()),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::ActionParser("A2",
                            new cminor::parsing::NonterminalParser("target", "Expression", 1)),
                        new cminor::parsing::CharParser('=')),
                    new cminor::parsing::NonterminalParser("source", "Expression", 1))))));
    AddRule(new AssignmentStatementRule("AssignmentStatement", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::NonterminalParser("AssignmentExpressionStatement", "AssignmentExpressionStatement", 1),
                new cminor::parsing::CharParser(';')))));
    AddRule(new ConstructionStatementRule("ConstructionStatement", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser("TypeExpr", "TypeExpr", 1),
                    new cminor::parsing::ActionParser("A0",
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("Identifier", "Identifier", 0)))),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::CharParser('='),
                        new cminor::parsing::ActionParser("A1",
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser("Expression", "Expression", 1)))))),
            new cminor::parsing::ActionParser("A2",
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new ConstructionStatementExpressionRule("ConstructionStatementExpression", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser("TypeExpr", "TypeExpr", 1),
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("Identifier", "Identifier", 0)))),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::CharParser('='),
                        new cminor::parsing::ActionParser("A2",
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser("Expression", "Expression", 1)))))))));
    AddRule(new IncrementExpressionStatementRule("IncrementExpressionStatement", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::StringParser("++"),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser("left", "Expression", 1)))),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::ActionParser("A2",
                        new cminor::parsing::NonterminalParser("right", "Expression", 1)),
                    new cminor::parsing::StringParser("++"))))));
    AddRule(new IncrementStatementRule("IncrementStatement", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::NonterminalParser("IncrementExpressionStatement", "IncrementExpressionStatement", 1),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new DecrementExpressionStatementRule("DecrementExpressionStatement", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::StringParser("--"),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser("left", "Expression", 1)))),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::ActionParser("A2",
                        new cminor::parsing::NonterminalParser("right", "Expression", 1)),
                    new cminor::parsing::StringParser("--"))))));
    AddRule(new DecrementStatementRule("DecrementStatement", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::NonterminalParser("DecrementExpressionStatement", "DecrementExpressionStatement", 1),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new ExpressionStatementRule("ExpressionStatement", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("Expression", "Expression", 1)),
                new cminor::parsing::CharParser(';')))));
    AddRule(new EmptyStatementRule("EmptyStatement", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::CharParser(';'))));
    AddRule(new ThrowStatementRule("ThrowStatement", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser("throw"),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::NonterminalParser("Expression", "Expression", 1))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new TryStatementRule("TryStatement", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::KeywordParser("try"),
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::NonterminalParser("tryBlock", "CompoundStatement", 1))),
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::PositiveParser(
                        new cminor::parsing::ActionParser("A1",
                            new cminor::parsing::NonterminalParser("Catch", "Catch", 1))),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::ActionParser("A2",
                            new cminor::parsing::NonterminalParser("Finally", "Finally", 1)))),
                new cminor::parsing::ActionParser("A3",
                    new cminor::parsing::NonterminalParser("fin", "Finally", 1))))));
    AddRule(new CatchRule("Catch", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::KeywordParser("catch"),
                                    new cminor::parsing::ExpectationParser(
                                        new cminor::parsing::CharParser('('))),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::NonterminalParser("TypeExpr", "TypeExpr", 1))),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser("Identifier", "Identifier", 0))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser(')'))),
                    new cminor::parsing::NonterminalParser("catchBlock1", "CompoundStatement", 1))),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser("catch"),
                    new cminor::parsing::NonterminalParser("catchBlock2", "CompoundStatement", 1))))));
    AddRule(new FinallyRule("Finally", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::KeywordParser("finally"),
                new cminor::parsing::NonterminalParser("CompoundStatement", "CompoundStatement", 1)))));
    AddRule(new UsingStatementRule("UsingStatement", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::KeywordParser("using"),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::CharParser('('))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("ConstructionStatementExpression", "ConstructionStatementExpression", 1))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser(')'))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser("Statement", "Statement", 1))))));
    AddRule(new LockStatementRule("LockStatement", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::KeywordParser("lock"),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::CharParser('('))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("Expression", "Expression", 1))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser(')'))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser("Statement", "Statement", 1))))));
}

} } // namespace cminor.parser
