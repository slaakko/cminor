#include "Statement.hpp"
#include <Cm.Parsing/Action.hpp>
#include <Cm.Parsing/Rule.hpp>
#include <Cm.Parsing/ParsingDomain.hpp>
#include <Cm.Parsing/Primitive.hpp>
#include <Cm.Parsing/Composite.hpp>
#include <Cm.Parsing/Nonterminal.hpp>
#include <Cm.Parsing/Exception.hpp>
#include <Cm.Parsing/StdLib.hpp>
#include <Cm.Parsing/XmlLog.hpp>
#include <cminor/parser/Keyword.hpp>
#include <cminor/parser/Expression.hpp>
#include <cminor/parser/TypeExpr.hpp>
#include <cminor/parser/Identifier.hpp>

namespace cminor { namespace parser {

using namespace Cm::Parsing;

StatementGrammar* StatementGrammar::Create()
{
    return Create(new Cm::Parsing::ParsingDomain());
}

StatementGrammar* StatementGrammar::Create(Cm::Parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    StatementGrammar* grammar(new StatementGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

StatementGrammar::StatementGrammar(Cm::Parsing::ParsingDomain* parsingDomain_): Cm::Parsing::Grammar("StatementGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

StatementNode* StatementGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
{
    Cm::Parsing::Scanner scanner(start, end, fileName, fileIndex, SkipRule());
    std::unique_ptr<Cm::Parsing::XmlLog> xmlLog;
    if (Log())
    {
        xmlLog.reset(new Cm::Parsing::XmlLog(*Log(), MaxLogLineLength()));
        scanner.SetLog(xmlLog.get());
        xmlLog->WriteBeginRule("parse");
    }
    Cm::Parsing::ObjectStack stack;
    stack.push(std::unique_ptr<Cm::Parsing::Object>(new ValueObject<ParsingContext*>(ctx)));
    Cm::Parsing::Match match = Cm::Parsing::Grammar::Parse(scanner, stack);
    Cm::Parsing::Span stop = scanner.GetSpan();
    if (Log())
    {
        xmlLog->WriteEndRule("parse");
    }
    if (!match.Hit() || !CC() && stop.Start() != int(end - start))
    {
        if (StartRule())
        {
            throw Cm::Parsing::ExpectationFailure(StartRule()->Info(), fileName, stop, start, end);
        }
        else
        {
            throw Cm::Parsing::ParsingException("grammar '" + Name() + "' has no start rule", fileName, scanner.GetSpan(), start, end);
        }
    }
    std::unique_ptr<Cm::Parsing::Object> value = std::move(stack.top());
    StatementNode* result = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(value.get());
    stack.pop();
    return result;
}

class StatementGrammar::StatementRule : public Cm::Parsing::Rule
{
public:
    StatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A3Action));
        Cm::Parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A4Action));
        Cm::Parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A5Action));
        Cm::Parsing::ActionParser* a6ActionParser = GetAction("A6");
        a6ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A6Action));
        Cm::Parsing::ActionParser* a7ActionParser = GetAction("A7");
        a7ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A7Action));
        Cm::Parsing::ActionParser* a8ActionParser = GetAction("A8");
        a8ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A8Action));
        Cm::Parsing::ActionParser* a9ActionParser = GetAction("A9");
        a9ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A9Action));
        Cm::Parsing::NonterminalParser* labeledStatementNonterminalParser = GetNonterminal("LabeledStatement");
        labeledStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreLabeledStatement));
        labeledStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostLabeledStatement));
        Cm::Parsing::NonterminalParser* controlStatementNonterminalParser = GetNonterminal("ControlStatement");
        controlStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreControlStatement));
        controlStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostControlStatement));
        Cm::Parsing::NonterminalParser* incrementStatementNonterminalParser = GetNonterminal("IncrementStatement");
        incrementStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreIncrementStatement));
        incrementStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostIncrementStatement));
        Cm::Parsing::NonterminalParser* decrementStatementNonterminalParser = GetNonterminal("DecrementStatement");
        decrementStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreDecrementStatement));
        decrementStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostDecrementStatement));
        Cm::Parsing::NonterminalParser* expressionStatementNonterminalParser = GetNonterminal("ExpressionStatement");
        expressionStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreExpressionStatement));
        expressionStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostExpressionStatement));
        Cm::Parsing::NonterminalParser* assignmentStatementNonterminalParser = GetNonterminal("AssignmentStatement");
        assignmentStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreAssignmentStatement));
        assignmentStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostAssignmentStatement));
        Cm::Parsing::NonterminalParser* constructionStatementNonterminalParser = GetNonterminal("ConstructionStatement");
        constructionStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreConstructionStatement));
        constructionStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostConstructionStatement));
        Cm::Parsing::NonterminalParser* emptyStatementNonterminalParser = GetNonterminal("EmptyStatement");
        emptyStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreEmptyStatement));
        emptyStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostEmptyStatement));
        Cm::Parsing::NonterminalParser* throwStatementNonterminalParser = GetNonterminal("ThrowStatement");
        throwStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreThrowStatement));
        throwStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostThrowStatement));
        Cm::Parsing::NonterminalParser* tryStatementNonterminalParser = GetNonterminal("TryStatement");
        tryStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreTryStatement));
        tryStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostTryStatement));
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
    void PreLabeledStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostLabeledStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromLabeledStatement_value = std::move(stack.top());
            context.fromLabeledStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromLabeledStatement_value.get());
            stack.pop();
        }
    }
    void PreControlStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostControlStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromControlStatement_value = std::move(stack.top());
            context.fromControlStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromControlStatement_value.get());
            stack.pop();
        }
    }
    void PreIncrementStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostIncrementStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromIncrementStatement_value = std::move(stack.top());
            context.fromIncrementStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromIncrementStatement_value.get());
            stack.pop();
        }
    }
    void PreDecrementStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostDecrementStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromDecrementStatement_value = std::move(stack.top());
            context.fromDecrementStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromDecrementStatement_value.get());
            stack.pop();
        }
    }
    void PreExpressionStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpressionStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromExpressionStatement_value = std::move(stack.top());
            context.fromExpressionStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromExpressionStatement_value.get());
            stack.pop();
        }
    }
    void PreAssignmentStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostAssignmentStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromAssignmentStatement_value = std::move(stack.top());
            context.fromAssignmentStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromAssignmentStatement_value.get());
            stack.pop();
        }
    }
    void PreConstructionStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostConstructionStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromConstructionStatement_value = std::move(stack.top());
            context.fromConstructionStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromConstructionStatement_value.get());
            stack.pop();
        }
    }
    void PreEmptyStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostEmptyStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromEmptyStatement_value = std::move(stack.top());
            context.fromEmptyStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromEmptyStatement_value.get());
            stack.pop();
        }
    }
    void PreThrowStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostThrowStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromThrowStatement_value = std::move(stack.top());
            context.fromThrowStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromThrowStatement_value.get());
            stack.pop();
        }
    }
    void PreTryStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostTryStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromTryStatement_value = std::move(stack.top());
            context.fromTryStatement = *static_cast<Cm::Parsing::ValueObject<TryStatementNode*>*>(fromTryStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromLabeledStatement(), fromControlStatement(), fromIncrementStatement(), fromDecrementStatement(), fromExpressionStatement(), fromAssignmentStatement(), fromConstructionStatement(), fromEmptyStatement(), fromThrowStatement(), fromTryStatement() {}
        ParsingContext* ctx;
        StatementNode* value;
        StatementNode* fromLabeledStatement;
        StatementNode* fromControlStatement;
        StatementNode* fromIncrementStatement;
        StatementNode* fromDecrementStatement;
        StatementNode* fromExpressionStatement;
        StatementNode* fromAssignmentStatement;
        StatementNode* fromConstructionStatement;
        StatementNode* fromEmptyStatement;
        StatementNode* fromThrowStatement;
        TryStatementNode* fromTryStatement;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::LabelIdRule : public Cm::Parsing::Rule
{
public:
    LabelIdRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("std::string");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<std::string>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<LabelIdRule>(this, &LabelIdRule::A0Action));
        Cm::Parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("identifier");
        identifierNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<LabelIdRule>(this, &LabelIdRule::Postidentifier));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromidentifier;
    }
    void Postidentifier(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromidentifier_value = std::move(stack.top());
            context.fromidentifier = *static_cast<Cm::Parsing::ValueObject<std::string>*>(fromidentifier_value.get());
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

class StatementGrammar::LabelRule : public Cm::Parsing::Rule
{
public:
    LabelRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("LabelNode*");
        AddLocalVariable(AttrOrVariable("std::string", "label"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<LabelNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<LabelRule>(this, &LabelRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<LabelRule>(this, &LabelRule::A1Action));
        Cm::Parsing::NonterminalParser* labelIdNonterminalParser = GetNonterminal("LabelId");
        labelIdNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<LabelRule>(this, &LabelRule::PostLabelId));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new LabelNode(span, context.label);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.label = context.fromLabelId;
    }
    void PostLabelId(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromLabelId_value = std::move(stack.top());
            context.fromLabelId = *static_cast<Cm::Parsing::ValueObject<std::string>*>(fromLabelId_value.get());
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

class StatementGrammar::LabeledStatementRule : public Cm::Parsing::Rule
{
public:
    LabeledStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<LabeledStatementRule>(this, &LabeledStatementRule::A0Action));
        Cm::Parsing::NonterminalParser* lblNonterminalParser = GetNonterminal("lbl");
        lblNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<LabeledStatementRule>(this, &LabeledStatementRule::Postlbl));
        Cm::Parsing::NonterminalParser* stmtNonterminalParser = GetNonterminal("stmt");
        stmtNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<LabeledStatementRule>(this, &LabeledStatementRule::Prestmt));
        stmtNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<LabeledStatementRule>(this, &LabeledStatementRule::Poststmt));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.fromstmt->SetLabelNode(context.fromlbl);
        context.value = context.fromstmt;
    }
    void Postlbl(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromlbl_value = std::move(stack.top());
            context.fromlbl = *static_cast<Cm::Parsing::ValueObject<LabelNode*>*>(fromlbl_value.get());
            stack.pop();
        }
    }
    void Prestmt(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Poststmt(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromstmt_value = std::move(stack.top());
            context.fromstmt = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromstmt_value.get());
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

class StatementGrammar::ControlStatementRule : public Cm::Parsing::Rule
{
public:
    ControlStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A3Action));
        Cm::Parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A4Action));
        Cm::Parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A5Action));
        Cm::Parsing::ActionParser* a6ActionParser = GetAction("A6");
        a6ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A6Action));
        Cm::Parsing::ActionParser* a7ActionParser = GetAction("A7");
        a7ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A7Action));
        Cm::Parsing::ActionParser* a8ActionParser = GetAction("A8");
        a8ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A8Action));
        Cm::Parsing::ActionParser* a9ActionParser = GetAction("A9");
        a9ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A9Action));
        Cm::Parsing::ActionParser* a10ActionParser = GetAction("A10");
        a10ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A10Action));
        Cm::Parsing::ActionParser* a11ActionParser = GetAction("A11");
        a11ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A11Action));
        Cm::Parsing::NonterminalParser* compoundStatementNonterminalParser = GetNonterminal("CompoundStatement");
        compoundStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreCompoundStatement));
        compoundStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostCompoundStatement));
        Cm::Parsing::NonterminalParser* returnStatementNonterminalParser = GetNonterminal("ReturnStatement");
        returnStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreReturnStatement));
        returnStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostReturnStatement));
        Cm::Parsing::NonterminalParser* ifStatementNonterminalParser = GetNonterminal("IfStatement");
        ifStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreIfStatement));
        ifStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostIfStatement));
        Cm::Parsing::NonterminalParser* whileStatementNonterminalParser = GetNonterminal("WhileStatement");
        whileStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreWhileStatement));
        whileStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostWhileStatement));
        Cm::Parsing::NonterminalParser* doStatementNonterminalParser = GetNonterminal("DoStatement");
        doStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreDoStatement));
        doStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostDoStatement));
        Cm::Parsing::NonterminalParser* forStatementNonterminalParser = GetNonterminal("ForStatement");
        forStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreForStatement));
        forStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostForStatement));
        Cm::Parsing::NonterminalParser* forEachStatementNonterminalParser = GetNonterminal("ForEachStatement");
        forEachStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreForEachStatement));
        forEachStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostForEachStatement));
        Cm::Parsing::NonterminalParser* breakStatementNonterminalParser = GetNonterminal("BreakStatement");
        breakStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreBreakStatement));
        breakStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostBreakStatement));
        Cm::Parsing::NonterminalParser* continueStatementNonterminalParser = GetNonterminal("ContinueStatement");
        continueStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreContinueStatement));
        continueStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostContinueStatement));
        Cm::Parsing::NonterminalParser* switchStatementNonterminalParser = GetNonterminal("SwitchStatement");
        switchStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreSwitchStatement));
        switchStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostSwitchStatement));
        Cm::Parsing::NonterminalParser* gotoCaseStatementNonterminalParser = GetNonterminal("GotoCaseStatement");
        gotoCaseStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreGotoCaseStatement));
        gotoCaseStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostGotoCaseStatement));
        Cm::Parsing::NonterminalParser* gotoDefaultStatementNonterminalParser = GetNonterminal("GotoDefaultStatement");
        gotoDefaultStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreGotoDefaultStatement));
        gotoDefaultStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostGotoDefaultStatement));
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
        context.value = context.fromSwitchStatement;
    }
    void A10Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromGotoCaseStatement;
    }
    void A11Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromGotoDefaultStatement;
    }
    void PreCompoundStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostCompoundStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromCompoundStatement_value = std::move(stack.top());
            context.fromCompoundStatement = *static_cast<Cm::Parsing::ValueObject<CompoundStatementNode*>*>(fromCompoundStatement_value.get());
            stack.pop();
        }
    }
    void PreReturnStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostReturnStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromReturnStatement_value = std::move(stack.top());
            context.fromReturnStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromReturnStatement_value.get());
            stack.pop();
        }
    }
    void PreIfStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostIfStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromIfStatement_value = std::move(stack.top());
            context.fromIfStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromIfStatement_value.get());
            stack.pop();
        }
    }
    void PreWhileStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostWhileStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromWhileStatement_value = std::move(stack.top());
            context.fromWhileStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromWhileStatement_value.get());
            stack.pop();
        }
    }
    void PreDoStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostDoStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromDoStatement_value = std::move(stack.top());
            context.fromDoStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromDoStatement_value.get());
            stack.pop();
        }
    }
    void PreForStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostForStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromForStatement_value = std::move(stack.top());
            context.fromForStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromForStatement_value.get());
            stack.pop();
        }
    }
    void PreForEachStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostForEachStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromForEachStatement_value = std::move(stack.top());
            context.fromForEachStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromForEachStatement_value.get());
            stack.pop();
        }
    }
    void PreBreakStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostBreakStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromBreakStatement_value = std::move(stack.top());
            context.fromBreakStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromBreakStatement_value.get());
            stack.pop();
        }
    }
    void PreContinueStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostContinueStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromContinueStatement_value = std::move(stack.top());
            context.fromContinueStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromContinueStatement_value.get());
            stack.pop();
        }
    }
    void PreSwitchStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostSwitchStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromSwitchStatement_value = std::move(stack.top());
            context.fromSwitchStatement = *static_cast<Cm::Parsing::ValueObject<SwitchStatementNode*>*>(fromSwitchStatement_value.get());
            stack.pop();
        }
    }
    void PreGotoCaseStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostGotoCaseStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromGotoCaseStatement_value = std::move(stack.top());
            context.fromGotoCaseStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromGotoCaseStatement_value.get());
            stack.pop();
        }
    }
    void PreGotoDefaultStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostGotoDefaultStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromGotoDefaultStatement_value = std::move(stack.top());
            context.fromGotoDefaultStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromGotoDefaultStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromCompoundStatement(), fromReturnStatement(), fromIfStatement(), fromWhileStatement(), fromDoStatement(), fromForStatement(), fromForEachStatement(), fromBreakStatement(), fromContinueStatement(), fromSwitchStatement(), fromGotoCaseStatement(), fromGotoDefaultStatement() {}
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
        SwitchStatementNode* fromSwitchStatement;
        StatementNode* fromGotoCaseStatement;
        StatementNode* fromGotoDefaultStatement;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::CompoundStatementRule : public Cm::Parsing::Rule
{
public:
    CompoundStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("CompoundStatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<CompoundStatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CompoundStatementRule>(this, &CompoundStatementRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CompoundStatementRule>(this, &CompoundStatementRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CompoundStatementRule>(this, &CompoundStatementRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CompoundStatementRule>(this, &CompoundStatementRule::A3Action));
        Cm::Parsing::NonterminalParser* stmtNonterminalParser = GetNonterminal("stmt");
        stmtNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<CompoundStatementRule>(this, &CompoundStatementRule::Prestmt));
        stmtNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<CompoundStatementRule>(this, &CompoundStatementRule::Poststmt));
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
    void Prestmt(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Poststmt(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromstmt_value = std::move(stack.top());
            context.fromstmt = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromstmt_value.get());
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

class StatementGrammar::ReturnStatementRule : public Cm::Parsing::Rule
{
public:
    ReturnStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ReturnStatementRule>(this, &ReturnStatementRule::A0Action));
        Cm::Parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ReturnStatementRule>(this, &ReturnStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ReturnStatementRule>(this, &ReturnStatementRule::PostExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ReturnStatementNode(span, context.fromExpression);
    }
    void PreExpression(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromExpression_value.get());
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

class StatementGrammar::IfStatementRule : public Cm::Parsing::Rule
{
public:
    IfStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<IfStatementRule>(this, &IfStatementRule::A0Action));
        Cm::Parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<IfStatementRule>(this, &IfStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<IfStatementRule>(this, &IfStatementRule::PostExpression));
        Cm::Parsing::NonterminalParser* thensNonterminalParser = GetNonterminal("thens");
        thensNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<IfStatementRule>(this, &IfStatementRule::Prethens));
        thensNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<IfStatementRule>(this, &IfStatementRule::Postthens));
        Cm::Parsing::NonterminalParser* elsesNonterminalParser = GetNonterminal("elses");
        elsesNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<IfStatementRule>(this, &IfStatementRule::Preelses));
        elsesNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<IfStatementRule>(this, &IfStatementRule::Postelses));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new IfStatementNode(span, context.fromExpression, context.fromthens, context.fromelses);
    }
    void PreExpression(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void Prethens(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postthens(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromthens_value = std::move(stack.top());
            context.fromthens = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromthens_value.get());
            stack.pop();
        }
    }
    void Preelses(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postelses(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromelses_value = std::move(stack.top());
            context.fromelses = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromelses_value.get());
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

class StatementGrammar::WhileStatementRule : public Cm::Parsing::Rule
{
public:
    WhileStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<WhileStatementRule>(this, &WhileStatementRule::A0Action));
        Cm::Parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<WhileStatementRule>(this, &WhileStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<WhileStatementRule>(this, &WhileStatementRule::PostExpression));
        Cm::Parsing::NonterminalParser* statementNonterminalParser = GetNonterminal("Statement");
        statementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<WhileStatementRule>(this, &WhileStatementRule::PreStatement));
        statementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<WhileStatementRule>(this, &WhileStatementRule::PostStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new WhileStatementNode(span, context.fromExpression, context.fromStatement);
    }
    void PreExpression(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PreStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromStatement_value = std::move(stack.top());
            context.fromStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromStatement_value.get());
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

class StatementGrammar::DoStatementRule : public Cm::Parsing::Rule
{
public:
    DoStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<DoStatementRule>(this, &DoStatementRule::A0Action));
        Cm::Parsing::NonterminalParser* statementNonterminalParser = GetNonterminal("Statement");
        statementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<DoStatementRule>(this, &DoStatementRule::PreStatement));
        statementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<DoStatementRule>(this, &DoStatementRule::PostStatement));
        Cm::Parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<DoStatementRule>(this, &DoStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<DoStatementRule>(this, &DoStatementRule::PostExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new DoStatementNode(span, context.fromStatement, context.fromExpression);
    }
    void PreStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromStatement_value = std::move(stack.top());
            context.fromStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromStatement_value.get());
            stack.pop();
        }
    }
    void PreExpression(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromExpression_value.get());
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

class StatementGrammar::ForStatementRule : public Cm::Parsing::Rule
{
public:
    ForStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ForStatementRule>(this, &ForStatementRule::A0Action));
        Cm::Parsing::NonterminalParser* forInitStatementNonterminalParser = GetNonterminal("ForInitStatement");
        forInitStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ForStatementRule>(this, &ForStatementRule::PreForInitStatement));
        forInitStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ForStatementRule>(this, &ForStatementRule::PostForInitStatement));
        Cm::Parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ForStatementRule>(this, &ForStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ForStatementRule>(this, &ForStatementRule::PostExpression));
        Cm::Parsing::NonterminalParser* forLoopExpressionStatementNonterminalParser = GetNonterminal("ForLoopExpressionStatement");
        forLoopExpressionStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ForStatementRule>(this, &ForStatementRule::PreForLoopExpressionStatement));
        forLoopExpressionStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ForStatementRule>(this, &ForStatementRule::PostForLoopExpressionStatement));
        Cm::Parsing::NonterminalParser* statementNonterminalParser = GetNonterminal("Statement");
        statementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ForStatementRule>(this, &ForStatementRule::PreStatement));
        statementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ForStatementRule>(this, &ForStatementRule::PostStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ForStatementNode(span, context.fromForInitStatement, context.fromExpression, context.fromForLoopExpressionStatement, context.fromStatement);
    }
    void PreForInitStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostForInitStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromForInitStatement_value = std::move(stack.top());
            context.fromForInitStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromForInitStatement_value.get());
            stack.pop();
        }
    }
    void PreExpression(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PreForLoopExpressionStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostForLoopExpressionStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromForLoopExpressionStatement_value = std::move(stack.top());
            context.fromForLoopExpressionStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromForLoopExpressionStatement_value.get());
            stack.pop();
        }
    }
    void PreStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromStatement_value = std::move(stack.top());
            context.fromStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromStatement_value.get());
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

class StatementGrammar::ForInitStatementRule : public Cm::Parsing::Rule
{
public:
    ForInitStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ForInitStatementRule>(this, &ForInitStatementRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ForInitStatementRule>(this, &ForInitStatementRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ForInitStatementRule>(this, &ForInitStatementRule::A2Action));
        Cm::Parsing::NonterminalParser* assignmentStatementNonterminalParser = GetNonterminal("AssignmentStatement");
        assignmentStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ForInitStatementRule>(this, &ForInitStatementRule::PreAssignmentStatement));
        assignmentStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ForInitStatementRule>(this, &ForInitStatementRule::PostAssignmentStatement));
        Cm::Parsing::NonterminalParser* constructionStatementNonterminalParser = GetNonterminal("ConstructionStatement");
        constructionStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ForInitStatementRule>(this, &ForInitStatementRule::PreConstructionStatement));
        constructionStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ForInitStatementRule>(this, &ForInitStatementRule::PostConstructionStatement));
        Cm::Parsing::NonterminalParser* emptyStatementNonterminalParser = GetNonterminal("EmptyStatement");
        emptyStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ForInitStatementRule>(this, &ForInitStatementRule::PreEmptyStatement));
        emptyStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ForInitStatementRule>(this, &ForInitStatementRule::PostEmptyStatement));
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
    void PreAssignmentStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostAssignmentStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromAssignmentStatement_value = std::move(stack.top());
            context.fromAssignmentStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromAssignmentStatement_value.get());
            stack.pop();
        }
    }
    void PreConstructionStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostConstructionStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromConstructionStatement_value = std::move(stack.top());
            context.fromConstructionStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromConstructionStatement_value.get());
            stack.pop();
        }
    }
    void PreEmptyStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostEmptyStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromEmptyStatement_value = std::move(stack.top());
            context.fromEmptyStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromEmptyStatement_value.get());
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
        StatementNode* fromConstructionStatement;
        StatementNode* fromEmptyStatement;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::ForLoopExpressionStatementRule : public Cm::Parsing::Rule
{
public:
    ForLoopExpressionStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ForLoopExpressionStatementRule>(this, &ForLoopExpressionStatementRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ForLoopExpressionStatementRule>(this, &ForLoopExpressionStatementRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ForLoopExpressionStatementRule>(this, &ForLoopExpressionStatementRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ForLoopExpressionStatementRule>(this, &ForLoopExpressionStatementRule::A3Action));
        Cm::Parsing::NonterminalParser* incrementExpressionStatementNonterminalParser = GetNonterminal("IncrementExpressionStatement");
        incrementExpressionStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ForLoopExpressionStatementRule>(this, &ForLoopExpressionStatementRule::PreIncrementExpressionStatement));
        incrementExpressionStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ForLoopExpressionStatementRule>(this, &ForLoopExpressionStatementRule::PostIncrementExpressionStatement));
        Cm::Parsing::NonterminalParser* decrementExpressionStatementNonterminalParser = GetNonterminal("DecrementExpressionStatement");
        decrementExpressionStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ForLoopExpressionStatementRule>(this, &ForLoopExpressionStatementRule::PreDecrementExpressionStatement));
        decrementExpressionStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ForLoopExpressionStatementRule>(this, &ForLoopExpressionStatementRule::PostDecrementExpressionStatement));
        Cm::Parsing::NonterminalParser* assignmentExpressionStatementNonterminalParser = GetNonterminal("AssignmentExpressionStatement");
        assignmentExpressionStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ForLoopExpressionStatementRule>(this, &ForLoopExpressionStatementRule::PreAssignmentExpressionStatement));
        assignmentExpressionStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ForLoopExpressionStatementRule>(this, &ForLoopExpressionStatementRule::PostAssignmentExpressionStatement));
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
    void PreIncrementExpressionStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostIncrementExpressionStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromIncrementExpressionStatement_value = std::move(stack.top());
            context.fromIncrementExpressionStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromIncrementExpressionStatement_value.get());
            stack.pop();
        }
    }
    void PreDecrementExpressionStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostDecrementExpressionStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromDecrementExpressionStatement_value = std::move(stack.top());
            context.fromDecrementExpressionStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromDecrementExpressionStatement_value.get());
            stack.pop();
        }
    }
    void PreAssignmentExpressionStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostAssignmentExpressionStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromAssignmentExpressionStatement_value = std::move(stack.top());
            context.fromAssignmentExpressionStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromAssignmentExpressionStatement_value.get());
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

class StatementGrammar::ForEachStatementRule : public Cm::Parsing::Rule
{
public:
    ForEachStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ForEachStatementRule>(this, &ForEachStatementRule::A0Action));
        Cm::Parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal("TypeExpr");
        typeExprNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ForEachStatementRule>(this, &ForEachStatementRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ForEachStatementRule>(this, &ForEachStatementRule::PostTypeExpr));
        Cm::Parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ForEachStatementRule>(this, &ForEachStatementRule::PostIdentifier));
        Cm::Parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ForEachStatementRule>(this, &ForEachStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ForEachStatementRule>(this, &ForEachStatementRule::PostExpression));
        Cm::Parsing::NonterminalParser* compoundStatementNonterminalParser = GetNonterminal("CompoundStatement");
        compoundStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ForEachStatementRule>(this, &ForEachStatementRule::PreCompoundStatement));
        compoundStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ForEachStatementRule>(this, &ForEachStatementRule::PostCompoundStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ForEachStatementNode(span, context.fromTypeExpr, context.fromIdentifier, context.fromExpression, context.fromCompoundStatement);
    }
    void PreTypeExpr(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostTypeExpr(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromTypeExpr_value = std::move(stack.top());
            context.fromTypeExpr = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromTypeExpr_value.get());
            stack.pop();
        }
    }
    void PostIdentifier(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromIdentifier_value = std::move(stack.top());
            context.fromIdentifier = *static_cast<Cm::Parsing::ValueObject<IdentifierNode*>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
    void PreExpression(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PreCompoundStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostCompoundStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromCompoundStatement_value = std::move(stack.top());
            context.fromCompoundStatement = *static_cast<Cm::Parsing::ValueObject<CompoundStatementNode*>*>(fromCompoundStatement_value.get());
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

class StatementGrammar::BreakStatementRule : public Cm::Parsing::Rule
{
public:
    BreakStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BreakStatementRule>(this, &BreakStatementRule::A0Action));
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

class StatementGrammar::ContinueStatementRule : public Cm::Parsing::Rule
{
public:
    ContinueStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ContinueStatementRule>(this, &ContinueStatementRule::A0Action));
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

class StatementGrammar::SwitchStatementRule : public Cm::Parsing::Rule
{
public:
    SwitchStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("SwitchStatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<SwitchStatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SwitchStatementRule>(this, &SwitchStatementRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SwitchStatementRule>(this, &SwitchStatementRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SwitchStatementRule>(this, &SwitchStatementRule::A2Action));
        Cm::Parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<SwitchStatementRule>(this, &SwitchStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<SwitchStatementRule>(this, &SwitchStatementRule::PostExpression));
        Cm::Parsing::NonterminalParser* caseStatementNonterminalParser = GetNonterminal("CaseStatement");
        caseStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<SwitchStatementRule>(this, &SwitchStatementRule::PreCaseStatement));
        caseStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<SwitchStatementRule>(this, &SwitchStatementRule::PostCaseStatement));
        Cm::Parsing::NonterminalParser* defaultStatementNonterminalParser = GetNonterminal("DefaultStatement");
        defaultStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<SwitchStatementRule>(this, &SwitchStatementRule::PreDefaultStatement));
        defaultStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<SwitchStatementRule>(this, &SwitchStatementRule::PostDefaultStatement));
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
    void PreExpression(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PreCaseStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostCaseStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromCaseStatement_value = std::move(stack.top());
            context.fromCaseStatement = *static_cast<Cm::Parsing::ValueObject<CaseStatementNode*>*>(fromCaseStatement_value.get());
            stack.pop();
        }
    }
    void PreDefaultStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostDefaultStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromDefaultStatement_value = std::move(stack.top());
            context.fromDefaultStatement = *static_cast<Cm::Parsing::ValueObject<DefaultStatementNode*>*>(fromDefaultStatement_value.get());
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

class StatementGrammar::CaseStatementRule : public Cm::Parsing::Rule
{
public:
    CaseStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("CaseStatementNode*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<CaseStatementNode>", "caseS"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<CaseStatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CaseStatementRule>(this, &CaseStatementRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CaseStatementRule>(this, &CaseStatementRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CaseStatementRule>(this, &CaseStatementRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CaseStatementRule>(this, &CaseStatementRule::A3Action));
        Cm::Parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CaseStatementRule>(this, &CaseStatementRule::A4Action));
        Cm::Parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<CaseStatementRule>(this, &CaseStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<CaseStatementRule>(this, &CaseStatementRule::PostExpression));
        Cm::Parsing::NonterminalParser* statementNonterminalParser = GetNonterminal("Statement");
        statementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<CaseStatementRule>(this, &CaseStatementRule::PreStatement));
        statementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<CaseStatementRule>(this, &CaseStatementRule::PostStatement));
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
    void PreExpression(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PreStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromStatement_value = std::move(stack.top());
            context.fromStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromStatement_value.get());
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

class StatementGrammar::DefaultStatementRule : public Cm::Parsing::Rule
{
public:
    DefaultStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("DefaultStatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<DefaultStatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<DefaultStatementRule>(this, &DefaultStatementRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<DefaultStatementRule>(this, &DefaultStatementRule::A1Action));
        Cm::Parsing::NonterminalParser* statementNonterminalParser = GetNonterminal("Statement");
        statementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<DefaultStatementRule>(this, &DefaultStatementRule::PreStatement));
        statementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<DefaultStatementRule>(this, &DefaultStatementRule::PostStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new DefaultStatementNode(span);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value->AddStatement(context.fromStatement);
    }
    void PreStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromStatement_value = std::move(stack.top());
            context.fromStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromStatement_value.get());
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

class StatementGrammar::GotoCaseStatementRule : public Cm::Parsing::Rule
{
public:
    GotoCaseStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<GotoCaseStatementRule>(this, &GotoCaseStatementRule::A0Action));
        Cm::Parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<GotoCaseStatementRule>(this, &GotoCaseStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<GotoCaseStatementRule>(this, &GotoCaseStatementRule::PostExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new GotoCaseStatementNode(span, context.fromExpression);
    }
    void PreExpression(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromExpression_value.get());
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

class StatementGrammar::GotoDefaultStatementRule : public Cm::Parsing::Rule
{
public:
    GotoDefaultStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<GotoDefaultStatementRule>(this, &GotoDefaultStatementRule::A0Action));
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

class StatementGrammar::AssignmentExpressionStatementRule : public Cm::Parsing::Rule
{
public:
    AssignmentExpressionStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "targetExpr"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<AssignmentExpressionStatementRule>(this, &AssignmentExpressionStatementRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<AssignmentExpressionStatementRule>(this, &AssignmentExpressionStatementRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<AssignmentExpressionStatementRule>(this, &AssignmentExpressionStatementRule::A2Action));
        a2ActionParser->SetFailureAction(new Cm::Parsing::MemberFailureAction<AssignmentExpressionStatementRule>(this, &AssignmentExpressionStatementRule::A2ActionFail));
        Cm::Parsing::NonterminalParser* targetNonterminalParser = GetNonterminal("target");
        targetNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<AssignmentExpressionStatementRule>(this, &AssignmentExpressionStatementRule::Pretarget));
        targetNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<AssignmentExpressionStatementRule>(this, &AssignmentExpressionStatementRule::Posttarget));
        Cm::Parsing::NonterminalParser* sourceNonterminalParser = GetNonterminal("source");
        sourceNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<AssignmentExpressionStatementRule>(this, &AssignmentExpressionStatementRule::Presource));
        sourceNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<AssignmentExpressionStatementRule>(this, &AssignmentExpressionStatementRule::Postsource));
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
    void Pretarget(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Posttarget(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromtarget_value = std::move(stack.top());
            context.fromtarget = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromtarget_value.get());
            stack.pop();
        }
    }
    void Presource(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postsource(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromsource_value = std::move(stack.top());
            context.fromsource = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromsource_value.get());
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

class StatementGrammar::AssignmentStatementRule : public Cm::Parsing::Rule
{
public:
    AssignmentStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<AssignmentStatementRule>(this, &AssignmentStatementRule::A0Action));
        Cm::Parsing::NonterminalParser* assignmentExpressionStatementNonterminalParser = GetNonterminal("AssignmentExpressionStatement");
        assignmentExpressionStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<AssignmentStatementRule>(this, &AssignmentStatementRule::PreAssignmentExpressionStatement));
        assignmentExpressionStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<AssignmentStatementRule>(this, &AssignmentStatementRule::PostAssignmentExpressionStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromAssignmentExpressionStatement;
        context.value->GetSpan().SetEnd(span.End());
    }
    void PreAssignmentExpressionStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostAssignmentExpressionStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromAssignmentExpressionStatement_value = std::move(stack.top());
            context.fromAssignmentExpressionStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromAssignmentExpressionStatement_value.get());
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

class StatementGrammar::ConstructionStatementRule : public Cm::Parsing::Rule
{
public:
    ConstructionStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ConstructionStatementRule>(this, &ConstructionStatementRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ConstructionStatementRule>(this, &ConstructionStatementRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ConstructionStatementRule>(this, &ConstructionStatementRule::A2Action));
        Cm::Parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal("TypeExpr");
        typeExprNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ConstructionStatementRule>(this, &ConstructionStatementRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ConstructionStatementRule>(this, &ConstructionStatementRule::PostTypeExpr));
        Cm::Parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ConstructionStatementRule>(this, &ConstructionStatementRule::PostIdentifier));
        Cm::Parsing::NonterminalParser* argumentListNonterminalParser = GetNonterminal("ArgumentList");
        argumentListNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ConstructionStatementRule>(this, &ConstructionStatementRule::PreArgumentList));
        Cm::Parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ConstructionStatementRule>(this, &ConstructionStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ConstructionStatementRule>(this, &ConstructionStatementRule::PostExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ConstructionStatementNode(span, context.fromTypeExpr, context.fromIdentifier);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value->AddArgument(context.fromExpression);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value->GetSpan().SetEnd(span.End());
    }
    void PreTypeExpr(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostTypeExpr(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromTypeExpr_value = std::move(stack.top());
            context.fromTypeExpr = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromTypeExpr_value.get());
            stack.pop();
        }
    }
    void PostIdentifier(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromIdentifier_value = std::move(stack.top());
            context.fromIdentifier = *static_cast<Cm::Parsing::ValueObject<IdentifierNode*>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
    void PreArgumentList(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Node*>(context.value)));
    }
    void PreExpression(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromTypeExpr(), fromIdentifier(), fromExpression() {}
        ParsingContext* ctx;
        StatementNode* value;
        Node* fromTypeExpr;
        IdentifierNode* fromIdentifier;
        Node* fromExpression;
    };
    std::stack<Context> contextStack;
    Context context;
};

class StatementGrammar::IncrementExpressionStatementRule : public Cm::Parsing::Rule
{
public:
    IncrementExpressionStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<IncrementExpressionStatementRule>(this, &IncrementExpressionStatementRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<IncrementExpressionStatementRule>(this, &IncrementExpressionStatementRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<IncrementExpressionStatementRule>(this, &IncrementExpressionStatementRule::A2Action));
        Cm::Parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<IncrementExpressionStatementRule>(this, &IncrementExpressionStatementRule::Preleft));
        leftNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<IncrementExpressionStatementRule>(this, &IncrementExpressionStatementRule::Postleft));
        Cm::Parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<IncrementExpressionStatementRule>(this, &IncrementExpressionStatementRule::Preright));
        rightNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<IncrementExpressionStatementRule>(this, &IncrementExpressionStatementRule::Postright));
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
    void Preleft(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postleft(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postright(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromright_value.get());
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

class StatementGrammar::IncrementStatementRule : public Cm::Parsing::Rule
{
public:
    IncrementStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<IncrementStatementRule>(this, &IncrementStatementRule::A0Action));
        Cm::Parsing::NonterminalParser* incrementExpressionStatementNonterminalParser = GetNonterminal("IncrementExpressionStatement");
        incrementExpressionStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<IncrementStatementRule>(this, &IncrementStatementRule::PreIncrementExpressionStatement));
        incrementExpressionStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<IncrementStatementRule>(this, &IncrementStatementRule::PostIncrementExpressionStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromIncrementExpressionStatement;
        context.value->GetSpan().SetEnd(span.End());
    }
    void PreIncrementExpressionStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostIncrementExpressionStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromIncrementExpressionStatement_value = std::move(stack.top());
            context.fromIncrementExpressionStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromIncrementExpressionStatement_value.get());
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

class StatementGrammar::DecrementExpressionStatementRule : public Cm::Parsing::Rule
{
public:
    DecrementExpressionStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<DecrementExpressionStatementRule>(this, &DecrementExpressionStatementRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<DecrementExpressionStatementRule>(this, &DecrementExpressionStatementRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<DecrementExpressionStatementRule>(this, &DecrementExpressionStatementRule::A2Action));
        Cm::Parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<DecrementExpressionStatementRule>(this, &DecrementExpressionStatementRule::Preleft));
        leftNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<DecrementExpressionStatementRule>(this, &DecrementExpressionStatementRule::Postleft));
        Cm::Parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<DecrementExpressionStatementRule>(this, &DecrementExpressionStatementRule::Preright));
        rightNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<DecrementExpressionStatementRule>(this, &DecrementExpressionStatementRule::Postright));
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
    void Preleft(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postleft(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postright(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromright_value.get());
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

class StatementGrammar::DecrementStatementRule : public Cm::Parsing::Rule
{
public:
    DecrementStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<DecrementStatementRule>(this, &DecrementStatementRule::A0Action));
        Cm::Parsing::NonterminalParser* decrementExpressionStatementNonterminalParser = GetNonterminal("DecrementExpressionStatement");
        decrementExpressionStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<DecrementStatementRule>(this, &DecrementStatementRule::PreDecrementExpressionStatement));
        decrementExpressionStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<DecrementStatementRule>(this, &DecrementStatementRule::PostDecrementExpressionStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromDecrementExpressionStatement;
        context.value->GetSpan().SetEnd(span.End());
    }
    void PreDecrementExpressionStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostDecrementExpressionStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromDecrementExpressionStatement_value = std::move(stack.top());
            context.fromDecrementExpressionStatement = *static_cast<Cm::Parsing::ValueObject<StatementNode*>*>(fromDecrementExpressionStatement_value.get());
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

class StatementGrammar::ExpressionStatementRule : public Cm::Parsing::Rule
{
public:
    ExpressionStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ExpressionStatementRule>(this, &ExpressionStatementRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ExpressionStatementRule>(this, &ExpressionStatementRule::A1Action));
        Cm::Parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ExpressionStatementRule>(this, &ExpressionStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ExpressionStatementRule>(this, &ExpressionStatementRule::PostExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ExpressionStatementNode(span, context.expr.release());
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.expr.reset(context.fromExpression);
    }
    void PreExpression(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromExpression_value.get());
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

class StatementGrammar::EmptyStatementRule : public Cm::Parsing::Rule
{
public:
    EmptyStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<EmptyStatementRule>(this, &EmptyStatementRule::A0Action));
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

class StatementGrammar::ThrowStatementRule : public Cm::Parsing::Rule
{
public:
    ThrowStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("StatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ThrowStatementRule>(this, &ThrowStatementRule::A0Action));
        Cm::Parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ThrowStatementRule>(this, &ThrowStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ThrowStatementRule>(this, &ThrowStatementRule::PostExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ThrowStatementNode(span, context.fromExpression);
    }
    void PreExpression(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromExpression_value.get());
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

class StatementGrammar::TryStatementRule : public Cm::Parsing::Rule
{
public:
    TryStatementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("TryStatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<TryStatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<TryStatementRule>(this, &TryStatementRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<TryStatementRule>(this, &TryStatementRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<TryStatementRule>(this, &TryStatementRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<TryStatementRule>(this, &TryStatementRule::A3Action));
        Cm::Parsing::NonterminalParser* tryBlockNonterminalParser = GetNonterminal("tryBlock");
        tryBlockNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<TryStatementRule>(this, &TryStatementRule::PretryBlock));
        tryBlockNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<TryStatementRule>(this, &TryStatementRule::PosttryBlock));
        Cm::Parsing::NonterminalParser* catchNonterminalParser = GetNonterminal("Catch");
        catchNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<TryStatementRule>(this, &TryStatementRule::PreCatch));
        catchNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<TryStatementRule>(this, &TryStatementRule::PostCatch));
        Cm::Parsing::NonterminalParser* finallyNonterminalParser = GetNonterminal("Finally");
        finallyNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<TryStatementRule>(this, &TryStatementRule::PreFinally));
        finallyNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<TryStatementRule>(this, &TryStatementRule::PostFinally));
        Cm::Parsing::NonterminalParser* finNonterminalParser = GetNonterminal("fin");
        finNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<TryStatementRule>(this, &TryStatementRule::Prefin));
        finNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<TryStatementRule>(this, &TryStatementRule::Postfin));
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
    void PretryBlock(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PosttryBlock(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromtryBlock_value = std::move(stack.top());
            context.fromtryBlock = *static_cast<Cm::Parsing::ValueObject<CompoundStatementNode*>*>(fromtryBlock_value.get());
            stack.pop();
        }
    }
    void PreCatch(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostCatch(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromCatch_value = std::move(stack.top());
            context.fromCatch = *static_cast<Cm::Parsing::ValueObject<CatchNode*>*>(fromCatch_value.get());
            stack.pop();
        }
    }
    void PreFinally(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostFinally(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromFinally_value = std::move(stack.top());
            context.fromFinally = *static_cast<Cm::Parsing::ValueObject<CompoundStatementNode*>*>(fromFinally_value.get());
            stack.pop();
        }
    }
    void Prefin(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postfin(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromfin_value = std::move(stack.top());
            context.fromfin = *static_cast<Cm::Parsing::ValueObject<CompoundStatementNode*>*>(fromfin_value.get());
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

class StatementGrammar::CatchRule : public Cm::Parsing::Rule
{
public:
    CatchRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("CatchNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<CatchNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CatchRule>(this, &CatchRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CatchRule>(this, &CatchRule::A1Action));
        Cm::Parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal("TypeExpr");
        typeExprNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<CatchRule>(this, &CatchRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<CatchRule>(this, &CatchRule::PostTypeExpr));
        Cm::Parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<CatchRule>(this, &CatchRule::PostIdentifier));
        Cm::Parsing::NonterminalParser* catchBlock1NonterminalParser = GetNonterminal("catchBlock1");
        catchBlock1NonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<CatchRule>(this, &CatchRule::PrecatchBlock1));
        catchBlock1NonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<CatchRule>(this, &CatchRule::PostcatchBlock1));
        Cm::Parsing::NonterminalParser* catchBlock2NonterminalParser = GetNonterminal("catchBlock2");
        catchBlock2NonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<CatchRule>(this, &CatchRule::PrecatchBlock2));
        catchBlock2NonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<CatchRule>(this, &CatchRule::PostcatchBlock2));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new CatchNode(span, context.fromTypeExpr, context.fromIdentifier, context.fromcatchBlock1);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new CatchNode(span, context.fromcatchBlock2);
    }
    void PreTypeExpr(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostTypeExpr(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromTypeExpr_value = std::move(stack.top());
            context.fromTypeExpr = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromTypeExpr_value.get());
            stack.pop();
        }
    }
    void PostIdentifier(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromIdentifier_value = std::move(stack.top());
            context.fromIdentifier = *static_cast<Cm::Parsing::ValueObject<IdentifierNode*>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
    void PrecatchBlock1(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostcatchBlock1(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromcatchBlock1_value = std::move(stack.top());
            context.fromcatchBlock1 = *static_cast<Cm::Parsing::ValueObject<CompoundStatementNode*>*>(fromcatchBlock1_value.get());
            stack.pop();
        }
    }
    void PrecatchBlock2(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostcatchBlock2(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromcatchBlock2_value = std::move(stack.top());
            context.fromcatchBlock2 = *static_cast<Cm::Parsing::ValueObject<CompoundStatementNode*>*>(fromcatchBlock2_value.get());
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

class StatementGrammar::FinallyRule : public Cm::Parsing::Rule
{
public:
    FinallyRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("CompoundStatementNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<CompoundStatementNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<FinallyRule>(this, &FinallyRule::A0Action));
        Cm::Parsing::NonterminalParser* compoundStatementNonterminalParser = GetNonterminal("CompoundStatement");
        compoundStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<FinallyRule>(this, &FinallyRule::PreCompoundStatement));
        compoundStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<FinallyRule>(this, &FinallyRule::PostCompoundStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromCompoundStatement;
    }
    void PreCompoundStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostCompoundStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromCompoundStatement_value = std::move(stack.top());
            context.fromCompoundStatement = *static_cast<Cm::Parsing::ValueObject<CompoundStatementNode*>*>(fromCompoundStatement_value.get());
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

void StatementGrammar::GetReferencedGrammars()
{
    Cm::Parsing::ParsingDomain* pd = GetParsingDomain();
    Cm::Parsing::Grammar* grammar0 = pd->GetGrammar("cminor.parser.ExpressionGrammar");
    if (!grammar0)
    {
        grammar0 = cminor::parser::ExpressionGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    Cm::Parsing::Grammar* grammar1 = pd->GetGrammar("cminor.parser.KeywordGrammar");
    if (!grammar1)
    {
        grammar1 = cminor::parser::KeywordGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    Cm::Parsing::Grammar* grammar2 = pd->GetGrammar("Cm.Parsing.stdlib");
    if (!grammar2)
    {
        grammar2 = Cm::Parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar2);
    Cm::Parsing::Grammar* grammar3 = pd->GetGrammar("cminor.parser.TypeExprGrammar");
    if (!grammar3)
    {
        grammar3 = cminor::parser::TypeExprGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
    Cm::Parsing::Grammar* grammar4 = pd->GetGrammar("cminor.parser.IdentifierGrammar");
    if (!grammar4)
    {
        grammar4 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar4);
}

void StatementGrammar::CreateRules()
{
    AddRuleLink(new Cm::Parsing::RuleLink("Expression", this, "ExpressionGrammar.Expression"));
    AddRuleLink(new Cm::Parsing::RuleLink("identifier", this, "Cm.Parsing.stdlib.identifier"));
    AddRuleLink(new Cm::Parsing::RuleLink("ArgumentList", this, "ExpressionGrammar.ArgumentList"));
    AddRuleLink(new Cm::Parsing::RuleLink("Keyword", this, "KeywordGrammar.Keyword"));
    AddRuleLink(new Cm::Parsing::RuleLink("TypeExpr", this, "TypeExprGrammar.TypeExpr"));
    AddRuleLink(new Cm::Parsing::RuleLink("Identifier", this, "IdentifierGrammar.Identifier"));
    AddRule(new StatementRule("Statement", GetScope(),
        new Cm::Parsing::AlternativeParser(
            new Cm::Parsing::AlternativeParser(
                new Cm::Parsing::AlternativeParser(
                    new Cm::Parsing::AlternativeParser(
                        new Cm::Parsing::AlternativeParser(
                            new Cm::Parsing::AlternativeParser(
                                new Cm::Parsing::AlternativeParser(
                                    new Cm::Parsing::AlternativeParser(
                                        new Cm::Parsing::AlternativeParser(
                                            new Cm::Parsing::ActionParser("A0",
                                                new Cm::Parsing::NonterminalParser("LabeledStatement", "LabeledStatement", 1)),
                                            new Cm::Parsing::ActionParser("A1",
                                                new Cm::Parsing::NonterminalParser("ControlStatement", "ControlStatement", 1))),
                                        new Cm::Parsing::ActionParser("A2",
                                            new Cm::Parsing::NonterminalParser("IncrementStatement", "IncrementStatement", 1))),
                                    new Cm::Parsing::ActionParser("A3",
                                        new Cm::Parsing::NonterminalParser("DecrementStatement", "DecrementStatement", 1))),
                                new Cm::Parsing::ActionParser("A4",
                                    new Cm::Parsing::NonterminalParser("ExpressionStatement", "ExpressionStatement", 1))),
                            new Cm::Parsing::ActionParser("A5",
                                new Cm::Parsing::NonterminalParser("AssignmentStatement", "AssignmentStatement", 1))),
                        new Cm::Parsing::ActionParser("A6",
                            new Cm::Parsing::NonterminalParser("ConstructionStatement", "ConstructionStatement", 1))),
                    new Cm::Parsing::ActionParser("A7",
                        new Cm::Parsing::NonterminalParser("EmptyStatement", "EmptyStatement", 1))),
                new Cm::Parsing::ActionParser("A8",
                    new Cm::Parsing::NonterminalParser("ThrowStatement", "ThrowStatement", 1))),
            new Cm::Parsing::ActionParser("A9",
                new Cm::Parsing::NonterminalParser("TryStatement", "TryStatement", 1)))));
    AddRule(new LabelIdRule("LabelId", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::DifferenceParser(
                new Cm::Parsing::NonterminalParser("identifier", "identifier", 0),
                new Cm::Parsing::NonterminalParser("Keyword", "Keyword", 0)))));
    AddRule(new LabelRule("Label", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::ActionParser("A1",
                    new Cm::Parsing::NonterminalParser("LabelId", "LabelId", 0)),
                new Cm::Parsing::CharParser(':')))));
    AddRule(new LabeledStatementRule("LabeledStatement", GetScope(),
        new Cm::Parsing::SequenceParser(
            new Cm::Parsing::NonterminalParser("lbl", "Label", 0),
            new Cm::Parsing::ActionParser("A0",
                new Cm::Parsing::NonterminalParser("stmt", "Statement", 1)))));
    AddRule(new ControlStatementRule("ControlStatement", GetScope(),
        new Cm::Parsing::AlternativeParser(
            new Cm::Parsing::AlternativeParser(
                new Cm::Parsing::AlternativeParser(
                    new Cm::Parsing::AlternativeParser(
                        new Cm::Parsing::AlternativeParser(
                            new Cm::Parsing::AlternativeParser(
                                new Cm::Parsing::AlternativeParser(
                                    new Cm::Parsing::AlternativeParser(
                                        new Cm::Parsing::AlternativeParser(
                                            new Cm::Parsing::AlternativeParser(
                                                new Cm::Parsing::AlternativeParser(
                                                    new Cm::Parsing::ActionParser("A0",
                                                        new Cm::Parsing::NonterminalParser("CompoundStatement", "CompoundStatement", 1)),
                                                    new Cm::Parsing::ActionParser("A1",
                                                        new Cm::Parsing::NonterminalParser("ReturnStatement", "ReturnStatement", 1))),
                                                new Cm::Parsing::ActionParser("A2",
                                                    new Cm::Parsing::NonterminalParser("IfStatement", "IfStatement", 1))),
                                            new Cm::Parsing::ActionParser("A3",
                                                new Cm::Parsing::NonterminalParser("WhileStatement", "WhileStatement", 1))),
                                        new Cm::Parsing::ActionParser("A4",
                                            new Cm::Parsing::NonterminalParser("DoStatement", "DoStatement", 1))),
                                    new Cm::Parsing::ActionParser("A5",
                                        new Cm::Parsing::NonterminalParser("ForStatement", "ForStatement", 1))),
                                new Cm::Parsing::ActionParser("A6",
                                    new Cm::Parsing::NonterminalParser("ForEachStatement", "ForEachStatement", 1))),
                            new Cm::Parsing::ActionParser("A7",
                                new Cm::Parsing::NonterminalParser("BreakStatement", "BreakStatement", 1))),
                        new Cm::Parsing::ActionParser("A8",
                            new Cm::Parsing::NonterminalParser("ContinueStatement", "ContinueStatement", 1))),
                    new Cm::Parsing::ActionParser("A9",
                        new Cm::Parsing::NonterminalParser("SwitchStatement", "SwitchStatement", 1))),
                new Cm::Parsing::ActionParser("A10",
                    new Cm::Parsing::NonterminalParser("GotoCaseStatement", "GotoCaseStatement", 1))),
            new Cm::Parsing::ActionParser("A11",
                new Cm::Parsing::NonterminalParser("GotoDefaultStatement", "GotoDefaultStatement", 1)))));
    AddRule(new CompoundStatementRule("CompoundStatement", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::ActionParser("A1",
                        new Cm::Parsing::CharParser('{')),
                    new Cm::Parsing::KleeneStarParser(
                        new Cm::Parsing::ActionParser("A2",
                            new Cm::Parsing::NonterminalParser("stmt", "Statement", 1)))),
                new Cm::Parsing::ActionParser("A3",
                    new Cm::Parsing::ExpectationParser(
                        new Cm::Parsing::CharParser('}')))))));
    AddRule(new ReturnStatementRule("ReturnStatement", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::KeywordParser("return"),
                    new Cm::Parsing::OptionalParser(
                        new Cm::Parsing::NonterminalParser("Expression", "Expression", 1))),
                new Cm::Parsing::ExpectationParser(
                    new Cm::Parsing::CharParser(';'))))));
    AddRule(new IfStatementRule("IfStatement", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::SequenceParser(
                            new Cm::Parsing::SequenceParser(
                                new Cm::Parsing::KeywordParser("if"),
                                new Cm::Parsing::CharParser('(')),
                            new Cm::Parsing::NonterminalParser("Expression", "Expression", 1)),
                        new Cm::Parsing::CharParser(')')),
                    new Cm::Parsing::NonterminalParser("thens", "Statement", 1)),
                new Cm::Parsing::OptionalParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::KeywordParser("else"),
                        new Cm::Parsing::NonterminalParser("elses", "Statement", 1)))))));
    AddRule(new WhileStatementRule("WhileStatement", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::SequenceParser(
                            new Cm::Parsing::KeywordParser("while"),
                            new Cm::Parsing::ExpectationParser(
                                new Cm::Parsing::CharParser('('))),
                        new Cm::Parsing::ExpectationParser(
                            new Cm::Parsing::NonterminalParser("Expression", "Expression", 1))),
                    new Cm::Parsing::ExpectationParser(
                        new Cm::Parsing::CharParser(')'))),
                new Cm::Parsing::ExpectationParser(
                    new Cm::Parsing::NonterminalParser("Statement", "Statement", 1))))));
    AddRule(new DoStatementRule("DoStatement", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::SequenceParser(
                            new Cm::Parsing::SequenceParser(
                                new Cm::Parsing::SequenceParser(
                                    new Cm::Parsing::KeywordParser("do"),
                                    new Cm::Parsing::ExpectationParser(
                                        new Cm::Parsing::NonterminalParser("Statement", "Statement", 1))),
                                new Cm::Parsing::ExpectationParser(
                                    new Cm::Parsing::KeywordParser("while"))),
                            new Cm::Parsing::ExpectationParser(
                                new Cm::Parsing::CharParser('('))),
                        new Cm::Parsing::ExpectationParser(
                            new Cm::Parsing::NonterminalParser("Expression", "Expression", 1))),
                    new Cm::Parsing::ExpectationParser(
                        new Cm::Parsing::CharParser(')'))),
                new Cm::Parsing::ExpectationParser(
                    new Cm::Parsing::CharParser(';'))))));
    AddRule(new ForStatementRule("ForStatement", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::SequenceParser(
                            new Cm::Parsing::SequenceParser(
                                new Cm::Parsing::SequenceParser(
                                    new Cm::Parsing::SequenceParser(
                                        new Cm::Parsing::KeywordParser("for"),
                                        new Cm::Parsing::CharParser('(')),
                                    new Cm::Parsing::ExpectationParser(
                                        new Cm::Parsing::NonterminalParser("ForInitStatement", "ForInitStatement", 1))),
                                new Cm::Parsing::OptionalParser(
                                    new Cm::Parsing::NonterminalParser("Expression", "Expression", 1))),
                            new Cm::Parsing::ExpectationParser(
                                new Cm::Parsing::CharParser(';'))),
                        new Cm::Parsing::ExpectationParser(
                            new Cm::Parsing::NonterminalParser("ForLoopExpressionStatement", "ForLoopExpressionStatement", 1))),
                    new Cm::Parsing::ExpectationParser(
                        new Cm::Parsing::CharParser(')'))),
                new Cm::Parsing::ExpectationParser(
                    new Cm::Parsing::NonterminalParser("Statement", "Statement", 1))))));
    AddRule(new ForInitStatementRule("ForInitStatement", GetScope(),
        new Cm::Parsing::AlternativeParser(
            new Cm::Parsing::AlternativeParser(
                new Cm::Parsing::ActionParser("A0",
                    new Cm::Parsing::NonterminalParser("AssignmentStatement", "AssignmentStatement", 1)),
                new Cm::Parsing::ActionParser("A1",
                    new Cm::Parsing::NonterminalParser("ConstructionStatement", "ConstructionStatement", 1))),
            new Cm::Parsing::ActionParser("A2",
                new Cm::Parsing::NonterminalParser("EmptyStatement", "EmptyStatement", 1)))));
    AddRule(new ForLoopExpressionStatementRule("ForLoopExpressionStatement", GetScope(),
        new Cm::Parsing::AlternativeParser(
            new Cm::Parsing::AlternativeParser(
                new Cm::Parsing::AlternativeParser(
                    new Cm::Parsing::ActionParser("A0",
                        new Cm::Parsing::NonterminalParser("IncrementExpressionStatement", "IncrementExpressionStatement", 1)),
                    new Cm::Parsing::ActionParser("A1",
                        new Cm::Parsing::NonterminalParser("DecrementExpressionStatement", "DecrementExpressionStatement", 1))),
                new Cm::Parsing::ActionParser("A2",
                    new Cm::Parsing::NonterminalParser("AssignmentExpressionStatement", "AssignmentExpressionStatement", 1))),
            new Cm::Parsing::ActionParser("A3",
                new Cm::Parsing::EmptyParser()))));
    AddRule(new ForEachStatementRule("ForEachStatement", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::SequenceParser(
                            new Cm::Parsing::SequenceParser(
                                new Cm::Parsing::SequenceParser(
                                    new Cm::Parsing::SequenceParser(
                                        new Cm::Parsing::KeywordParser("foreach"),
                                        new Cm::Parsing::ExpectationParser(
                                            new Cm::Parsing::CharParser('('))),
                                    new Cm::Parsing::ExpectationParser(
                                        new Cm::Parsing::NonterminalParser("TypeExpr", "TypeExpr", 1))),
                                new Cm::Parsing::ExpectationParser(
                                    new Cm::Parsing::NonterminalParser("Identifier", "Identifier", 0))),
                            new Cm::Parsing::ExpectationParser(
                                new Cm::Parsing::KeywordParser("in"))),
                        new Cm::Parsing::ExpectationParser(
                            new Cm::Parsing::NonterminalParser("Expression", "Expression", 1))),
                    new Cm::Parsing::ExpectationParser(
                        new Cm::Parsing::CharParser(')'))),
                new Cm::Parsing::NonterminalParser("CompoundStatement", "CompoundStatement", 1)))));
    AddRule(new BreakStatementRule("BreakStatement", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::KeywordParser("break"),
                new Cm::Parsing::ExpectationParser(
                    new Cm::Parsing::CharParser(';'))))));
    AddRule(new ContinueStatementRule("ContinueStatement", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::KeywordParser("continue"),
                new Cm::Parsing::ExpectationParser(
                    new Cm::Parsing::CharParser(';'))))));
    AddRule(new SwitchStatementRule("SwitchStatement", GetScope(),
        new Cm::Parsing::SequenceParser(
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::ActionParser("A0",
                        new Cm::Parsing::SequenceParser(
                            new Cm::Parsing::SequenceParser(
                                new Cm::Parsing::SequenceParser(
                                    new Cm::Parsing::KeywordParser("switch"),
                                    new Cm::Parsing::ExpectationParser(
                                        new Cm::Parsing::CharParser('('))),
                                new Cm::Parsing::ExpectationParser(
                                    new Cm::Parsing::NonterminalParser("Expression", "Expression", 1))),
                            new Cm::Parsing::ExpectationParser(
                                new Cm::Parsing::CharParser(')')))),
                    new Cm::Parsing::ExpectationParser(
                        new Cm::Parsing::CharParser('{'))),
                new Cm::Parsing::KleeneStarParser(
                    new Cm::Parsing::AlternativeParser(
                        new Cm::Parsing::ActionParser("A1",
                            new Cm::Parsing::NonterminalParser("CaseStatement", "CaseStatement", 1)),
                        new Cm::Parsing::ActionParser("A2",
                            new Cm::Parsing::NonterminalParser("DefaultStatement", "DefaultStatement", 1))))),
            new Cm::Parsing::ExpectationParser(
                new Cm::Parsing::CharParser('}')))));
    AddRule(new CaseStatementRule("CaseStatement", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::ActionParser("A1",
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::ActionParser("A2",
                            new Cm::Parsing::EmptyParser()),
                        new Cm::Parsing::PositiveParser(
                            new Cm::Parsing::SequenceParser(
                                new Cm::Parsing::SequenceParser(
                                    new Cm::Parsing::KeywordParser("case"),
                                    new Cm::Parsing::ActionParser("A3",
                                        new Cm::Parsing::NonterminalParser("Expression", "Expression", 1))),
                                new Cm::Parsing::ExpectationParser(
                                    new Cm::Parsing::CharParser(':')))))),
                new Cm::Parsing::KleeneStarParser(
                    new Cm::Parsing::ActionParser("A4",
                        new Cm::Parsing::NonterminalParser("Statement", "Statement", 1)))))));
    AddRule(new DefaultStatementRule("DefaultStatement", GetScope(),
        new Cm::Parsing::SequenceParser(
            new Cm::Parsing::ActionParser("A0",
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::KeywordParser("default"),
                    new Cm::Parsing::ExpectationParser(
                        new Cm::Parsing::CharParser(':')))),
            new Cm::Parsing::KleeneStarParser(
                new Cm::Parsing::ActionParser("A1",
                    new Cm::Parsing::NonterminalParser("Statement", "Statement", 1))))));
    AddRule(new GotoCaseStatementRule("GotoCaseStatement", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::KeywordParser("goto"),
                        new Cm::Parsing::KeywordParser("case")),
                    new Cm::Parsing::ExpectationParser(
                        new Cm::Parsing::NonterminalParser("Expression", "Expression", 1))),
                new Cm::Parsing::ExpectationParser(
                    new Cm::Parsing::CharParser(';'))))));
    AddRule(new GotoDefaultStatementRule("GotoDefaultStatement", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::KeywordParser("goto"),
                    new Cm::Parsing::KeywordParser("default")),
                new Cm::Parsing::ExpectationParser(
                    new Cm::Parsing::CharParser(';'))))));
    AddRule(new AssignmentExpressionStatementRule("AssignmentExpressionStatement", GetScope(),
        new Cm::Parsing::SequenceParser(
            new Cm::Parsing::ActionParser("A0",
                new Cm::Parsing::EmptyParser()),
            new Cm::Parsing::ActionParser("A1",
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::ActionParser("A2",
                            new Cm::Parsing::NonterminalParser("target", "Expression", 1)),
                        new Cm::Parsing::CharParser('=')),
                    new Cm::Parsing::ExpectationParser(
                        new Cm::Parsing::NonterminalParser("source", "Expression", 1)))))));
    AddRule(new AssignmentStatementRule("AssignmentStatement", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::NonterminalParser("AssignmentExpressionStatement", "AssignmentExpressionStatement", 1),
                new Cm::Parsing::ExpectationParser(
                    new Cm::Parsing::CharParser(';'))))));
    AddRule(new ConstructionStatementRule("ConstructionStatement", GetScope(),
        new Cm::Parsing::SequenceParser(
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::NonterminalParser("TypeExpr", "TypeExpr", 1),
                    new Cm::Parsing::ActionParser("A0",
                        new Cm::Parsing::ExpectationParser(
                            new Cm::Parsing::NonterminalParser("Identifier", "Identifier", 0)))),
                new Cm::Parsing::OptionalParser(
                    new Cm::Parsing::AlternativeParser(
                        new Cm::Parsing::SequenceParser(
                            new Cm::Parsing::SequenceParser(
                                new Cm::Parsing::CharParser('('),
                                new Cm::Parsing::NonterminalParser("ArgumentList", "ArgumentList", 2)),
                            new Cm::Parsing::ExpectationParser(
                                new Cm::Parsing::CharParser(')'))),
                        new Cm::Parsing::SequenceParser(
                            new Cm::Parsing::CharParser('='),
                            new Cm::Parsing::ActionParser("A1",
                                new Cm::Parsing::ExpectationParser(
                                    new Cm::Parsing::NonterminalParser("Expression", "Expression", 1))))))),
            new Cm::Parsing::ActionParser("A2",
                new Cm::Parsing::ExpectationParser(
                    new Cm::Parsing::CharParser(';'))))));
    AddRule(new IncrementExpressionStatementRule("IncrementExpressionStatement", GetScope(),
        new Cm::Parsing::AlternativeParser(
            new Cm::Parsing::ActionParser("A0",
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::StringParser("++"),
                    new Cm::Parsing::ExpectationParser(
                        new Cm::Parsing::NonterminalParser("left", "Expression", 1)))),
            new Cm::Parsing::ActionParser("A1",
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::ActionParser("A2",
                        new Cm::Parsing::NonterminalParser("right", "Expression", 1)),
                    new Cm::Parsing::StringParser("++"))))));
    AddRule(new IncrementStatementRule("IncrementStatement", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::NonterminalParser("IncrementExpressionStatement", "IncrementExpressionStatement", 1),
                new Cm::Parsing::ExpectationParser(
                    new Cm::Parsing::CharParser(';'))))));
    AddRule(new DecrementExpressionStatementRule("DecrementExpressionStatement", GetScope(),
        new Cm::Parsing::AlternativeParser(
            new Cm::Parsing::ActionParser("A0",
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::StringParser("--"),
                    new Cm::Parsing::ExpectationParser(
                        new Cm::Parsing::NonterminalParser("left", "Expression", 1)))),
            new Cm::Parsing::ActionParser("A1",
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::ActionParser("A2",
                        new Cm::Parsing::NonterminalParser("right", "Expression", 1)),
                    new Cm::Parsing::StringParser("--"))))));
    AddRule(new DecrementStatementRule("DecrementStatement", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::NonterminalParser("DecrementExpressionStatement", "DecrementExpressionStatement", 1),
                new Cm::Parsing::ExpectationParser(
                    new Cm::Parsing::CharParser(';'))))));
    AddRule(new ExpressionStatementRule("ExpressionStatement", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::ActionParser("A1",
                    new Cm::Parsing::NonterminalParser("Expression", "Expression", 1)),
                new Cm::Parsing::CharParser(';')))));
    AddRule(new EmptyStatementRule("EmptyStatement", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::CharParser(';'))));
    AddRule(new ThrowStatementRule("ThrowStatement", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::KeywordParser("throw"),
                    new Cm::Parsing::OptionalParser(
                        new Cm::Parsing::NonterminalParser("Expression", "Expression", 1))),
                new Cm::Parsing::ExpectationParser(
                    new Cm::Parsing::CharParser(';'))))));
    AddRule(new TryStatementRule("TryStatement", GetScope(),
        new Cm::Parsing::SequenceParser(
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::KeywordParser("try"),
                new Cm::Parsing::ActionParser("A0",
                    new Cm::Parsing::NonterminalParser("tryBlock", "CompoundStatement", 1))),
            new Cm::Parsing::AlternativeParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::PositiveParser(
                        new Cm::Parsing::ActionParser("A1",
                            new Cm::Parsing::NonterminalParser("Catch", "Catch", 1))),
                    new Cm::Parsing::OptionalParser(
                        new Cm::Parsing::ActionParser("A2",
                            new Cm::Parsing::NonterminalParser("Finally", "Finally", 1)))),
                new Cm::Parsing::ActionParser("A3",
                    new Cm::Parsing::NonterminalParser("fin", "Finally", 1))))));
    AddRule(new CatchRule("Catch", GetScope(),
        new Cm::Parsing::AlternativeParser(
            new Cm::Parsing::ActionParser("A0",
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::SequenceParser(
                            new Cm::Parsing::SequenceParser(
                                new Cm::Parsing::SequenceParser(
                                    new Cm::Parsing::KeywordParser("catch"),
                                    new Cm::Parsing::ExpectationParser(
                                        new Cm::Parsing::CharParser('('))),
                                new Cm::Parsing::ExpectationParser(
                                    new Cm::Parsing::NonterminalParser("TypeExpr", "TypeExpr", 1))),
                            new Cm::Parsing::ExpectationParser(
                                new Cm::Parsing::NonterminalParser("Identifier", "Identifier", 0))),
                        new Cm::Parsing::ExpectationParser(
                            new Cm::Parsing::CharParser(')'))),
                    new Cm::Parsing::NonterminalParser("catchBlock1", "CompoundStatement", 1))),
            new Cm::Parsing::ActionParser("A1",
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::KeywordParser("catch"),
                    new Cm::Parsing::NonterminalParser("catchBlock2", "CompoundStatement", 1))))));
    AddRule(new FinallyRule("Finally", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::KeywordParser("finally"),
                new Cm::Parsing::NonterminalParser("CompoundStatement", "CompoundStatement", 1)))));
}

} } // namespace cminor.parser
