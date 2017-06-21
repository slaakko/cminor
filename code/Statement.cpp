#include "Statement.hpp"
#include <cminor/parsing/Action.hpp>
#include <cminor/parsing/Rule.hpp>
#include <cminor/parsing/ParsingDomain.hpp>
#include <cminor/parsing/Primitive.hpp>
#include <cminor/parsing/Composite.hpp>
#include <cminor/parsing/Nonterminal.hpp>
#include <cminor/parsing/Exception.hpp>
#include <cminor/parsing/StdLib.hpp>
#include <cminor/parsing/XmlLog.hpp>
#include <cminor/util/Unicode.hpp>
#include <cminor/code/Expression.hpp>
#include <cminor/code/Declaration.hpp>
#include <cminor/code/Declarator.hpp>
#include <cminor/code/Identifier.hpp>
#include <cminor/codedom/Type.hpp>

namespace cminor { namespace code {

using namespace cminor::codedom;
using namespace cminor::parsing;
using namespace cminor::util;
using namespace cminor::unicode;

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

StatementGrammar::StatementGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar(ToUtf32("StatementGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cminor.code")), parsingDomain_)
{
    SetOwner(0);
}

cminor::codedom::CompoundStatement* StatementGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName)
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
    if (!match.Hit() || stop.Start() != int(end - start))
    {
        if (StartRule())
        {
            throw cminor::parsing::ExpectationFailure(StartRule()->Info(), fileName, stop, start, end);
        }
        else
        {
            throw cminor::parsing::ParsingException("grammar '" + ToUtf8(Name()) + "' has no start rule", fileName, scanner.GetSpan(), start, end);
        }
    }
    std::unique_ptr<cminor::parsing::Object> value = std::move(stack.top());
    cminor::codedom::CompoundStatement* result = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CompoundStatement*>*>(value.get());
    stack.pop();
    return result;
}

class StatementGrammar::StatementRule : public cminor::parsing::Rule
{
public:
    StatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Statement*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction(ToUtf32("A4"));
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction(ToUtf32("A5"));
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction(ToUtf32("A6"));
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A6Action));
        cminor::parsing::ActionParser* a7ActionParser = GetAction(ToUtf32("A7"));
        a7ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A7Action));
        cminor::parsing::ActionParser* a8ActionParser = GetAction(ToUtf32("A8"));
        a8ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A8Action));
        cminor::parsing::NonterminalParser* labeledStatementNonterminalParser = GetNonterminal(ToUtf32("LabeledStatement"));
        labeledStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostLabeledStatement));
        cminor::parsing::NonterminalParser* emptyStatementNonterminalParser = GetNonterminal(ToUtf32("EmptyStatement"));
        emptyStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostEmptyStatement));
        cminor::parsing::NonterminalParser* compoundStatementNonterminalParser = GetNonterminal(ToUtf32("CompoundStatement"));
        compoundStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostCompoundStatement));
        cminor::parsing::NonterminalParser* selectionStatementNonterminalParser = GetNonterminal(ToUtf32("SelectionStatement"));
        selectionStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostSelectionStatement));
        cminor::parsing::NonterminalParser* iterationStatementNonterminalParser = GetNonterminal(ToUtf32("IterationStatement"));
        iterationStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostIterationStatement));
        cminor::parsing::NonterminalParser* jumpStatementNonterminalParser = GetNonterminal(ToUtf32("JumpStatement"));
        jumpStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostJumpStatement));
        cminor::parsing::NonterminalParser* declarationStatementNonterminalParser = GetNonterminal(ToUtf32("DeclarationStatement"));
        declarationStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostDeclarationStatement));
        cminor::parsing::NonterminalParser* tryStatementNonterminalParser = GetNonterminal(ToUtf32("TryStatement"));
        tryStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostTryStatement));
        cminor::parsing::NonterminalParser* expressionStatementNonterminalParser = GetNonterminal(ToUtf32("ExpressionStatement"));
        expressionStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostExpressionStatement));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromLabeledStatement;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromEmptyStatement;
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromCompoundStatement;
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromSelectionStatement;
    }
    void A4Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromIterationStatement;
    }
    void A5Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromJumpStatement;
    }
    void A6Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromDeclarationStatement;
    }
    void A7Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromTryStatement;
    }
    void A8Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromLabeledStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(fromLabeledStatement_value.get());
            stack.pop();
        }
    }
    void PostEmptyStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromEmptyStatement_value = std::move(stack.top());
            context->fromEmptyStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(fromEmptyStatement_value.get());
            stack.pop();
        }
    }
    void PostCompoundStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCompoundStatement_value = std::move(stack.top());
            context->fromCompoundStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CompoundStatement*>*>(fromCompoundStatement_value.get());
            stack.pop();
        }
    }
    void PostSelectionStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSelectionStatement_value = std::move(stack.top());
            context->fromSelectionStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(fromSelectionStatement_value.get());
            stack.pop();
        }
    }
    void PostIterationStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIterationStatement_value = std::move(stack.top());
            context->fromIterationStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(fromIterationStatement_value.get());
            stack.pop();
        }
    }
    void PostJumpStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromJumpStatement_value = std::move(stack.top());
            context->fromJumpStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(fromJumpStatement_value.get());
            stack.pop();
        }
    }
    void PostDeclarationStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDeclarationStatement_value = std::move(stack.top());
            context->fromDeclarationStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(fromDeclarationStatement_value.get());
            stack.pop();
        }
    }
    void PostTryStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTryStatement_value = std::move(stack.top());
            context->fromTryStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TryStatement*>*>(fromTryStatement_value.get());
            stack.pop();
        }
    }
    void PostExpressionStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpressionStatement_value = std::move(stack.top());
            context->fromExpressionStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(fromExpressionStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromLabeledStatement(), fromEmptyStatement(), fromCompoundStatement(), fromSelectionStatement(), fromIterationStatement(), fromJumpStatement(), fromDeclarationStatement(), fromTryStatement(), fromExpressionStatement() {}
        cminor::codedom::Statement* value;
        cminor::codedom::Statement* fromLabeledStatement;
        cminor::codedom::Statement* fromEmptyStatement;
        cminor::codedom::CompoundStatement* fromCompoundStatement;
        cminor::codedom::Statement* fromSelectionStatement;
        cminor::codedom::Statement* fromIterationStatement;
        cminor::codedom::Statement* fromJumpStatement;
        cminor::codedom::Statement* fromDeclarationStatement;
        cminor::codedom::TryStatement* fromTryStatement;
        cminor::codedom::Statement* fromExpressionStatement;
    };
};

class StatementGrammar::LabeledStatementRule : public cminor::parsing::Rule
{
public:
    LabeledStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Statement*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LabeledStatementRule>(this, &LabeledStatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LabeledStatementRule>(this, &LabeledStatementRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LabeledStatementRule>(this, &LabeledStatementRule::A2Action));
        cminor::parsing::NonterminalParser* labelNonterminalParser = GetNonterminal(ToUtf32("Label"));
        labelNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LabeledStatementRule>(this, &LabeledStatementRule::PostLabel));
        cminor::parsing::NonterminalParser* s1NonterminalParser = GetNonterminal(ToUtf32("s1"));
        s1NonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LabeledStatementRule>(this, &LabeledStatementRule::Posts1));
        cminor::parsing::NonterminalParser* constantExpressionNonterminalParser = GetNonterminal(ToUtf32("ConstantExpression"));
        constantExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LabeledStatementRule>(this, &LabeledStatementRule::PostConstantExpression));
        cminor::parsing::NonterminalParser* s2NonterminalParser = GetNonterminal(ToUtf32("s2"));
        s2NonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LabeledStatementRule>(this, &LabeledStatementRule::Posts2));
        cminor::parsing::NonterminalParser* s3NonterminalParser = GetNonterminal(ToUtf32("s3"));
        s3NonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LabeledStatementRule>(this, &LabeledStatementRule::Posts3));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new LabeledStatement(context->fromLabel, context->froms1);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new CaseStatement(context->fromConstantExpression, context->froms2);
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromLabel = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromLabel_value.get());
            stack.pop();
        }
    }
    void Posts1(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> froms1_value = std::move(stack.top());
            context->froms1 = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(froms1_value.get());
            stack.pop();
        }
    }
    void PostConstantExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromConstantExpression_value = std::move(stack.top());
            context->fromConstantExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromConstantExpression_value.get());
            stack.pop();
        }
    }
    void Posts2(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> froms2_value = std::move(stack.top());
            context->froms2 = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(froms2_value.get());
            stack.pop();
        }
    }
    void Posts3(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> froms3_value = std::move(stack.top());
            context->froms3 = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(froms3_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromLabel(), froms1(), fromConstantExpression(), froms2(), froms3() {}
        cminor::codedom::Statement* value;
        std::u32string fromLabel;
        cminor::codedom::Statement* froms1;
        cminor::codedom::CppObject* fromConstantExpression;
        cminor::codedom::Statement* froms2;
        cminor::codedom::Statement* froms3;
    };
};

class StatementGrammar::LabelRule : public cminor::parsing::Rule
{
public:
    LabelRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("std::u32string"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<std::u32string>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LabelRule>(this, &LabelRule::A0Action));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal(ToUtf32("Identifier"));
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LabelRule>(this, &LabelRule::PostIdentifier));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromIdentifier = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromIdentifier() {}
        std::u32string value;
        std::u32string fromIdentifier;
    };
};

class StatementGrammar::EmptyStatementRule : public cminor::parsing::Rule
{
public:
    EmptyStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Statement*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EmptyStatementRule>(this, &EmptyStatementRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new EmptyStatement;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::codedom::Statement* value;
    };
};

class StatementGrammar::ExpressionStatementRule : public cminor::parsing::Rule
{
public:
    ExpressionStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Statement*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExpressionStatementRule>(this, &ExpressionStatementRule::A0Action));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal(ToUtf32("Expression"));
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ExpressionStatementRule>(this, &ExpressionStatementRule::PostExpression));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromExpression() {}
        cminor::codedom::Statement* value;
        cminor::codedom::CppObject* fromExpression;
    };
};

class StatementGrammar::CompoundStatementRule : public cminor::parsing::Rule
{
public:
    CompoundStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CompoundStatement*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<CompoundStatement>"), ToUtf32("cs")));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CompoundStatement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CompoundStatementRule>(this, &CompoundStatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CompoundStatementRule>(this, &CompoundStatementRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CompoundStatementRule>(this, &CompoundStatementRule::A2Action));
        cminor::parsing::NonterminalParser* statementNonterminalParser = GetNonterminal(ToUtf32("Statement"));
        statementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CompoundStatementRule>(this, &CompoundStatementRule::PostStatement));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->cs.reset(new CompoundStatement);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->cs->Add(context->fromStatement);
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(fromStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), cs(), fromStatement() {}
        cminor::codedom::CompoundStatement* value;
        std::unique_ptr<CompoundStatement> cs;
        cminor::codedom::Statement* fromStatement;
    };
};

class StatementGrammar::SelectionStatementRule : public cminor::parsing::Rule
{
public:
    SelectionStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Statement*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SelectionStatementRule>(this, &SelectionStatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SelectionStatementRule>(this, &SelectionStatementRule::A1Action));
        cminor::parsing::NonterminalParser* ifStatementNonterminalParser = GetNonterminal(ToUtf32("IfStatement"));
        ifStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SelectionStatementRule>(this, &SelectionStatementRule::PostIfStatement));
        cminor::parsing::NonterminalParser* switchStatementNonterminalParser = GetNonterminal(ToUtf32("SwitchStatement"));
        switchStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SelectionStatementRule>(this, &SelectionStatementRule::PostSwitchStatement));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromIfStatement;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromIfStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(fromIfStatement_value.get());
            stack.pop();
        }
    }
    void PostSwitchStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSwitchStatement_value = std::move(stack.top());
            context->fromSwitchStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(fromSwitchStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromIfStatement(), fromSwitchStatement() {}
        cminor::codedom::Statement* value;
        cminor::codedom::Statement* fromIfStatement;
        cminor::codedom::Statement* fromSwitchStatement;
    };
};

class StatementGrammar::IfStatementRule : public cminor::parsing::Rule
{
public:
    IfStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Statement*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IfStatementRule>(this, &IfStatementRule::A0Action));
        cminor::parsing::NonterminalParser* conditionNonterminalParser = GetNonterminal(ToUtf32("Condition"));
        conditionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IfStatementRule>(this, &IfStatementRule::PostCondition));
        cminor::parsing::NonterminalParser* thenStatementNonterminalParser = GetNonterminal(ToUtf32("thenStatement"));
        thenStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IfStatementRule>(this, &IfStatementRule::PostthenStatement));
        cminor::parsing::NonterminalParser* elseStatementNonterminalParser = GetNonterminal(ToUtf32("elseStatement"));
        elseStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IfStatementRule>(this, &IfStatementRule::PostelseStatement));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromCondition = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromCondition_value.get());
            stack.pop();
        }
    }
    void PostthenStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromthenStatement_value = std::move(stack.top());
            context->fromthenStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(fromthenStatement_value.get());
            stack.pop();
        }
    }
    void PostelseStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromelseStatement_value = std::move(stack.top());
            context->fromelseStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(fromelseStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromCondition(), fromthenStatement(), fromelseStatement() {}
        cminor::codedom::Statement* value;
        cminor::codedom::CppObject* fromCondition;
        cminor::codedom::Statement* fromthenStatement;
        cminor::codedom::Statement* fromelseStatement;
    };
};

class StatementGrammar::SwitchStatementRule : public cminor::parsing::Rule
{
public:
    SwitchStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Statement*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SwitchStatementRule>(this, &SwitchStatementRule::A0Action));
        cminor::parsing::NonterminalParser* conditionNonterminalParser = GetNonterminal(ToUtf32("Condition"));
        conditionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SwitchStatementRule>(this, &SwitchStatementRule::PostCondition));
        cminor::parsing::NonterminalParser* statementNonterminalParser = GetNonterminal(ToUtf32("Statement"));
        statementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SwitchStatementRule>(this, &SwitchStatementRule::PostStatement));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromCondition = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromCondition_value.get());
            stack.pop();
        }
    }
    void PostStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromStatement_value = std::move(stack.top());
            context->fromStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(fromStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromCondition(), fromStatement() {}
        cminor::codedom::Statement* value;
        cminor::codedom::CppObject* fromCondition;
        cminor::codedom::Statement* fromStatement;
    };
};

class StatementGrammar::IterationStatementRule : public cminor::parsing::Rule
{
public:
    IterationStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Statement*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IterationStatementRule>(this, &IterationStatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IterationStatementRule>(this, &IterationStatementRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IterationStatementRule>(this, &IterationStatementRule::A2Action));
        cminor::parsing::NonterminalParser* whileStatementNonterminalParser = GetNonterminal(ToUtf32("WhileStatement"));
        whileStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IterationStatementRule>(this, &IterationStatementRule::PostWhileStatement));
        cminor::parsing::NonterminalParser* doStatementNonterminalParser = GetNonterminal(ToUtf32("DoStatement"));
        doStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IterationStatementRule>(this, &IterationStatementRule::PostDoStatement));
        cminor::parsing::NonterminalParser* forStatementNonterminalParser = GetNonterminal(ToUtf32("ForStatement"));
        forStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IterationStatementRule>(this, &IterationStatementRule::PostForStatement));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromWhileStatement;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromDoStatement;
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromWhileStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(fromWhileStatement_value.get());
            stack.pop();
        }
    }
    void PostDoStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDoStatement_value = std::move(stack.top());
            context->fromDoStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(fromDoStatement_value.get());
            stack.pop();
        }
    }
    void PostForStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromForStatement_value = std::move(stack.top());
            context->fromForStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(fromForStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromWhileStatement(), fromDoStatement(), fromForStatement() {}
        cminor::codedom::Statement* value;
        cminor::codedom::Statement* fromWhileStatement;
        cminor::codedom::Statement* fromDoStatement;
        cminor::codedom::Statement* fromForStatement;
    };
};

class StatementGrammar::WhileStatementRule : public cminor::parsing::Rule
{
public:
    WhileStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Statement*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<WhileStatementRule>(this, &WhileStatementRule::A0Action));
        cminor::parsing::NonterminalParser* conditionNonterminalParser = GetNonterminal(ToUtf32("Condition"));
        conditionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<WhileStatementRule>(this, &WhileStatementRule::PostCondition));
        cminor::parsing::NonterminalParser* statementNonterminalParser = GetNonterminal(ToUtf32("Statement"));
        statementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<WhileStatementRule>(this, &WhileStatementRule::PostStatement));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromCondition = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromCondition_value.get());
            stack.pop();
        }
    }
    void PostStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromStatement_value = std::move(stack.top());
            context->fromStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(fromStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromCondition(), fromStatement() {}
        cminor::codedom::Statement* value;
        cminor::codedom::CppObject* fromCondition;
        cminor::codedom::Statement* fromStatement;
    };
};

class StatementGrammar::DoStatementRule : public cminor::parsing::Rule
{
public:
    DoStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Statement*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DoStatementRule>(this, &DoStatementRule::A0Action));
        cminor::parsing::NonterminalParser* statementNonterminalParser = GetNonterminal(ToUtf32("Statement"));
        statementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DoStatementRule>(this, &DoStatementRule::PostStatement));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal(ToUtf32("Expression"));
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DoStatementRule>(this, &DoStatementRule::PostExpression));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(fromStatement_value.get());
            stack.pop();
        }
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromStatement(), fromExpression() {}
        cminor::codedom::Statement* value;
        cminor::codedom::Statement* fromStatement;
        cminor::codedom::CppObject* fromExpression;
    };
};

class StatementGrammar::ForStatementRule : public cminor::parsing::Rule
{
public:
    ForStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Statement*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ForStatementRule>(this, &ForStatementRule::A0Action));
        cminor::parsing::NonterminalParser* forInitStatementNonterminalParser = GetNonterminal(ToUtf32("ForInitStatement"));
        forInitStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForStatementRule>(this, &ForStatementRule::PostForInitStatement));
        cminor::parsing::NonterminalParser* conditionNonterminalParser = GetNonterminal(ToUtf32("Condition"));
        conditionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForStatementRule>(this, &ForStatementRule::PostCondition));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal(ToUtf32("Expression"));
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForStatementRule>(this, &ForStatementRule::PostExpression));
        cminor::parsing::NonterminalParser* statementNonterminalParser = GetNonterminal(ToUtf32("Statement"));
        statementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForStatementRule>(this, &ForStatementRule::PostStatement));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromForInitStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromForInitStatement_value.get());
            stack.pop();
        }
    }
    void PostCondition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCondition_value = std::move(stack.top());
            context->fromCondition = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromCondition_value.get());
            stack.pop();
        }
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PostStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromStatement_value = std::move(stack.top());
            context->fromStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(fromStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromForInitStatement(), fromCondition(), fromExpression(), fromStatement() {}
        cminor::codedom::Statement* value;
        cminor::codedom::CppObject* fromForInitStatement;
        cminor::codedom::CppObject* fromCondition;
        cminor::codedom::CppObject* fromExpression;
        cminor::codedom::Statement* fromStatement;
    };
};

class StatementGrammar::ForInitStatementRule : public cminor::parsing::Rule
{
public:
    ForInitStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ForInitStatementRule>(this, &ForInitStatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ForInitStatementRule>(this, &ForInitStatementRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ForInitStatementRule>(this, &ForInitStatementRule::A2Action));
        cminor::parsing::NonterminalParser* emptyStatementNonterminalParser = GetNonterminal(ToUtf32("EmptyStatement"));
        emptyStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForInitStatementRule>(this, &ForInitStatementRule::PostEmptyStatement));
        cminor::parsing::NonterminalParser* expressionStatementNonterminalParser = GetNonterminal(ToUtf32("ExpressionStatement"));
        expressionStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForInitStatementRule>(this, &ForInitStatementRule::PostExpressionStatement));
        cminor::parsing::NonterminalParser* simpleDeclarationNonterminalParser = GetNonterminal(ToUtf32("SimpleDeclaration"));
        simpleDeclarationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ForInitStatementRule>(this, &ForInitStatementRule::PostSimpleDeclaration));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromEmptyStatement;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromExpressionStatement;
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromEmptyStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(fromEmptyStatement_value.get());
            stack.pop();
        }
    }
    void PostExpressionStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpressionStatement_value = std::move(stack.top());
            context->fromExpressionStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(fromExpressionStatement_value.get());
            stack.pop();
        }
    }
    void PostSimpleDeclaration(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSimpleDeclaration_value = std::move(stack.top());
            context->fromSimpleDeclaration = *static_cast<cminor::parsing::ValueObject<cminor::codedom::SimpleDeclaration*>*>(fromSimpleDeclaration_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromEmptyStatement(), fromExpressionStatement(), fromSimpleDeclaration() {}
        cminor::codedom::CppObject* value;
        cminor::codedom::Statement* fromEmptyStatement;
        cminor::codedom::Statement* fromExpressionStatement;
        cminor::codedom::SimpleDeclaration* fromSimpleDeclaration;
    };
};

class StatementGrammar::JumpStatementRule : public cminor::parsing::Rule
{
public:
    JumpStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Statement*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<JumpStatementRule>(this, &JumpStatementRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<JumpStatementRule>(this, &JumpStatementRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<JumpStatementRule>(this, &JumpStatementRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<JumpStatementRule>(this, &JumpStatementRule::A3Action));
        cminor::parsing::NonterminalParser* breakStatementNonterminalParser = GetNonterminal(ToUtf32("BreakStatement"));
        breakStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<JumpStatementRule>(this, &JumpStatementRule::PostBreakStatement));
        cminor::parsing::NonterminalParser* continueStatementNonterminalParser = GetNonterminal(ToUtf32("ContinueStatement"));
        continueStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<JumpStatementRule>(this, &JumpStatementRule::PostContinueStatement));
        cminor::parsing::NonterminalParser* returnStatementNonterminalParser = GetNonterminal(ToUtf32("ReturnStatement"));
        returnStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<JumpStatementRule>(this, &JumpStatementRule::PostReturnStatement));
        cminor::parsing::NonterminalParser* gotoStatementNonterminalParser = GetNonterminal(ToUtf32("GotoStatement"));
        gotoStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<JumpStatementRule>(this, &JumpStatementRule::PostGotoStatement));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromBreakStatement;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromContinueStatement;
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromReturnStatement;
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromBreakStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(fromBreakStatement_value.get());
            stack.pop();
        }
    }
    void PostContinueStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromContinueStatement_value = std::move(stack.top());
            context->fromContinueStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(fromContinueStatement_value.get());
            stack.pop();
        }
    }
    void PostReturnStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromReturnStatement_value = std::move(stack.top());
            context->fromReturnStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(fromReturnStatement_value.get());
            stack.pop();
        }
    }
    void PostGotoStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromGotoStatement_value = std::move(stack.top());
            context->fromGotoStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Statement*>*>(fromGotoStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromBreakStatement(), fromContinueStatement(), fromReturnStatement(), fromGotoStatement() {}
        cminor::codedom::Statement* value;
        cminor::codedom::Statement* fromBreakStatement;
        cminor::codedom::Statement* fromContinueStatement;
        cminor::codedom::Statement* fromReturnStatement;
        cminor::codedom::Statement* fromGotoStatement;
    };
};

class StatementGrammar::BreakStatementRule : public cminor::parsing::Rule
{
public:
    BreakStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Statement*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BreakStatementRule>(this, &BreakStatementRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new BreakStatement;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::codedom::Statement* value;
    };
};

class StatementGrammar::ContinueStatementRule : public cminor::parsing::Rule
{
public:
    ContinueStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Statement*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ContinueStatementRule>(this, &ContinueStatementRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ContinueStatement;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::codedom::Statement* value;
    };
};

class StatementGrammar::ReturnStatementRule : public cminor::parsing::Rule
{
public:
    ReturnStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Statement*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ReturnStatementRule>(this, &ReturnStatementRule::A0Action));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal(ToUtf32("Expression"));
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ReturnStatementRule>(this, &ReturnStatementRule::PostExpression));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromExpression() {}
        cminor::codedom::Statement* value;
        cminor::codedom::CppObject* fromExpression;
    };
};

class StatementGrammar::GotoStatementRule : public cminor::parsing::Rule
{
public:
    GotoStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Statement*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<GotoStatementRule>(this, &GotoStatementRule::A0Action));
        cminor::parsing::NonterminalParser* gotoTargetNonterminalParser = GetNonterminal(ToUtf32("GotoTarget"));
        gotoTargetNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<GotoStatementRule>(this, &GotoStatementRule::PostGotoTarget));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromGotoTarget = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromGotoTarget_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromGotoTarget() {}
        cminor::codedom::Statement* value;
        std::u32string fromGotoTarget;
    };
};

class StatementGrammar::GotoTargetRule : public cminor::parsing::Rule
{
public:
    GotoTargetRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("std::u32string"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<std::u32string>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<GotoTargetRule>(this, &GotoTargetRule::A0Action));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal(ToUtf32("Identifier"));
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<GotoTargetRule>(this, &GotoTargetRule::PostIdentifier));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromIdentifier = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromIdentifier() {}
        std::u32string value;
        std::u32string fromIdentifier;
    };
};

class StatementGrammar::DeclarationStatementRule : public cminor::parsing::Rule
{
public:
    DeclarationStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Statement*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Statement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeclarationStatementRule>(this, &DeclarationStatementRule::A0Action));
        cminor::parsing::NonterminalParser* blockDeclarationNonterminalParser = GetNonterminal(ToUtf32("BlockDeclaration"));
        blockDeclarationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DeclarationStatementRule>(this, &DeclarationStatementRule::PostBlockDeclaration));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromBlockDeclaration = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromBlockDeclaration_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromBlockDeclaration() {}
        cminor::codedom::Statement* value;
        cminor::codedom::CppObject* fromBlockDeclaration;
    };
};

class StatementGrammar::ConditionRule : public cminor::parsing::Rule
{
public:
    ConditionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<TypeId>"), ToUtf32("ti")));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConditionRule>(this, &ConditionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConditionRule>(this, &ConditionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConditionRule>(this, &ConditionRule::A2Action));
        cminor::parsing::NonterminalParser* typeIdNonterminalParser = GetNonterminal(ToUtf32("TypeId"));
        typeIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConditionRule>(this, &ConditionRule::PostTypeId));
        cminor::parsing::NonterminalParser* declaratorNonterminalParser = GetNonterminal(ToUtf32("Declarator"));
        declaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConditionRule>(this, &ConditionRule::PostDeclarator));
        cminor::parsing::NonterminalParser* assignmentExpressionNonterminalParser = GetNonterminal(ToUtf32("AssignmentExpression"));
        assignmentExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConditionRule>(this, &ConditionRule::PostAssignmentExpression));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal(ToUtf32("Expression"));
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConditionRule>(this, &ConditionRule::PostExpression));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ti.reset(context->fromTypeId);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ConditionWithDeclarator(context->ti.release(), context->fromDeclarator, context->fromAssignmentExpression);
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromTypeId = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TypeId*>*>(fromTypeId_value.get());
            stack.pop();
        }
    }
    void PostDeclarator(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDeclarator_value = std::move(stack.top());
            context->fromDeclarator = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromDeclarator_value.get());
            stack.pop();
        }
    }
    void PostAssignmentExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAssignmentExpression_value = std::move(stack.top());
            context->fromAssignmentExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromAssignmentExpression_value.get());
            stack.pop();
        }
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), ti(), fromTypeId(), fromDeclarator(), fromAssignmentExpression(), fromExpression() {}
        cminor::codedom::CppObject* value;
        std::unique_ptr<TypeId> ti;
        cminor::codedom::TypeId* fromTypeId;
        std::u32string fromDeclarator;
        cminor::codedom::CppObject* fromAssignmentExpression;
        cminor::codedom::CppObject* fromExpression;
    };
};

class StatementGrammar::TryStatementRule : public cminor::parsing::Rule
{
public:
    TryStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::TryStatement*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::TryStatement*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TryStatementRule>(this, &TryStatementRule::A0Action));
        cminor::parsing::NonterminalParser* compoundStatementNonterminalParser = GetNonterminal(ToUtf32("CompoundStatement"));
        compoundStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TryStatementRule>(this, &TryStatementRule::PostCompoundStatement));
        cminor::parsing::NonterminalParser* handlerSeqNonterminalParser = GetNonterminal(ToUtf32("HandlerSeq"));
        handlerSeqNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<TryStatementRule>(this, &TryStatementRule::PreHandlerSeq));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromCompoundStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CompoundStatement*>*>(fromCompoundStatement_value.get());
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
        cminor::codedom::TryStatement* value;
        cminor::codedom::CompoundStatement* fromCompoundStatement;
    };
};

class StatementGrammar::HandlerSeqRule : public cminor::parsing::Rule
{
public:
    HandlerSeqRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("TryStatement*"), ToUtf32("st")));
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
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<HandlerSeqRule>(this, &HandlerSeqRule::A0Action));
        cminor::parsing::NonterminalParser* handlerNonterminalParser = GetNonterminal(ToUtf32("Handler"));
        handlerNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<HandlerSeqRule>(this, &HandlerSeqRule::PostHandler));
        cminor::parsing::NonterminalParser* handlerSeqNonterminalParser = GetNonterminal(ToUtf32("HandlerSeq"));
        handlerSeqNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<HandlerSeqRule>(this, &HandlerSeqRule::PreHandlerSeq));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromHandler = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Handler*>*>(fromHandler_value.get());
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
        cminor::codedom::Handler* fromHandler;
    };
};

class StatementGrammar::HandlerRule : public cminor::parsing::Rule
{
public:
    HandlerRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Handler*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Handler*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<HandlerRule>(this, &HandlerRule::A0Action));
        cminor::parsing::NonterminalParser* exceptionDeclarationNonterminalParser = GetNonterminal(ToUtf32("ExceptionDeclaration"));
        exceptionDeclarationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<HandlerRule>(this, &HandlerRule::PostExceptionDeclaration));
        cminor::parsing::NonterminalParser* compoundStatementNonterminalParser = GetNonterminal(ToUtf32("CompoundStatement"));
        compoundStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<HandlerRule>(this, &HandlerRule::PostCompoundStatement));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromExceptionDeclaration = *static_cast<cminor::parsing::ValueObject<cminor::codedom::ExceptionDeclaration*>*>(fromExceptionDeclaration_value.get());
            stack.pop();
        }
    }
    void PostCompoundStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCompoundStatement_value = std::move(stack.top());
            context->fromCompoundStatement = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CompoundStatement*>*>(fromCompoundStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromExceptionDeclaration(), fromCompoundStatement() {}
        cminor::codedom::Handler* value;
        cminor::codedom::ExceptionDeclaration* fromExceptionDeclaration;
        cminor::codedom::CompoundStatement* fromCompoundStatement;
    };
};

class StatementGrammar::ExceptionDeclarationRule : public cminor::parsing::Rule
{
public:
    ExceptionDeclarationRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::ExceptionDeclaration*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<ExceptionDeclaration>"), ToUtf32("ed")));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::ExceptionDeclaration*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExceptionDeclarationRule>(this, &ExceptionDeclarationRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExceptionDeclarationRule>(this, &ExceptionDeclarationRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExceptionDeclarationRule>(this, &ExceptionDeclarationRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExceptionDeclarationRule>(this, &ExceptionDeclarationRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction(ToUtf32("A4"));
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExceptionDeclarationRule>(this, &ExceptionDeclarationRule::A4Action));
        cminor::parsing::NonterminalParser* typeSpecifierSeqNonterminalParser = GetNonterminal(ToUtf32("TypeSpecifierSeq"));
        typeSpecifierSeqNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ExceptionDeclarationRule>(this, &ExceptionDeclarationRule::PreTypeSpecifierSeq));
        cminor::parsing::NonterminalParser* declaratorNonterminalParser = GetNonterminal(ToUtf32("Declarator"));
        declaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ExceptionDeclarationRule>(this, &ExceptionDeclarationRule::PostDeclarator));
        cminor::parsing::NonterminalParser* abstractDeclaratorNonterminalParser = GetNonterminal(ToUtf32("AbstractDeclarator"));
        abstractDeclaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ExceptionDeclarationRule>(this, &ExceptionDeclarationRule::PostAbstractDeclarator));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ed.reset(new ExceptionDeclaration);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->ed.release();
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ed->GetTypeId()->Declarator() = context->fromDeclarator;
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ed->GetTypeId()->Declarator() = context->fromAbstractDeclarator;
    }
    void A4Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value->CatchAll() = true;
    }
    void PreTypeSpecifierSeq(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::TypeId*>(context->ed->GetTypeId())));
    }
    void PostDeclarator(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDeclarator_value = std::move(stack.top());
            context->fromDeclarator = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromDeclarator_value.get());
            stack.pop();
        }
    }
    void PostAbstractDeclarator(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAbstractDeclarator_value = std::move(stack.top());
            context->fromAbstractDeclarator = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromAbstractDeclarator_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), ed(), fromDeclarator(), fromAbstractDeclarator() {}
        cminor::codedom::ExceptionDeclaration* value;
        std::unique_ptr<ExceptionDeclaration> ed;
        std::u32string fromDeclarator;
        std::u32string fromAbstractDeclarator;
    };
};

void StatementGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar(ToUtf32("cminor.code.DeclarationGrammar"));
    if (!grammar0)
    {
        grammar0 = cminor::code::DeclarationGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar(ToUtf32("cminor.code.ExpressionGrammar"));
    if (!grammar1)
    {
        grammar1 = cminor::code::ExpressionGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar(ToUtf32("cminor.code.DeclaratorGrammar"));
    if (!grammar2)
    {
        grammar2 = cminor::code::DeclaratorGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    cminor::parsing::Grammar* grammar3 = pd->GetGrammar(ToUtf32("cminor.code.IdentifierGrammar"));
    if (!grammar3)
    {
        grammar3 = cminor::code::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
    cminor::parsing::Grammar* grammar4 = pd->GetGrammar(ToUtf32("cminor.parsing.stdlib"));
    if (!grammar4)
    {
        grammar4 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar4);
}

void StatementGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("SimpleDeclaration"), this, ToUtf32("DeclarationGrammar.SimpleDeclaration")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Expression"), this, ToUtf32("ExpressionGrammar.Expression")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("BlockDeclaration"), this, ToUtf32("DeclarationGrammar.BlockDeclaration")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("ConstantExpression"), this, ToUtf32("ExpressionGrammar.ConstantExpression")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("AssignmentExpression"), this, ToUtf32("ExpressionGrammar.AssignmentExpression")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("TypeId"), this, ToUtf32("DeclaratorGrammar.TypeId")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("TypeSpecifierSeq"), this, ToUtf32("DeclaratorGrammar.TypeSpecifierSeq")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Declarator"), this, ToUtf32("DeclaratorGrammar.Declarator")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Identifier"), this, ToUtf32("IdentifierGrammar.Identifier")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("AbstractDeclarator"), this, ToUtf32("DeclaratorGrammar.AbstractDeclarator")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("spaces_and_comments"), this, ToUtf32("cminor.parsing.stdlib.spaces_and_comments")));
    AddRule(new StatementRule(ToUtf32("Statement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::AlternativeParser(
                                    new cminor::parsing::AlternativeParser(
                                        new cminor::parsing::ActionParser(ToUtf32("A0"),
                                            new cminor::parsing::NonterminalParser(ToUtf32("LabeledStatement"), ToUtf32("LabeledStatement"), 0)),
                                        new cminor::parsing::ActionParser(ToUtf32("A1"),
                                            new cminor::parsing::NonterminalParser(ToUtf32("EmptyStatement"), ToUtf32("EmptyStatement"), 0))),
                                    new cminor::parsing::ActionParser(ToUtf32("A2"),
                                        new cminor::parsing::NonterminalParser(ToUtf32("CompoundStatement"), ToUtf32("CompoundStatement"), 0))),
                                new cminor::parsing::ActionParser(ToUtf32("A3"),
                                    new cminor::parsing::NonterminalParser(ToUtf32("SelectionStatement"), ToUtf32("SelectionStatement"), 0))),
                            new cminor::parsing::ActionParser(ToUtf32("A4"),
                                new cminor::parsing::NonterminalParser(ToUtf32("IterationStatement"), ToUtf32("IterationStatement"), 0))),
                        new cminor::parsing::ActionParser(ToUtf32("A5"),
                            new cminor::parsing::NonterminalParser(ToUtf32("JumpStatement"), ToUtf32("JumpStatement"), 0))),
                    new cminor::parsing::ActionParser(ToUtf32("A6"),
                        new cminor::parsing::NonterminalParser(ToUtf32("DeclarationStatement"), ToUtf32("DeclarationStatement"), 0))),
                new cminor::parsing::ActionParser(ToUtf32("A7"),
                    new cminor::parsing::NonterminalParser(ToUtf32("TryStatement"), ToUtf32("TryStatement"), 0))),
            new cminor::parsing::ActionParser(ToUtf32("A8"),
                new cminor::parsing::NonterminalParser(ToUtf32("ExpressionStatement"), ToUtf32("ExpressionStatement"), 0)))));
    AddRule(new LabeledStatementRule(ToUtf32("LabeledStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("Label"), ToUtf32("Label"), 0),
                            new cminor::parsing::DifferenceParser(
                                new cminor::parsing::CharParser(':'),
                                new cminor::parsing::StringParser(ToUtf32("::")))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("s1"), ToUtf32("Statement"), 0)))),
                new cminor::parsing::ActionParser(ToUtf32("A1"),
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::KeywordParser(ToUtf32("case")),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::NonterminalParser(ToUtf32("ConstantExpression"), ToUtf32("ConstantExpression"), 0))),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::CharParser(':'))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("s2"), ToUtf32("Statement"), 0))))),
            new cminor::parsing::ActionParser(ToUtf32("A2"),
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::KeywordParser(ToUtf32("default")),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser(':'))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("s3"), ToUtf32("Statement"), 0)))))));
    AddRule(new LabelRule(ToUtf32("Label"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::NonterminalParser(ToUtf32("Identifier"), ToUtf32("Identifier"), 0))));
    AddRule(new EmptyStatementRule(ToUtf32("EmptyStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::CharParser(';'))));
    AddRule(new ExpressionStatementRule(ToUtf32("ExpressionStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::NonterminalParser(ToUtf32("Expression"), ToUtf32("Expression"), 0),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new CompoundStatementRule(ToUtf32("CompoundStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::CharParser('{')),
                new cminor::parsing::KleeneStarParser(
                    new cminor::parsing::ActionParser(ToUtf32("A1"),
                        new cminor::parsing::NonterminalParser(ToUtf32("Statement"), ToUtf32("Statement"), 0)))),
            new cminor::parsing::ActionParser(ToUtf32("A2"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser('}'))))));
    AddRule(new SelectionStatementRule(ToUtf32("SelectionStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("IfStatement"), ToUtf32("IfStatement"), 0)),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::NonterminalParser(ToUtf32("SwitchStatement"), ToUtf32("SwitchStatement"), 0)))));
    AddRule(new IfStatementRule(ToUtf32("IfStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::KeywordParser(ToUtf32("if")),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::CharParser('('))),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser(ToUtf32("Condition"), ToUtf32("Condition"), 0))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser(')'))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("thenStatement"), ToUtf32("Statement"), 0))),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::KeywordParser(ToUtf32("else")),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("elseStatement"), ToUtf32("Statement"), 0))))))));
    AddRule(new SwitchStatementRule(ToUtf32("SwitchStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::KeywordParser(ToUtf32("switch")),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser('('))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("Condition"), ToUtf32("Condition"), 0))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(')'))),
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("Statement"), ToUtf32("Statement"), 0))))));
    AddRule(new IterationStatementRule(ToUtf32("IterationStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::NonterminalParser(ToUtf32("WhileStatement"), ToUtf32("WhileStatement"), 0)),
                new cminor::parsing::ActionParser(ToUtf32("A1"),
                    new cminor::parsing::NonterminalParser(ToUtf32("DoStatement"), ToUtf32("DoStatement"), 0))),
            new cminor::parsing::ActionParser(ToUtf32("A2"),
                new cminor::parsing::NonterminalParser(ToUtf32("ForStatement"), ToUtf32("ForStatement"), 0)))));
    AddRule(new WhileStatementRule(ToUtf32("WhileStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::KeywordParser(ToUtf32("while")),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser('('))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("Condition"), ToUtf32("Condition"), 0))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(')'))),
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("Statement"), ToUtf32("Statement"), 0))))));
    AddRule(new DoStatementRule(ToUtf32("DoStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::KeywordParser(ToUtf32("do")),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::NonterminalParser(ToUtf32("Statement"), ToUtf32("Statement"), 0))),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::KeywordParser(ToUtf32("while")))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser('('))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("Expression"), ToUtf32("Expression"), 0))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(')'))),
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new ForStatementRule(ToUtf32("ForStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::KeywordParser(ToUtf32("for")),
                                    new cminor::parsing::ExpectationParser(
                                        new cminor::parsing::CharParser('('))),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::NonterminalParser(ToUtf32("ForInitStatement"), ToUtf32("ForInitStatement"), 0))),
                            new cminor::parsing::OptionalParser(
                                new cminor::parsing::NonterminalParser(ToUtf32("Condition"), ToUtf32("Condition"), 0))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser(';'))),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("Expression"), ToUtf32("Expression"), 0))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(')'))),
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("Statement"), ToUtf32("Statement"), 0))))));
    AddRule(new ForInitStatementRule(ToUtf32("ForInitStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::NonterminalParser(ToUtf32("EmptyStatement"), ToUtf32("EmptyStatement"), 0)),
                new cminor::parsing::ActionParser(ToUtf32("A1"),
                    new cminor::parsing::NonterminalParser(ToUtf32("ExpressionStatement"), ToUtf32("ExpressionStatement"), 0))),
            new cminor::parsing::ActionParser(ToUtf32("A2"),
                new cminor::parsing::NonterminalParser(ToUtf32("SimpleDeclaration"), ToUtf32("SimpleDeclaration"), 0)))));
    AddRule(new JumpStatementRule(ToUtf32("JumpStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::ActionParser(ToUtf32("A0"),
                        new cminor::parsing::NonterminalParser(ToUtf32("BreakStatement"), ToUtf32("BreakStatement"), 0)),
                    new cminor::parsing::ActionParser(ToUtf32("A1"),
                        new cminor::parsing::NonterminalParser(ToUtf32("ContinueStatement"), ToUtf32("ContinueStatement"), 0))),
                new cminor::parsing::ActionParser(ToUtf32("A2"),
                    new cminor::parsing::NonterminalParser(ToUtf32("ReturnStatement"), ToUtf32("ReturnStatement"), 0))),
            new cminor::parsing::ActionParser(ToUtf32("A3"),
                new cminor::parsing::NonterminalParser(ToUtf32("GotoStatement"), ToUtf32("GotoStatement"), 0)))));
    AddRule(new BreakStatementRule(ToUtf32("BreakStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::KeywordParser(ToUtf32("break")),
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new ContinueStatementRule(ToUtf32("ContinueStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::KeywordParser(ToUtf32("continue")),
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new ReturnStatementRule(ToUtf32("ReturnStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::KeywordParser(ToUtf32("return")),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("Expression"), ToUtf32("Expression"), 0))),
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new GotoStatementRule(ToUtf32("GotoStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::KeywordParser(ToUtf32("goto")),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("GotoTarget"), ToUtf32("GotoTarget"), 0))),
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new GotoTargetRule(ToUtf32("GotoTarget"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::NonterminalParser(ToUtf32("Identifier"), ToUtf32("Identifier"), 0))));
    AddRule(new DeclarationStatementRule(ToUtf32("DeclarationStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::NonterminalParser(ToUtf32("BlockDeclaration"), ToUtf32("BlockDeclaration"), 0))));
    AddRule(new ConditionRule(ToUtf32("Condition"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::ActionParser(ToUtf32("A0"),
                            new cminor::parsing::NonterminalParser(ToUtf32("TypeId"), ToUtf32("TypeId"), 0)),
                        new cminor::parsing::NonterminalParser(ToUtf32("Declarator"), ToUtf32("Declarator"), 0)),
                    new cminor::parsing::CharParser('=')),
                new cminor::parsing::ActionParser(ToUtf32("A1"),
                    new cminor::parsing::NonterminalParser(ToUtf32("AssignmentExpression"), ToUtf32("AssignmentExpression"), 0))),
            new cminor::parsing::ActionParser(ToUtf32("A2"),
                new cminor::parsing::NonterminalParser(ToUtf32("Expression"), ToUtf32("Expression"), 0)))));
    AddRule(new TryStatementRule(ToUtf32("TryStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::KeywordParser(ToUtf32("try")),
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("CompoundStatement"), ToUtf32("CompoundStatement"), 0)))),
            new cminor::parsing::ExpectationParser(
                new cminor::parsing::NonterminalParser(ToUtf32("HandlerSeq"), ToUtf32("HandlerSeq"), 1)))));
    AddRule(new HandlerSeqRule(ToUtf32("HandlerSeq"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("Handler"), ToUtf32("Handler"), 0))),
            new cminor::parsing::OptionalParser(
                new cminor::parsing::NonterminalParser(ToUtf32("HandlerSeq"), ToUtf32("HandlerSeq"), 1)))));
    AddRule(new HandlerRule(ToUtf32("Handler"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::KeywordParser(ToUtf32("catch")),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser('('))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("ExceptionDeclaration"), ToUtf32("ExceptionDeclaration"), 0))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(')'))),
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("CompoundStatement"), ToUtf32("CompoundStatement"), 0))))));
    AddRule(new ExceptionDeclarationRule(ToUtf32("ExceptionDeclaration"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::EmptyParser()),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("TypeSpecifierSeq"), ToUtf32("TypeSpecifierSeq"), 1),
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::ActionParser(ToUtf32("A2"),
                                    new cminor::parsing::NonterminalParser(ToUtf32("Declarator"), ToUtf32("Declarator"), 0)),
                                new cminor::parsing::ActionParser(ToUtf32("A3"),
                                    new cminor::parsing::NonterminalParser(ToUtf32("AbstractDeclarator"), ToUtf32("AbstractDeclarator"), 0))),
                            new cminor::parsing::EmptyParser())),
                    new cminor::parsing::ActionParser(ToUtf32("A4"),
                        new cminor::parsing::StringParser(ToUtf32("..."))))))));
    SetStartRuleName(ToUtf32("CompoundStatement"));
    SetSkipRuleName(ToUtf32("spaces_and_comments"));
}

} } // namespace cminor.code
