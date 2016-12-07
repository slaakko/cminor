#include <cminor/pl/Action.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/StdLib.hpp>
#include <cminor/pl/XmlLog.hpp>
#include <sstream>

namespace cminor { namespace parsing {

using namespace cminor::parsing;

stdlib* stdlib::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

stdlib* stdlib::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    stdlib* grammar(new stdlib(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

stdlib::stdlib(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("stdlib", parsingDomain_->GetNamespaceScope("cminor.parsing"), parsingDomain_)
{
    SetOwner(0);
}

class stdlib::intRule : public cminor::parsing::Rule
{
public:
    intRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("int");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<int>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<intRule>(this, &intRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        std::stringstream s;
        s.str(std::string(matchBegin, matchEnd));
        s >> context.value;
    }
private:
    struct Context
    {
        Context(): value() {}
        int value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class stdlib::uintRule : public cminor::parsing::Rule
{
public:
    uintRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("unsigned");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<unsigned>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<uintRule>(this, &uintRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        std::stringstream s;
        s.str(std::string(matchBegin, matchEnd));
        s >> context.value;
    }
private:
    struct Context
    {
        Context(): value() {}
        unsigned value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class stdlib::longRule : public cminor::parsing::Rule
{
public:
    longRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("int64_t");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<int64_t>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<longRule>(this, &longRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        std::stringstream s;
        s.str(std::string(matchBegin, matchEnd));
        s >> context.value;
    }
private:
    struct Context
    {
        Context(): value() {}
        int64_t value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class stdlib::ulongRule : public cminor::parsing::Rule
{
public:
    ulongRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("uint64_t");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<uint64_t>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ulongRule>(this, &ulongRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        std::stringstream s;
        s.str(std::string(matchBegin, matchEnd));
        s >> context.value;
    }
private:
    struct Context
    {
        Context(): value() {}
        uint64_t value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class stdlib::hexRule : public cminor::parsing::Rule
{
public:
    hexRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("uint64_t");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<uint64_t>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<hexRule>(this, &hexRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        std::stringstream s;
        s.str(std::string(matchBegin, matchEnd));
        s >> std::hex >> context.value;
    }
private:
    struct Context
    {
        Context(): value() {}
        uint64_t value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class stdlib::hex_literalRule : public cminor::parsing::Rule
{
public:
    hex_literalRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("uint64_t");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<uint64_t>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<hex_literalRule>(this, &hex_literalRule::A0Action));
        cminor::parsing::NonterminalParser* hexNonterminalParser = GetNonterminal("hex");
        hexNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<hex_literalRule>(this, &hex_literalRule::Posthex));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromhex;
    }
    void Posthex(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromhex_value = std::move(stack.top());
            context.fromhex = *static_cast<cminor::parsing::ValueObject<uint64_t>*>(fromhex_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromhex() {}
        uint64_t value;
        uint64_t fromhex;
    };
    std::stack<Context> contextStack;
    Context context;
};

class stdlib::realRule : public cminor::parsing::Rule
{
public:
    realRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("double");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<double>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<realRule>(this, &realRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        std::stringstream s;
        s.str(std::string(matchBegin, matchEnd));
        s >> context.value;
    }
private:
    struct Context
    {
        Context(): value() {}
        double value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class stdlib::urealRule : public cminor::parsing::Rule
{
public:
    urealRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("double");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<double>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<urealRule>(this, &urealRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        std::stringstream s;
        s.str(std::string(matchBegin, matchEnd));
        s >> context.value;
    }
private:
    struct Context
    {
        Context(): value() {}
        double value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class stdlib::numberRule : public cminor::parsing::Rule
{
public:
    numberRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("double");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<double>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<numberRule>(this, &numberRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<numberRule>(this, &numberRule::A1Action));
        cminor::parsing::NonterminalParser* rNonterminalParser = GetNonterminal("r");
        rNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<numberRule>(this, &numberRule::Postr));
        cminor::parsing::NonterminalParser* iNonterminalParser = GetNonterminal("i");
        iNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<numberRule>(this, &numberRule::Posti));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromr;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromi;
    }
    void Postr(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromr_value = std::move(stack.top());
            context.fromr = *static_cast<cminor::parsing::ValueObject<double>*>(fromr_value.get());
            stack.pop();
        }
    }
    void Posti(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromi_value = std::move(stack.top());
            context.fromi = *static_cast<cminor::parsing::ValueObject<int>*>(fromi_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromr(), fromi() {}
        double value;
        double fromr;
        int fromi;
    };
    std::stack<Context> contextStack;
    Context context;
};

class stdlib::boolRule : public cminor::parsing::Rule
{
public:
    boolRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("bool");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<bool>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<boolRule>(this, &boolRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<boolRule>(this, &boolRule::A1Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = true;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = false;
    }
private:
    struct Context
    {
        Context(): value() {}
        bool value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class stdlib::identifierRule : public cminor::parsing::Rule
{
public:
    identifierRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<identifierRule>(this, &identifierRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = std::string(matchBegin, matchEnd);
    }
private:
    struct Context
    {
        Context(): value() {}
        std::string value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class stdlib::qualified_idRule : public cminor::parsing::Rule
{
public:
    qualified_idRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<qualified_idRule>(this, &qualified_idRule::A0Action));
        cminor::parsing::NonterminalParser* firstNonterminalParser = GetNonterminal("first");
        firstNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<qualified_idRule>(this, &qualified_idRule::Postfirst));
        cminor::parsing::NonterminalParser* restNonterminalParser = GetNonterminal("rest");
        restNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<qualified_idRule>(this, &qualified_idRule::Postrest));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = std::string(matchBegin, matchEnd);
    }
    void Postfirst(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromfirst_value = std::move(stack.top());
            context.fromfirst = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromfirst_value.get());
            stack.pop();
        }
    }
    void Postrest(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromrest_value = std::move(stack.top());
            context.fromrest = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromrest_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromfirst(), fromrest() {}
        std::string value;
        std::string fromfirst;
        std::string fromrest;
    };
    std::stack<Context> contextStack;
    Context context;
};

class stdlib::escapeRule : public cminor::parsing::Rule
{
public:
    escapeRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("char");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<char>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<escapeRule>(this, &escapeRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<escapeRule>(this, &escapeRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<escapeRule>(this, &escapeRule::A2Action));
        cminor::parsing::NonterminalParser* xNonterminalParser = GetNonterminal("x");
        xNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<escapeRule>(this, &escapeRule::Postx));
        cminor::parsing::NonterminalParser* decimalEscapeNonterminalParser = GetNonterminal("decimalEscape");
        decimalEscapeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<escapeRule>(this, &escapeRule::PostdecimalEscape));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = char(context.fromx);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = char(context.fromdecimalEscape);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        char c = *matchBegin;
        switch (c)
        {
            case 'a': context.value = '\a';
            break;
            case 'b': context.value = '\b';
            break;
            case 'f': context.value = '\f';
            break;
            case 'n': context.value = '\n';
            break;
            case 'r': context.value = '\r';
            break;
            case 't': context.value = '\t';
            break;
            case 'v': context.value = '\v';
            break;
            case '0': context.value = '\0';
            break;
            default: context.value = c;
            break;
        }
    }
    void Postx(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromx_value = std::move(stack.top());
            context.fromx = *static_cast<cminor::parsing::ValueObject<uint64_t>*>(fromx_value.get());
            stack.pop();
        }
    }
    void PostdecimalEscape(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromdecimalEscape_value = std::move(stack.top());
            context.fromdecimalEscape = *static_cast<cminor::parsing::ValueObject<unsigned>*>(fromdecimalEscape_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromx(), fromdecimalEscape() {}
        char value;
        uint64_t fromx;
        unsigned fromdecimalEscape;
    };
    std::stack<Context> contextStack;
    Context context;
};

class stdlib::charRule : public cminor::parsing::Rule
{
public:
    charRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("char");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<char>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<charRule>(this, &charRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<charRule>(this, &charRule::A1Action));
        cminor::parsing::NonterminalParser* escapeNonterminalParser = GetNonterminal("escape");
        escapeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<charRule>(this, &charRule::Postescape));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = *matchBegin;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromescape;
    }
    void Postescape(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromescape_value = std::move(stack.top());
            context.fromescape = *static_cast<cminor::parsing::ValueObject<char>*>(fromescape_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromescape() {}
        char value;
        char fromescape;
    };
    std::stack<Context> contextStack;
    Context context;
};

class stdlib::stringRule : public cminor::parsing::Rule
{
public:
    stringRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<stringRule>(this, &stringRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<stringRule>(this, &stringRule::A1Action));
        cminor::parsing::NonterminalParser* escapeNonterminalParser = GetNonterminal("escape");
        escapeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<stringRule>(this, &stringRule::Postescape));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value.append(std::string(matchBegin, matchEnd));
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value.append(1, context.fromescape);
    }
    void Postescape(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromescape_value = std::move(stack.top());
            context.fromescape = *static_cast<cminor::parsing::ValueObject<char>*>(fromescape_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromescape() {}
        std::string value;
        char fromescape;
    };
    std::stack<Context> contextStack;
    Context context;
};

void stdlib::GetReferencedGrammars()
{
}

void stdlib::CreateRules()
{
    AddRule(new cminor::parsing::Rule("spaces", GetScope(),
        new cminor::parsing::PositiveParser(
            new cminor::parsing::SpaceParser())));
    AddRule(new cminor::parsing::Rule("newline", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::StringParser("\r\n"),
                new cminor::parsing::StringParser("\n")),
            new cminor::parsing::StringParser("\r"))));
    AddRule(new cminor::parsing::Rule("comment", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::NonterminalParser("line_comment", "line_comment", 0),
            new cminor::parsing::NonterminalParser("block_comment", "block_comment", 0))));
    AddRule(new cminor::parsing::Rule("line_comment", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::StringParser("//"),
                new cminor::parsing::KleeneStarParser(
                    new cminor::parsing::CharSetParser("\r\n", true))),
            new cminor::parsing::NonterminalParser("newline", "newline", 0))));
    AddRule(new cminor::parsing::Rule("block_comment", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::StringParser("/*"),
                new cminor::parsing::KleeneStarParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::NonterminalParser("string", "string", 0),
                            new cminor::parsing::NonterminalParser("char", "char", 0)),
                        new cminor::parsing::DifferenceParser(
                            new cminor::parsing::AnyCharParser(),
                            new cminor::parsing::StringParser("*/"))))),
            new cminor::parsing::StringParser("*/"))));
    AddRule(new cminor::parsing::Rule("spaces_and_comments", GetScope(),
        new cminor::parsing::PositiveParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::SpaceParser(),
                new cminor::parsing::NonterminalParser("comment", "comment", 0)))));
    AddRule(new cminor::parsing::Rule("digit_sequence", GetScope(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::PositiveParser(
                new cminor::parsing::DigitParser()))));
    AddRule(new cminor::parsing::Rule("sign", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::CharParser('+'),
            new cminor::parsing::CharParser('-'))));
    AddRule(new intRule("int", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::NonterminalParser("sign", "sign", 0)),
                    new cminor::parsing::NonterminalParser("digit_sequence", "digit_sequence", 0))))));
    AddRule(new uintRule("uint", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::NonterminalParser("digit_sequence", "digit_sequence", 0))));
    AddRule(new longRule("long", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::NonterminalParser("sign", "sign", 0)),
                    new cminor::parsing::NonterminalParser("digit_sequence", "digit_sequence", 0))))));
    AddRule(new ulongRule("ulong", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::NonterminalParser("digit_sequence", "digit_sequence", 0))));
    AddRule(new hexRule("hex", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::TokenParser(
                new cminor::parsing::PositiveParser(
                    new cminor::parsing::HexDigitParser())))));
    AddRule(new hex_literalRule("hex_literal", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::StringParser("0x"),
                        new cminor::parsing::StringParser("0X")),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser("hex", "hex", 0)))))));
    AddRule(new realRule("real", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::NonterminalParser("sign", "sign", 0)),
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::NonterminalParser("fractional_real", "fractional_real", 0),
                        new cminor::parsing::NonterminalParser("exponent_real", "exponent_real", 0)))))));
    AddRule(new urealRule("ureal", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::NonterminalParser("fractional_real", "fractional_real", 0),
                new cminor::parsing::NonterminalParser("exponent_real", "exponent_real", 0)))));
    AddRule(new cminor::parsing::Rule("fractional_real", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::OptionalParser(
                                new cminor::parsing::NonterminalParser("digit_sequence", "digit_sequence", 0)),
                            new cminor::parsing::CharParser('.')),
                        new cminor::parsing::NonterminalParser("digit_sequence", "digit_sequence", 0)),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::NonterminalParser("exponent_part", "exponent_part", 0)))),
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser("digit_sequence", "digit_sequence", 0),
                    new cminor::parsing::CharParser('.'))))));
    AddRule(new cminor::parsing::Rule("exponent_real", GetScope(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::NonterminalParser("digit_sequence", "digit_sequence", 0),
                new cminor::parsing::NonterminalParser("exponent_part", "exponent_part", 0)))));
    AddRule(new cminor::parsing::Rule("exponent_part", GetScope(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharSetParser("eE"),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::NonterminalParser("sign", "sign", 0))),
                new cminor::parsing::NonterminalParser("digit_sequence", "digit_sequence", 0)))));
    AddRule(new numberRule("number", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("r", "real", 0)),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::NonterminalParser("i", "int", 0)))));
    AddRule(new boolRule("bool", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::KeywordParser("true")),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::KeywordParser("false")))));
    AddRule(new identifierRule("identifier", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::LetterParser(),
                        new cminor::parsing::CharParser('_')),
                    new cminor::parsing::KleeneStarParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::LetterParser(),
                                new cminor::parsing::DigitParser()),
                            new cminor::parsing::CharParser('_'))))))));
    AddRule(new qualified_idRule("qualified_id", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser("first", "identifier", 0),
                    new cminor::parsing::KleeneStarParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::CharParser('.'),
                            new cminor::parsing::NonterminalParser("rest", "identifier", 0))))))));
    AddRule(new escapeRule("escape", GetScope(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharParser('\\'),
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::CharSetParser("xX"),
                            new cminor::parsing::ActionParser("A0",
                                new cminor::parsing::NonterminalParser("x", "hex", 0))),
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::CharSetParser("dD"),
                            new cminor::parsing::ActionParser("A1",
                                new cminor::parsing::NonterminalParser("decimalEscape", "uint", 0)))),
                    new cminor::parsing::ActionParser("A2",
                        new cminor::parsing::CharSetParser("dDxX", true)))))));
    AddRule(new charRule("char", GetScope(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('\''),
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::ActionParser("A0",
                            new cminor::parsing::CharSetParser("\\\r\n", true)),
                        new cminor::parsing::ActionParser("A1",
                            new cminor::parsing::NonterminalParser("escape", "escape", 0)))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser('\''))))));
    AddRule(new stringRule("string", GetScope(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('\"'),
                    new cminor::parsing::KleeneStarParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::ActionParser("A0",
                                new cminor::parsing::PositiveParser(
                                    new cminor::parsing::CharSetParser("\"\\\r\n", true))),
                            new cminor::parsing::ActionParser("A1",
                                new cminor::parsing::NonterminalParser("escape", "escape", 0))))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser('\"'))))));
}

} } // namespace cminor.parsing
