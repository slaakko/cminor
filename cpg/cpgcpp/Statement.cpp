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
#include <cminor/cpg/cpgcpp/Expression.hpp>
#include <cminor/cpg/cpgcpp/Declaration.hpp>
#include <cminor/cpg/cpgcpp/Declarator.hpp>
#include <cminor/cpg/cpgcpp/Identifier.hpp>
#include <cminor/pom/Type.hpp>

namespace cpg { namespace cpp {

using namespace cminor::pom;
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

StatementGrammar::StatementGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("StatementGrammar", parsingDomain_->GetNamespaceScope("cpg.cpp"), parsingDomain_)
{
    SetOwner(0);
}

cminor::pom::CompoundStatement* StatementGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName)
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
    std::unique_ptr<cminor::parsing::ParsingData> parsingData(new cminor::parsing::ParsingData(GetParsingDomain()->GetNumRules()));
    scanner.SetParsingData(parsingData.get());
    cminor::parsing::Match match = cminor::parsing::Grammar::Parse(scanner, stack, parsingData.get());
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
    cminor::pom::CompoundStatement* result = *static_cast<cminor::parsing::ValueObject<cminor::pom::CompoundStatement*>*>(value.get());
    stack.pop();
    return result;
}

class StatementGrammar::StatementRule : public cminor::parsing::Rule
{
public:
    StatementRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Statement*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
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
        cminor::parsing::NonterminalParser* labeledStatementNonterminalParser = GetNonterminal("LabeledStatement");
        labeledStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostLabeledStatement));
        cminor::parsing::NonterminalParser* emptyStatementNonterminalParser = GetNonterminal("EmptyStatement");
        emptyStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostEmptyStatement));
        cminor::parsing::NonterminalParser* compoundStatementNonterminalParser = GetNonterminal("CompoundStatement");
        compoundStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostCompoundStatement));
        cminor::parsing::NonterminalParser* selectionStatementNonterminalParser = GetNonterminal("SelectionStatement");
        selectionStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostSelectionStatement));
        cminor::parsing::NonterminalParser* iterationStatementNonterminalParser = GetNonterminal("IterationStatement");
        iterationStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostIterationStatement));
        cminor::parsing::NonterminalParser* jumpStatementNonterminalParser = GetNonterminal("JumpStatement");
        jumpStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostJumpStatement));
        cminor::parsing::NonterminalParser* declarationStatementNonterminalParser = GetNonterminal("DeclarationStatement");
        declarationStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostDeclarationStatement));
        cminor::parsing::NonterminalParser* tryStatementNonterminalParser = GetNonterminal("TryStatement");
        tryStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostTryStatement));
        cminor::parsing::NonterminalParser* expressionStatementNonterminalParser = GetNonterminal("ExpressionStatement");
        expressionStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostExpressionStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromLabeledStatement;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromEmptyStatement;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromCompoundStatement;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromSelectionStatement;
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromIterationStatement;
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromJumpStatement;
    }
    void A6Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromDeclarationStatement;
    }
    void A7Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromTryStatement;
    }
    void A8Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromExpressionStatement;
    }
    void PostLabeledStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromLabeledStatement_value = std::move(stack.top());
            context->fromLabeledStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(fromLabeledStatement_value.get());
            stack.pop();
        }
    }
    void PostEmptyStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromEmptyStatement_value = std::move(stack.top());
            context->fromEmptyStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(fromEmptyStatement_value.get());
            stack.pop();
        }
    }
    void PostCompoundStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCompoundStatement_value = std::move(stack.top());
            context->fromCompoundStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::CompoundStatement*>*>(fromCompoundStatement_value.get());
            stack.pop();
        }
    }
    void PostSelectionStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSelectionStatement_value = std::move(stack.top());
            context->fromSelectionStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(fromSelectionStatement_value.get());
            stack.pop();
        }
    }
    void PostIterationStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIterationStatement_value = std::move(stack.top());
            context->fromIterationStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(fromIterationStatement_value.get());
            stack.pop();
        }
    }
    void PostJumpStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromJumpStatement_value = std::move(stack.top());
            context->fromJumpStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(fromJumpStatement_value.get());
            stack.pop();
        }
    }
    void PostDeclarationStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDeclarationStatement_value = std::move(stack.top());
            context->fromDeclarationStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(fromDeclarationStatement_value.get());
            stack.pop();
        }
    }
    void PostTryStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTryStatement_value = std::move(stack.top());
            context->fromTryStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::TryStatement*>*>(fromTryStatement_value.get());
            stack.pop();
        }
    }
    void PostExpressionStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpressionStatement_value = std::move(stack.top());
            context->fromExpressionStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(fromExpressionStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromLabeledStatement(), fromEmptyStatement(), fromCompoundStatement(), fromSelectionStatement(), fromIterationStatement(), fromJumpStatement(), fromDeclarationStatement(), fromTryStatement(), fromExpressionStatement() {}
        cminor::pom::Statement* value;
        cminor::pom::Statement* fromLabeledStatement;
        cminor::pom::Statement* fromEmptyStatement;
        cminor::pom::CompoundStatement* fromCompoundStatement;
        cminor::pom::Statement* fromSelectionStatement;
        cminor::pom::Statement* fromIterationStatement;
        cminor::pom::Statement* fromJumpStatement;
        cminor::pom::Statement* fromDeclarationStatement;
        cminor::pom::TryStatement* fromTryStatement;
        cminor::pom::Statement* fromExpressionStatement;
    };
};

class StatementGrammar::LabeledStatementRule : public cminor::parsing::Rule
{
public:
    LabeledStatementRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Statement*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LabeledStatementRule>(this, &LabeledStatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LabeledStatementRule>(this, &LabeledStatementRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LabeledStatementRule>(this, &LabeledStatementRule::A2Action));
        cminor::parsing::NonterminalParser* labelNonterminalParser = GetNonterminal("Label");
        labelNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LabeledStatementRule>(this, &LabeledStatementRule::PostLabel));
        cminor::parsing::NonterminalParser* s1NonterminalParser = GetNonterminal("s1");
        s1NonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LabeledStatementRule>(this, &LabeledStatementRule::Posts1));
        cminor::parsing::NonterminalParser* constantExpressionNonterminalParser = GetNonterminal("ConstantExpression");
        constantExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LabeledStatementRule>(this, &LabeledStatementRule::PostConstantExpression));
        cminor::parsing::NonterminalParser* s2NonterminalParser = GetNonterminal("s2");
        s2NonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LabeledStatementRule>(this, &LabeledStatementRule::Posts2));
        cminor::parsing::NonterminalParser* s3NonterminalParser = GetNonterminal("s3");
        s3NonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LabeledStatementRule>(this, &LabeledStatementRule::Posts3));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new LabeledStatement(context->fromLabel, context->froms1);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new CaseStatement(context->fromConstantExpression, context->froms2);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new DefaultStatement(context->froms3);
    }
    void PostLabel(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromLabel_value = std::move(stack.top());
            context->fromLabel = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromLabel_value.get());
            stack.pop();
        }
    }
    void Posts1(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> froms1_value = std::move(stack.top());
            context->froms1 = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(froms1_value.get());
            stack.pop();
        }
    }
    void PostConstantExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromConstantExpression_value = std::move(stack.top());
            context->fromConstantExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromConstantExpression_value.get());
            stack.pop();
        }
    }
    void Posts2(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> froms2_value = std::move(stack.top());
            context->froms2 = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(froms2_value.get());
            stack.pop();
        }
    }
    void Posts3(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> froms3_value = std::move(stack.top());
            context->froms3 = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(froms3_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromLabel(), froms1(), fromConstantExpression(), froms2(), froms3() {}
        cminor::pom::Statement* value;
        std::string fromLabel;
        cminor::pom::Statement* froms1;
        cminor::pom::CppObject* fromConstantExpression;
        cminor::pom::Statement* froms2;
        cminor::pom::Statement* froms3;
    };
};

class StatementGrammar::LabelRule : public cminor::parsing::Rule
{
public:
    LabelRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("std::string");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<std::string>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LabelRule>(this, &LabelRule::A0Action));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LabelRule>(this, &LabelRule::PostIdentifier));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromIdentifier;
    }
    void PostIdentifier(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIdentifier_value = std::move(stack.top());
            context->fromIdentifier = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromIdentifier() {}
        std::string value;
        std::string fromIdentifier;
    };
};

class StatementGrammar::EmptyStatementRule : public cminor::parsing::Rule
{
public:
    EmptyStatementRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Statement*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EmptyStatementRule>(this, &EmptyStatementRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new EmptyStatement;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::pom::Statement* value;
    };
};

class StatementGrammar::ExpressionStatementRule : public cminor::parsing::Rule
{
public:
    ExpressionStatementRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Statement*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExpressionStatementRule>(this, &ExpressionStatementRule::A0Action));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ExpressionStatementRule>(this, &ExpressionStatementRule::PostExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ExpressionStatement(context->fromExpression);
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromExpression() {}
        cminor::pom::Statement* value;
        cminor::pom::CppObject* fromExpression;
    };
};

class StatementGrammar::CompoundStatementRule : public cminor::parsing::Rule
{
public:
    CompoundStatementRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::CompoundStatement*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<CompoundStatement>", "cs"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CompoundStatement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CompoundStatementRule>(this, &CompoundStatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CompoundStatementRule>(this, &CompoundStatementRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CompoundStatementRule>(this, &CompoundStatementRule::A2Action));
        cminor::parsing::NonterminalParser* statementNonterminalParser = GetNonterminal("Statement");
        statementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CompoundStatementRule>(this, &CompoundStatementRule::PostStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->cs.reset(new CompoundStatement);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->cs->Add(context->fromStatement);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->cs.release();
    }
    void PostStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromStatement_value = std::move(stack.top());
            context->fromStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(fromStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), cs(), fromStatement() {}
        cminor::pom::CompoundStatement* value;
        std::unique_ptr<CompoundStatement> cs;
        cminor::pom::Statement* fromStatement;
    };
};

class StatementGrammar::SelectionStatementRule : public cminor::parsing::Rule
{
public:
    SelectionStatementRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Statement*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SelectionStatementRule>(this, &SelectionStatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SelectionStatementRule>(this, &SelectionStatementRule::A1Action));
        cminor::parsing::NonterminalParser* ifStatementNonterminalParser = GetNonterminal("IfStatement");
        ifStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SelectionStatementRule>(this, &SelectionStatementRule::PostIfStatement));
        cminor::parsing::NonterminalParser* switchStatementNonterminalParser = GetNonterminal("SwitchStatement");
        switchStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SelectionStatementRule>(this, &SelectionStatementRule::PostSwitchStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromIfStatement;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromSwitchStatement;
    }
    void PostIfStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIfStatement_value = std::move(stack.top());
            context->fromIfStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(fromIfStatement_value.get());
            stack.pop();
        }
    }
    void PostSwitchStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSwitchStatement_value = std::move(stack.top());
            context->fromSwitchStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(fromSwitchStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromIfStatement(), fromSwitchStatement() {}
        cminor::pom::Statement* value;
        cminor::pom::Statement* fromIfStatement;
        cminor::pom::Statement* fromSwitchStatement;
    };
};

class StatementGrammar::IfStatementRule : public cminor::parsing::Rule
{
public:
    IfStatementRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Statement*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IfStatementRule>(this, &IfStatementRule::A0Action));
        cminor::parsing::NonterminalParser* conditionNonterminalParser = GetNonterminal("Condition");
        conditionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IfStatementRule>(this, &IfStatementRule::PostCondition));
        cminor::parsing::NonterminalParser* thenStatementNonterminalParser = GetNonterminal("thenStatement");
        thenStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IfStatementRule>(this, &IfStatementRule::PostthenStatement));
        cminor::parsing::NonterminalParser* elseStatementNonterminalParser = GetNonterminal("elseStatement");
        elseStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IfStatementRule>(this, &IfStatementRule::PostelseStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new IfStatement(context->fromCondition, context->fromthenStatement, context->fromelseStatement);
    }
    void PostCondition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCondition_value = std::move(stack.top());
            context->fromCondition = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromCondition_value.get());
            stack.pop();
        }
    }
    void PostthenStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromthenStatement_value = std::move(stack.top());
            context->fromthenStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(fromthenStatement_value.get());
            stack.pop();
        }
    }
    void PostelseStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromelseStatement_value = std::move(stack.top());
            context->fromelseStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(fromelseStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromCondition(), fromthenStatement(), fromelseStatement() {}
        cminor::pom::Statement* value;
        cminor::pom::CppObject* fromCondition;
        cminor::pom::Statement* fromthenStatement;
        cminor::pom::Statement* fromelseStatement;
    };
};

class StatementGrammar::SwitchStatementRule : public cminor::parsing::Rule
{
public:
    SwitchStatementRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Statement*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SwitchStatementRule>(this, &SwitchStatementRule::A0Action));
        cminor::parsing::NonterminalParser* conditionNonterminalParser = GetNonterminal("Condition");
        conditionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SwitchStatementRule>(this, &SwitchStatementRule::PostCondition));
        cminor::parsing::NonterminalParser* statementNonterminalParser = GetNonterminal("Statement");
        statementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SwitchStatementRule>(this, &SwitchStatementRule::PostStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new SwitchStatement(context->fromCondition, context->fromStatement);
    }
    void PostCondition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCondition_value = std::move(stack.top());
            context->fromCondition = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromCondition_value.get());
            stack.pop();
        }
    }
    void PostStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromStatement_value = std::move(stack.top());
            context->fromStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(fromStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromCondition(), fromStatement() {}
        cminor::pom::Statement* value;
        cminor::pom::CppObject* fromCondition;
        cminor::pom::Statement* fromStatement;
    };
};

class StatementGrammar::IterationStatementRule : public cminor::parsing::Rule
{
public:
    IterationStatementRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Statement*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IterationStatementRule>(this, &IterationStatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IterationStatementRule>(this, &IterationStatementRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IterationStatementRule>(this, &IterationStatementRule::A2Action));
        cminor::parsing::NonterminalParser* whileStatementNonterminalParser = GetNonterminal("WhileStatement");
        whileStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IterationStatementRule>(this, &IterationStatementRule::PostWhileStatement));
        cminor::parsing::NonterminalParser* doStatementNonterminalParser = GetNonterminal("DoStatement");
        doStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IterationStatementRule>(this, &IterationStatementRule::PostDoStatement));
        cminor::parsing::NonterminalParser* forStatementNonterminalParser = GetNonterminal("ForStatement");
        forStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IterationStatementRule>(this, &IterationStatementRule::PostForStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromWhileStatement;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromDoStatement;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromForStatement;
    }
    void PostWhileStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromWhileStatement_value = std::move(stack.top());
            context->fromWhileStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(fromWhileStatement_value.get());
            stack.pop();
        }
    }
    void PostDoStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDoStatement_value = std::move(stack.top());
            context->fromDoStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(fromDoStatement_value.get());
            stack.pop();
        }
    }
    void PostForStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromForStatement_value = std::move(stack.top());
            context->fromForStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(fromForStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromWhileStatement(), fromDoStatement(), fromForStatement() {}
        cminor::pom::Statement* value;
        cminor::pom::Statement* fromWhileStatement;
        cminor::pom::Statement* fromDoStatement;
        cminor::pom::Statement* fromForStatement;
    };
};

class StatementGrammar::WhileStatementRule : public cminor::parsing::Rule
{
public:
    WhileStatementRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Statement*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<WhileStatementRule>(this, &WhileStatementRule::A0Action));
        cminor::parsing::NonterminalParser* conditionNonterminalParser = GetNonterminal("Condition");
        conditionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<WhileStatementRule>(this, &WhileStatementRule::PostCondition));
        cminor::parsing::NonterminalParser* statementNonterminalParser = GetNonterminal("Statement");
        statementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<WhileStatementRule>(this, &WhileStatementRule::PostStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new WhileStatement(context->fromCondition, context->fromStatement);
    }
    void PostCondition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCondition_value = std::move(stack.top());
            context->fromCondition = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromCondition_value.get());
            stack.pop();
        }
    }
    void PostStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromStatement_value = std::move(stack.top());
            context->fromStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(fromStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromCondition(), fromStatement() {}
        cminor::pom::Statement* value;
        cminor::pom::CppObject* fromCondition;
        cminor::pom::Statement* fromStatement;
    };
};

class StatementGrammar::DoStatementRule : public cminor::parsing::Rule
{
public:
    DoStatementRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Statement*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DoStatementRule>(this, &DoStatementRule::A0Action));
        cminor::parsing::NonterminalParser* statementNonterminalParser = GetNonterminal("Statement");
        statementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DoStatementRule>(this, &DoStatementRule::PostStatement));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DoStatementRule>(this, &DoStatementRule::PostExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new DoStatement(context->fromStatement, context->fromExpression);
    }
    void PostStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromStatement_value = std::move(stack.top());
            context->fromStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(fromStatement_value.get());
            stack.pop();
        }
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromStatement(), fromExpression() {}
        cminor::pom::Statement* value;
        cminor::pom::Statement* fromStatement;
        cminor::pom::CppObject* fromExpression;
    };
};

class StatementGrammar::ForStatementRule : public cminor::parsing::Rule
{
public:
    ForStatementRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Statement*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ForStatementRule>(this, &ForStatementRule::A0Action));
        cminor::parsing::NonterminalParser* forInitStatementNonterminalParser = GetNonterminal("ForInitStatement");
        forInitStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForStatementRule>(this, &ForStatementRule::PostForInitStatement));
        cminor::parsing::NonterminalParser* conditionNonterminalParser = GetNonterminal("Condition");
        conditionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForStatementRule>(this, &ForStatementRule::PostCondition));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForStatementRule>(this, &ForStatementRule::PostExpression));
        cminor::parsing::NonterminalParser* statementNonterminalParser = GetNonterminal("Statement");
        statementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForStatementRule>(this, &ForStatementRule::PostStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ForStatement(context->fromForInitStatement, context->fromCondition, context->fromExpression, context->fromStatement);
    }
    void PostForInitStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromForInitStatement_value = std::move(stack.top());
            context->fromForInitStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromForInitStatement_value.get());
            stack.pop();
        }
    }
    void PostCondition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCondition_value = std::move(stack.top());
            context->fromCondition = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromCondition_value.get());
            stack.pop();
        }
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PostStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromStatement_value = std::move(stack.top());
            context->fromStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(fromStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromForInitStatement(), fromCondition(), fromExpression(), fromStatement() {}
        cminor::pom::Statement* value;
        cminor::pom::CppObject* fromForInitStatement;
        cminor::pom::CppObject* fromCondition;
        cminor::pom::CppObject* fromExpression;
        cminor::pom::Statement* fromStatement;
    };
};

class StatementGrammar::ForInitStatementRule : public cminor::parsing::Rule
{
public:
    ForInitStatementRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::CppObject*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ForInitStatementRule>(this, &ForInitStatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ForInitStatementRule>(this, &ForInitStatementRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ForInitStatementRule>(this, &ForInitStatementRule::A2Action));
        cminor::parsing::NonterminalParser* emptyStatementNonterminalParser = GetNonterminal("EmptyStatement");
        emptyStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForInitStatementRule>(this, &ForInitStatementRule::PostEmptyStatement));
        cminor::parsing::NonterminalParser* expressionStatementNonterminalParser = GetNonterminal("ExpressionStatement");
        expressionStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForInitStatementRule>(this, &ForInitStatementRule::PostExpressionStatement));
        cminor::parsing::NonterminalParser* simpleDeclarationNonterminalParser = GetNonterminal("SimpleDeclaration");
        simpleDeclarationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForInitStatementRule>(this, &ForInitStatementRule::PostSimpleDeclaration));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromEmptyStatement;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromExpressionStatement;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromSimpleDeclaration;
    }
    void PostEmptyStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromEmptyStatement_value = std::move(stack.top());
            context->fromEmptyStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(fromEmptyStatement_value.get());
            stack.pop();
        }
    }
    void PostExpressionStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpressionStatement_value = std::move(stack.top());
            context->fromExpressionStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(fromExpressionStatement_value.get());
            stack.pop();
        }
    }
    void PostSimpleDeclaration(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSimpleDeclaration_value = std::move(stack.top());
            context->fromSimpleDeclaration = *static_cast<cminor::parsing::ValueObject<cminor::pom::SimpleDeclaration*>*>(fromSimpleDeclaration_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromEmptyStatement(), fromExpressionStatement(), fromSimpleDeclaration() {}
        cminor::pom::CppObject* value;
        cminor::pom::Statement* fromEmptyStatement;
        cminor::pom::Statement* fromExpressionStatement;
        cminor::pom::SimpleDeclaration* fromSimpleDeclaration;
    };
};

class StatementGrammar::JumpStatementRule : public cminor::parsing::Rule
{
public:
    JumpStatementRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Statement*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<JumpStatementRule>(this, &JumpStatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<JumpStatementRule>(this, &JumpStatementRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<JumpStatementRule>(this, &JumpStatementRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<JumpStatementRule>(this, &JumpStatementRule::A3Action));
        cminor::parsing::NonterminalParser* breakStatementNonterminalParser = GetNonterminal("BreakStatement");
        breakStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<JumpStatementRule>(this, &JumpStatementRule::PostBreakStatement));
        cminor::parsing::NonterminalParser* continueStatementNonterminalParser = GetNonterminal("ContinueStatement");
        continueStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<JumpStatementRule>(this, &JumpStatementRule::PostContinueStatement));
        cminor::parsing::NonterminalParser* returnStatementNonterminalParser = GetNonterminal("ReturnStatement");
        returnStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<JumpStatementRule>(this, &JumpStatementRule::PostReturnStatement));
        cminor::parsing::NonterminalParser* gotoStatementNonterminalParser = GetNonterminal("GotoStatement");
        gotoStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<JumpStatementRule>(this, &JumpStatementRule::PostGotoStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromBreakStatement;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromContinueStatement;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromReturnStatement;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromGotoStatement;
    }
    void PostBreakStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromBreakStatement_value = std::move(stack.top());
            context->fromBreakStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(fromBreakStatement_value.get());
            stack.pop();
        }
    }
    void PostContinueStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromContinueStatement_value = std::move(stack.top());
            context->fromContinueStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(fromContinueStatement_value.get());
            stack.pop();
        }
    }
    void PostReturnStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromReturnStatement_value = std::move(stack.top());
            context->fromReturnStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(fromReturnStatement_value.get());
            stack.pop();
        }
    }
    void PostGotoStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromGotoStatement_value = std::move(stack.top());
            context->fromGotoStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::Statement*>*>(fromGotoStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromBreakStatement(), fromContinueStatement(), fromReturnStatement(), fromGotoStatement() {}
        cminor::pom::Statement* value;
        cminor::pom::Statement* fromBreakStatement;
        cminor::pom::Statement* fromContinueStatement;
        cminor::pom::Statement* fromReturnStatement;
        cminor::pom::Statement* fromGotoStatement;
    };
};

class StatementGrammar::BreakStatementRule : public cminor::parsing::Rule
{
public:
    BreakStatementRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Statement*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BreakStatementRule>(this, &BreakStatementRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new BreakStatement;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::pom::Statement* value;
    };
};

class StatementGrammar::ContinueStatementRule : public cminor::parsing::Rule
{
public:
    ContinueStatementRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Statement*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ContinueStatementRule>(this, &ContinueStatementRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ContinueStatement;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::pom::Statement* value;
    };
};

class StatementGrammar::ReturnStatementRule : public cminor::parsing::Rule
{
public:
    ReturnStatementRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Statement*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ReturnStatementRule>(this, &ReturnStatementRule::A0Action));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ReturnStatementRule>(this, &ReturnStatementRule::PostExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ReturnStatement(context->fromExpression);
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromExpression() {}
        cminor::pom::Statement* value;
        cminor::pom::CppObject* fromExpression;
    };
};

class StatementGrammar::GotoStatementRule : public cminor::parsing::Rule
{
public:
    GotoStatementRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Statement*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<GotoStatementRule>(this, &GotoStatementRule::A0Action));
        cminor::parsing::NonterminalParser* gotoTargetNonterminalParser = GetNonterminal("GotoTarget");
        gotoTargetNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<GotoStatementRule>(this, &GotoStatementRule::PostGotoTarget));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new GotoStatement(context->fromGotoTarget);
    }
    void PostGotoTarget(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromGotoTarget_value = std::move(stack.top());
            context->fromGotoTarget = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromGotoTarget_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromGotoTarget() {}
        cminor::pom::Statement* value;
        std::string fromGotoTarget;
    };
};

class StatementGrammar::GotoTargetRule : public cminor::parsing::Rule
{
public:
    GotoTargetRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("std::string");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<std::string>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<GotoTargetRule>(this, &GotoTargetRule::A0Action));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<GotoTargetRule>(this, &GotoTargetRule::PostIdentifier));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromIdentifier;
    }
    void PostIdentifier(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIdentifier_value = std::move(stack.top());
            context->fromIdentifier = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromIdentifier() {}
        std::string value;
        std::string fromIdentifier;
    };
};

class StatementGrammar::DeclarationStatementRule : public cminor::parsing::Rule
{
public:
    DeclarationStatementRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Statement*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeclarationStatementRule>(this, &DeclarationStatementRule::A0Action));
        cminor::parsing::NonterminalParser* blockDeclarationNonterminalParser = GetNonterminal("BlockDeclaration");
        blockDeclarationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DeclarationStatementRule>(this, &DeclarationStatementRule::PostBlockDeclaration));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new DeclarationStatement(context->fromBlockDeclaration);
    }
    void PostBlockDeclaration(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromBlockDeclaration_value = std::move(stack.top());
            context->fromBlockDeclaration = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromBlockDeclaration_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromBlockDeclaration() {}
        cminor::pom::Statement* value;
        cminor::pom::CppObject* fromBlockDeclaration;
    };
};

class StatementGrammar::ConditionRule : public cminor::parsing::Rule
{
public:
    ConditionRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::CppObject*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<TypeId>", "ti"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConditionRule>(this, &ConditionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConditionRule>(this, &ConditionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConditionRule>(this, &ConditionRule::A2Action));
        cminor::parsing::NonterminalParser* typeIdNonterminalParser = GetNonterminal("TypeId");
        typeIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConditionRule>(this, &ConditionRule::PostTypeId));
        cminor::parsing::NonterminalParser* declaratorNonterminalParser = GetNonterminal("Declarator");
        declaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConditionRule>(this, &ConditionRule::PostDeclarator));
        cminor::parsing::NonterminalParser* assignmentExpressionNonterminalParser = GetNonterminal("AssignmentExpression");
        assignmentExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConditionRule>(this, &ConditionRule::PostAssignmentExpression));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConditionRule>(this, &ConditionRule::PostExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ti.reset(context->fromTypeId);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ConditionWithDeclarator(context->ti.release(), context->fromDeclarator, context->fromAssignmentExpression);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromExpression;
    }
    void PostTypeId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeId_value = std::move(stack.top());
            context->fromTypeId = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeId*>*>(fromTypeId_value.get());
            stack.pop();
        }
    }
    void PostDeclarator(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDeclarator_value = std::move(stack.top());
            context->fromDeclarator = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromDeclarator_value.get());
            stack.pop();
        }
    }
    void PostAssignmentExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAssignmentExpression_value = std::move(stack.top());
            context->fromAssignmentExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromAssignmentExpression_value.get());
            stack.pop();
        }
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), ti(), fromTypeId(), fromDeclarator(), fromAssignmentExpression(), fromExpression() {}
        cminor::pom::CppObject* value;
        std::unique_ptr<TypeId> ti;
        cminor::pom::TypeId* fromTypeId;
        std::string fromDeclarator;
        cminor::pom::CppObject* fromAssignmentExpression;
        cminor::pom::CppObject* fromExpression;
    };
};

class StatementGrammar::TryStatementRule : public cminor::parsing::Rule
{
public:
    TryStatementRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::TryStatement*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::TryStatement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TryStatementRule>(this, &TryStatementRule::A0Action));
        cminor::parsing::NonterminalParser* compoundStatementNonterminalParser = GetNonterminal("CompoundStatement");
        compoundStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TryStatementRule>(this, &TryStatementRule::PostCompoundStatement));
        cminor::parsing::NonterminalParser* handlerSeqNonterminalParser = GetNonterminal("HandlerSeq");
        handlerSeqNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<TryStatementRule>(this, &TryStatementRule::PreHandlerSeq));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new TryStatement(context->fromCompoundStatement);
    }
    void PostCompoundStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCompoundStatement_value = std::move(stack.top());
            context->fromCompoundStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::CompoundStatement*>*>(fromCompoundStatement_value.get());
            stack.pop();
        }
    }
    void PreHandlerSeq(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<TryStatement*>(context->value)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromCompoundStatement() {}
        cminor::pom::TryStatement* value;
        cminor::pom::CompoundStatement* fromCompoundStatement;
    };
};

class StatementGrammar::HandlerSeqRule : public cminor::parsing::Rule
{
public:
    HandlerSeqRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable("TryStatement*", "st"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> st_value = std::move(stack.top());
        context->st = *static_cast<cminor::parsing::ValueObject<TryStatement*>*>(st_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<HandlerSeqRule>(this, &HandlerSeqRule::A0Action));
        cminor::parsing::NonterminalParser* handlerNonterminalParser = GetNonterminal("Handler");
        handlerNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<HandlerSeqRule>(this, &HandlerSeqRule::PostHandler));
        cminor::parsing::NonterminalParser* handlerSeqNonterminalParser = GetNonterminal("HandlerSeq");
        handlerSeqNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<HandlerSeqRule>(this, &HandlerSeqRule::PreHandlerSeq));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->st->Add(context->fromHandler);
    }
    void PostHandler(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromHandler_value = std::move(stack.top());
            context->fromHandler = *static_cast<cminor::parsing::ValueObject<cminor::pom::Handler*>*>(fromHandler_value.get());
            stack.pop();
        }
    }
    void PreHandlerSeq(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<TryStatement*>(context->st)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): st(), fromHandler() {}
        TryStatement* st;
        cminor::pom::Handler* fromHandler;
    };
};

class StatementGrammar::HandlerRule : public cminor::parsing::Rule
{
public:
    HandlerRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Handler*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Handler*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<HandlerRule>(this, &HandlerRule::A0Action));
        cminor::parsing::NonterminalParser* exceptionDeclarationNonterminalParser = GetNonterminal("ExceptionDeclaration");
        exceptionDeclarationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<HandlerRule>(this, &HandlerRule::PostExceptionDeclaration));
        cminor::parsing::NonterminalParser* compoundStatementNonterminalParser = GetNonterminal("CompoundStatement");
        compoundStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<HandlerRule>(this, &HandlerRule::PostCompoundStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Handler(context->fromExceptionDeclaration, context->fromCompoundStatement);
    }
    void PostExceptionDeclaration(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExceptionDeclaration_value = std::move(stack.top());
            context->fromExceptionDeclaration = *static_cast<cminor::parsing::ValueObject<cminor::pom::ExceptionDeclaration*>*>(fromExceptionDeclaration_value.get());
            stack.pop();
        }
    }
    void PostCompoundStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCompoundStatement_value = std::move(stack.top());
            context->fromCompoundStatement = *static_cast<cminor::parsing::ValueObject<cminor::pom::CompoundStatement*>*>(fromCompoundStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromExceptionDeclaration(), fromCompoundStatement() {}
        cminor::pom::Handler* value;
        cminor::pom::ExceptionDeclaration* fromExceptionDeclaration;
        cminor::pom::CompoundStatement* fromCompoundStatement;
    };
};

class StatementGrammar::ExceptionDeclarationRule : public cminor::parsing::Rule
{
public:
    ExceptionDeclarationRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::ExceptionDeclaration*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<ExceptionDeclaration>", "ed"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::ExceptionDeclaration*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExceptionDeclarationRule>(this, &ExceptionDeclarationRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExceptionDeclarationRule>(this, &ExceptionDeclarationRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExceptionDeclarationRule>(this, &ExceptionDeclarationRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExceptionDeclarationRule>(this, &ExceptionDeclarationRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExceptionDeclarationRule>(this, &ExceptionDeclarationRule::A4Action));
        cminor::parsing::NonterminalParser* typeSpecifierSeqNonterminalParser = GetNonterminal("TypeSpecifierSeq");
        typeSpecifierSeqNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ExceptionDeclarationRule>(this, &ExceptionDeclarationRule::PreTypeSpecifierSeq));
        cminor::parsing::NonterminalParser* declaratorNonterminalParser = GetNonterminal("Declarator");
        declaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ExceptionDeclarationRule>(this, &ExceptionDeclarationRule::PostDeclarator));
        cminor::parsing::NonterminalParser* abstractDeclaratorNonterminalParser = GetNonterminal("AbstractDeclarator");
        abstractDeclaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ExceptionDeclarationRule>(this, &ExceptionDeclarationRule::PostAbstractDeclarator));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ed.reset(new ExceptionDeclaration);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->ed.release();
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ed->GetTypeId()->Declarator() = context->fromDeclarator;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ed->GetTypeId()->Declarator() = context->fromAbstractDeclarator;
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value->CatchAll() = true;
    }
    void PreTypeSpecifierSeq(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::TypeId*>(context->ed->GetTypeId())));
    }
    void PostDeclarator(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDeclarator_value = std::move(stack.top());
            context->fromDeclarator = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromDeclarator_value.get());
            stack.pop();
        }
    }
    void PostAbstractDeclarator(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAbstractDeclarator_value = std::move(stack.top());
            context->fromAbstractDeclarator = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromAbstractDeclarator_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), ed(), fromDeclarator(), fromAbstractDeclarator() {}
        cminor::pom::ExceptionDeclaration* value;
        std::unique_ptr<ExceptionDeclaration> ed;
        std::string fromDeclarator;
        std::string fromAbstractDeclarator;
    };
};

void StatementGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("cpg.cpp.ExpressionGrammar");
    if (!grammar0)
    {
        grammar0 = cpg::cpp::ExpressionGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar("cpg.cpp.DeclarationGrammar");
    if (!grammar1)
    {
        grammar1 = cpg::cpp::DeclarationGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar("cpg.cpp.DeclaratorGrammar");
    if (!grammar2)
    {
        grammar2 = cpg::cpp::DeclaratorGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    cminor::parsing::Grammar* grammar3 = pd->GetGrammar("cpg.cpp.IdentifierGrammar");
    if (!grammar3)
    {
        grammar3 = cpg::cpp::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
    cminor::parsing::Grammar* grammar4 = pd->GetGrammar("cminor.parsing.stdlib");
    if (!grammar4)
    {
        grammar4 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar4);
}

void StatementGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("Expression", this, "ExpressionGrammar.Expression"));
    AddRuleLink(new cminor::parsing::RuleLink("ConstantExpression", this, "ExpressionGrammar.ConstantExpression"));
    AddRuleLink(new cminor::parsing::RuleLink("Declarator", this, "DeclaratorGrammar.Declarator"));
    AddRuleLink(new cminor::parsing::RuleLink("AssignmentExpression", this, "ExpressionGrammar.AssignmentExpression"));
    AddRuleLink(new cminor::parsing::RuleLink("SimpleDeclaration", this, "DeclarationGrammar.SimpleDeclaration"));
    AddRuleLink(new cminor::parsing::RuleLink("TypeId", this, "DeclaratorGrammar.TypeId"));
    AddRuleLink(new cminor::parsing::RuleLink("TypeSpecifierSeq", this, "DeclaratorGrammar.TypeSpecifierSeq"));
    AddRuleLink(new cminor::parsing::RuleLink("Identifier", this, "IdentifierGrammar.Identifier"));
    AddRuleLink(new cminor::parsing::RuleLink("AbstractDeclarator", this, "DeclaratorGrammar.AbstractDeclarator"));
    AddRuleLink(new cminor::parsing::RuleLink("BlockDeclaration", this, "DeclarationGrammar.BlockDeclaration"));
    AddRuleLink(new cminor::parsing::RuleLink("spaces_and_comments", this, "cminor.parsing.stdlib.spaces_and_comments"));
    AddRule(new StatementRule("Statement", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::AlternativeParser(
                                    new cminor::parsing::AlternativeParser(
                                        new cminor::parsing::ActionParser("A0",
                                            new cminor::parsing::NonterminalParser("LabeledStatement", "LabeledStatement", 0)),
                                        new cminor::parsing::ActionParser("A1",
                                            new cminor::parsing::NonterminalParser("EmptyStatement", "EmptyStatement", 0))),
                                    new cminor::parsing::ActionParser("A2",
                                        new cminor::parsing::NonterminalParser("CompoundStatement", "CompoundStatement", 0))),
                                new cminor::parsing::ActionParser("A3",
                                    new cminor::parsing::NonterminalParser("SelectionStatement", "SelectionStatement", 0))),
                            new cminor::parsing::ActionParser("A4",
                                new cminor::parsing::NonterminalParser("IterationStatement", "IterationStatement", 0))),
                        new cminor::parsing::ActionParser("A5",
                            new cminor::parsing::NonterminalParser("JumpStatement", "JumpStatement", 0))),
                    new cminor::parsing::ActionParser("A6",
                        new cminor::parsing::NonterminalParser("DeclarationStatement", "DeclarationStatement", 0))),
                new cminor::parsing::ActionParser("A7",
                    new cminor::parsing::NonterminalParser("TryStatement", "TryStatement", 0))),
            new cminor::parsing::ActionParser("A8",
                new cminor::parsing::NonterminalParser("ExpressionStatement", "ExpressionStatement", 0)))));
    AddRule(new LabeledStatementRule("LabeledStatement", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::NonterminalParser("Label", "Label", 0),
                            new cminor::parsing::DifferenceParser(
                                new cminor::parsing::CharParser(':'),
                                new cminor::parsing::StringParser("::"))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("s1", "Statement", 0)))),
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::KeywordParser("case"),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::NonterminalParser("ConstantExpression", "ConstantExpression", 0))),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::CharParser(':'))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("s2", "Statement", 0))))),
            new cminor::parsing::ActionParser("A2",
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::KeywordParser("default"),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser(':'))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser("s3", "Statement", 0)))))));
    AddRule(new LabelRule("Label", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::NonterminalParser("Identifier", "Identifier", 0))));
    AddRule(new EmptyStatementRule("EmptyStatement", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::CharParser(';'))));
    AddRule(new ExpressionStatementRule("ExpressionStatement", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::NonterminalParser("Expression", "Expression", 0),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new CompoundStatementRule("CompoundStatement", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::CharParser('{')),
                new cminor::parsing::KleeneStarParser(
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::NonterminalParser("Statement", "Statement", 0)))),
            new cminor::parsing::ActionParser("A2",
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser('}'))))));
    AddRule(new SelectionStatementRule("SelectionStatement", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("IfStatement", "IfStatement", 0)),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::NonterminalParser("SwitchStatement", "SwitchStatement", 0)))));
    AddRule(new IfStatementRule("IfStatement", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::KeywordParser("if"),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::CharParser('('))),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser("Condition", "Condition", 0))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser(')'))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser("thenStatement", "Statement", 0))),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::KeywordParser("else"),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("elseStatement", "Statement", 0))))))));
    AddRule(new SwitchStatementRule("SwitchStatement", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::KeywordParser("switch"),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser('('))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser("Condition", "Condition", 0))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(')'))),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser("Statement", "Statement", 0))))));
    AddRule(new IterationStatementRule("IterationStatement", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::NonterminalParser("WhileStatement", "WhileStatement", 0)),
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("DoStatement", "DoStatement", 0))),
            new cminor::parsing::ActionParser("A2",
                new cminor::parsing::NonterminalParser("ForStatement", "ForStatement", 0)))));
    AddRule(new WhileStatementRule("WhileStatement", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::KeywordParser("while"),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser('('))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser("Condition", "Condition", 0))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(')'))),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser("Statement", "Statement", 0))))));
    AddRule(new DoStatementRule("DoStatement", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::KeywordParser("do"),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::NonterminalParser("Statement", "Statement", 0))),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::KeywordParser("while"))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser('('))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser("Expression", "Expression", 0))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(')'))),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new ForStatementRule("ForStatement", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::KeywordParser("for"),
                                    new cminor::parsing::ExpectationParser(
                                        new cminor::parsing::CharParser('('))),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::NonterminalParser("ForInitStatement", "ForInitStatement", 0))),
                            new cminor::parsing::OptionalParser(
                                new cminor::parsing::NonterminalParser("Condition", "Condition", 0))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser(';'))),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::NonterminalParser("Expression", "Expression", 0))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(')'))),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser("Statement", "Statement", 0))))));
    AddRule(new ForInitStatementRule("ForInitStatement", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::NonterminalParser("EmptyStatement", "EmptyStatement", 0)),
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("ExpressionStatement", "ExpressionStatement", 0))),
            new cminor::parsing::ActionParser("A2",
                new cminor::parsing::NonterminalParser("SimpleDeclaration", "SimpleDeclaration", 0)))));
    AddRule(new JumpStatementRule("JumpStatement", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::ActionParser("A0",
                        new cminor::parsing::NonterminalParser("BreakStatement", "BreakStatement", 0)),
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::NonterminalParser("ContinueStatement", "ContinueStatement", 0))),
                new cminor::parsing::ActionParser("A2",
                    new cminor::parsing::NonterminalParser("ReturnStatement", "ReturnStatement", 0))),
            new cminor::parsing::ActionParser("A3",
                new cminor::parsing::NonterminalParser("GotoStatement", "GotoStatement", 0)))));
    AddRule(new BreakStatementRule("BreakStatement", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::KeywordParser("break"),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new ContinueStatementRule("ContinueStatement", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::KeywordParser("continue"),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new ReturnStatementRule("ReturnStatement", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::KeywordParser("return"),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::NonterminalParser("Expression", "Expression", 0))),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new GotoStatementRule("GotoStatement", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::KeywordParser("goto"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser("GotoTarget", "GotoTarget", 0))),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new GotoTargetRule("GotoTarget", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::NonterminalParser("Identifier", "Identifier", 0))));
    AddRule(new DeclarationStatementRule("DeclarationStatement", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::NonterminalParser("BlockDeclaration", "BlockDeclaration", 0))));
    AddRule(new ConditionRule("Condition", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::ActionParser("A0",
                            new cminor::parsing::NonterminalParser("TypeId", "TypeId", 0)),
                        new cminor::parsing::NonterminalParser("Declarator", "Declarator", 0)),
                    new cminor::parsing::CharParser('=')),
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("AssignmentExpression", "AssignmentExpression", 0))),
            new cminor::parsing::ActionParser("A2",
                new cminor::parsing::NonterminalParser("Expression", "Expression", 0)))));
    AddRule(new TryStatementRule("TryStatement", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::KeywordParser("try"),
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser("CompoundStatement", "CompoundStatement", 0)))),
            new cminor::parsing::ExpectationParser(
                new cminor::parsing::NonterminalParser("HandlerSeq", "HandlerSeq", 1)))));
    AddRule(new HandlerSeqRule("HandlerSeq", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser("Handler", "Handler", 0))),
            new cminor::parsing::OptionalParser(
                new cminor::parsing::NonterminalParser("HandlerSeq", "HandlerSeq", 1)))));
    AddRule(new HandlerRule("Handler", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::KeywordParser("catch"),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser('('))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser("ExceptionDeclaration", "ExceptionDeclaration", 0))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(')'))),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser("CompoundStatement", "CompoundStatement", 0))))));
    AddRule(new ExceptionDeclarationRule("ExceptionDeclaration", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::EmptyParser()),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::NonterminalParser("TypeSpecifierSeq", "TypeSpecifierSeq", 1),
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::ActionParser("A2",
                                    new cminor::parsing::NonterminalParser("Declarator", "Declarator", 0)),
                                new cminor::parsing::ActionParser("A3",
                                    new cminor::parsing::NonterminalParser("AbstractDeclarator", "AbstractDeclarator", 0))),
                            new cminor::parsing::EmptyParser())),
                    new cminor::parsing::ActionParser("A4",
                        new cminor::parsing::StringParser("...")))))));
    SetStartRuleName("CompoundStatement");
    SetSkipRuleName("spaces_and_comments");
}

} } // namespace cpg.cpp
