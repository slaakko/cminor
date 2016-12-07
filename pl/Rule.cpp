// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/pl/Rule.hpp>
#include <cminor/pl/Scope.hpp>
#include <cminor/pl/Action.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pl/XmlLog.hpp>
#include <cminor/pl/Visitor.hpp>
#include <cminor/pl/Grammar.hpp>
#include <cminor/pom/Visitor.hpp>
#include <cminor/pom/Expression.hpp>

namespace cminor { namespace parsing {

AttrOrVariable::AttrOrVariable(const std::string& typeName_, const std::string& name_): typeName(typeName_), name(name_) 
{
}

Rule::Rule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
    Parser(name_, "<" + name_ + ">"),
    grammar(nullptr),
    definition(definition_),
    inheritedAttributes(),
    localVariables(),
    valueTypeName(),
    actions(),
    nonterminals(),
    ccCount(0), 
    ccRule(false), 
    ccStart(),
    ccEnd(),
    ccSkip(false)
{
    Own(definition);
    SetScope(new Scope(Name(), EnclosingScope()));
}

Rule::Rule(const std::string& name_, Scope* enclosingScope_):
    Parser(name_, "<" + name_ + ">"),
    grammar(nullptr),
    definition(),
    inheritedAttributes(),
    localVariables(),
    valueTypeName(),
    actions(),
    nonterminals(),
    ccCount(0),
    ccRule(false),
    ccStart(),
    ccEnd(),
    ccSkip(false)
{
    SetScope(new Scope(Name(), EnclosingScope()));
}

void Rule::SetDefinition(Parser* definition_) 
{ 
    definition = definition_; 
    Own(definition);
}

void Rule::AddAction(ActionParser* action)
{
    Own(action);
    actions.push_back(action);
    action->SetEnclosingScope(GetScope());
    action->AddToScope();
}

ActionParser* Rule::GetAction(const std::string& actionName) const
{
    ParsingObject* object = GetScope()->Get(actionName);
    if (!object)
    {
        ThrowException("action '" + actionName + "' not found in rule '" + FullName() + "'", GetSpan());
    }
    else if (object->IsActionParser())
    {
        return static_cast<ActionParser*>(object);
    }
    else
    {
        ThrowException("'" + actionName + "' is not an action", GetSpan());
    }
    return nullptr;
}

void Rule::AddNonterminal(NonterminalParser* nonterminal)
{
    Own(nonterminal);
    nonterminals.push_back(nonterminal);
    nonterminal->SetEnclosingScope(GetScope());
    nonterminal->AddToScope();
}

NonterminalParser* Rule::GetNonterminal(const std::string& nonterminalName) const
{
    ParsingObject* object = GetScope()->Get(nonterminalName);
    if (!object)
    {
        ThrowException("nonterminal '" + nonterminalName + "' not found", GetSpan());
    }
    else if (object->IsNonterminalParser())
    {
        return static_cast<NonterminalParser*>(object);
    }
    else
    {
        ThrowException("'" + nonterminalName + "' is not a nonterminal", GetSpan());
    }
    return nullptr;
}

using cminor::pom::IdExpr;

class CodeExpandingVisitor : public cminor::pom::Visitor
{
public:
    CodeExpandingVisitor(std::unordered_map<std::string, std::string>& expandMap_): expandMap(expandMap_) {}
    virtual void Visit(IdExpr& expr)
    {
        std::unordered_map<std::string, std::string>::const_iterator i = expandMap.find(expr.Name());
        if (i != expandMap.end())
        {
            expr.SetName(i->second);
        }
    }
private:
    std::unordered_map<std::string, std::string>& expandMap;
};

void Rule::ExpandCode()
{
    std::unordered_map<std::string, std::string> expandMap;
    expandMap["value"] = "context.value";
    int n = int(nonterminals.size());
    for (int i = 0; i < n; ++i)
    {
        NonterminalParser* nonterminal = nonterminals[i];
        expandMap[nonterminal->Name()] = "context." + nonterminal->ValueFieldName();
    }
    n = int(inheritedAttributes.size());
    for (int i = 0; i < n; ++i)
    {
        const AttrOrVariable& attr = inheritedAttributes[i];
        expandMap[attr.Name()] = "context." + attr.Name();
    }
    n = int(localVariables.size());
    for (int i = 0; i < n; ++i)
    {
        const AttrOrVariable& var = localVariables[i];
        expandMap[var.Name()] = "context." + var.Name();
    }
    CodeExpandingVisitor visitor(expandMap);
    n = int(actions.size());
    for (int i = 0; i < n; ++i)
    {
        ActionParser* action = actions[i];
        action->SuccessCode()->Accept(visitor);
        if (action->FailCode())
        {
            action->FailCode()->Accept(visitor);
        }
    }
    n = int(nonterminals.size());
    for (int i = 0; i < n; ++i)
    {
        NonterminalParser* nonterminal = nonterminals[i];
        if (!nonterminal->Arguments().empty())
        {
            int m = int(nonterminal->Arguments().size());
            for (int j = 0; j < m; ++j)
            {
                cminor::pom::CppObject* argument = nonterminal->Arguments()[j];
                argument->Accept(visitor);
            }
        }
    }
}

Match Rule::Parse(Scanner& scanner, ObjectStack& stack)
{
    if (CC() && ccRule)
    {
        scanner.PushCCRule(this);
    }
    bool writeToLog = !scanner.Skipping() && scanner.Log();
    if (writeToLog)
    {
        scanner.Log()->WriteBeginRule(Name());
        scanner.Log()->IncIndent();
        scanner.Log()->WriteTry(scanner.RestOfLine());
        scanner.Log()->IncIndent();
    }
    int startIndex = scanner.GetSpan().Start();
    Enter(stack);
    Match match = definition ? definition->Parse(scanner, stack) : Match::Nothing();
    Leave(stack, match.Hit());
    if (writeToLog)
    {
        scanner.Log()->DecIndent();
        if (match.Hit())
        {
            std::string matched(scanner.Start() + startIndex, scanner.Start() + scanner.GetSpan().Start());
            scanner.Log()->WriteSuccess(matched);
        }
        else
        {
            scanner.Log()->WriteFail();
        }
        scanner.Log()->DecIndent();
        scanner.Log()->WriteEndRule(Name());
    }
    if (CC())
    {
        if (scanner.Synchronizing() && ccRule && ccCount == 0)
        {
            scanner.SetSynchronizing(false);
        }
        if (ccRule)
        {
            scanner.PopCCRule();
        }
    }
    return match;
}

void Rule::Accept(Visitor& visitor)
{
    visitor.BeginVisit(*this);
    if (definition)
    {
        definition->Accept(visitor);
    }
    visitor.EndVisit(*this);
}

std::string GetPrefix(const std::string& fullName)
{
    std::string::size_type dotPos = fullName.find('.');
    if (dotPos != std::string::npos)
    {
        return fullName.substr(0, dotPos);
    }
    return fullName;
}

std::string GetCommonName(const std::string& qualifiedId)
{
    std::string::size_type lastDotPos = qualifiedId.rfind('.');
    if (lastDotPos != std::string::npos)
    {
        return qualifiedId.substr(lastDotPos + 1);
    }
    return qualifiedId;
}

RuleLink::RuleLink(const std::string& name_, Grammar* grammar_, const std::string& linkedRuleName_): ParsingObject(name_, grammar_->GetScope()), linkedRuleName(linkedRuleName_), grammar(grammar_)
{
    std::string prefix = GetPrefix(linkedRuleName);
    if (prefix == "stdlib")
    {
        linkedRuleName = "cminor.parsing.stdlib." + GetCommonName(linkedRuleName);
    }
}

RuleLink::RuleLink(Grammar* grammar_, const std::string& linkedRuleName_): ParsingObject(GetCommonName(linkedRuleName_), grammar_->GetScope()), linkedRuleName(linkedRuleName_), grammar(grammar_)
{
    std::string prefix = GetPrefix(linkedRuleName);
    if (prefix == "stdlib")
    {
        linkedRuleName = "cminor.parsing.stdlib." + GetCommonName(linkedRuleName);
    }
}

void RuleLink::AddToScope()
{
    ParsingObject::AddToScope();
    grammar->AddRuleLink(this);
}

void RuleLink::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::parsing
