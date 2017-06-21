// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_PARSING_PARSING_OBJECT_INCLUDED
#define CMINOR_PARSING_PARSING_OBJECT_INCLUDED
#include <cminor/parsing/Scanner.hpp>
#include <memory>

namespace cminor { namespace parsing {

class Visitor;
class Scope;

class ParsingObject
{
public:
    static const int external = -1;
    ParsingObject(const std::u32string& name_);
    ParsingObject(const std::u32string& name_, Scope* enclosingScope_);
    virtual ~ParsingObject();
    virtual void Accept(Visitor& visitor) = 0;
    void Own(ParsingObject* object);
    bool IsOwned() const { return isOwned; }
    void SetOwned() { isOwned = true; }
    void ResetOwned() { isOwned = false; }
    void SetOwner(int owner_) { owner = owner_; }
    int Owner() const { return owner; }
    void SetExternal() { owner = external; }
    bool IsExternal() const { return owner == external; }
    const std::u32string& Name() const { return name; }
    void SetName(const std::u32string& name_) { name = name_; }
    virtual std::u32string FullName() const;
    Scope* EnclosingScope() const { return enclosingScope; }
    void SetEnclosingScope(Scope* enclosingScope_) { enclosingScope = enclosingScope_; }
    void SetScope(Scope* scope_);
    Scope* GetScope() const { if (scope == nullptr) return enclosingScope; else return scope; }
    virtual bool IsActionParser() const { return false; }
    virtual bool IsNonterminalParser() const { return false; }
    virtual bool IsRule() const { return false; }
    virtual bool IsRuleLink() const { return false; }
    virtual bool IsNamespace() const { return false; }
    virtual void AddToScope();
    void SetSpan(const Span& span_) { span = span_; }
    const Span& GetSpan() const { return span; }
private:
    std::u32string name;
    std::vector<std::unique_ptr<ParsingObject>> ownedObjects;
    int owner;
    bool isOwned;
    Scope* enclosingScope;
    Scope* scope;
    Span span;
};

} } // namespace cminor::parsing

#endif // CMINOR_PARSING_PARSING_OBJECT_INCLUDED
