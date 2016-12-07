/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#ifndef CMINOR_PARSING_PARSING_OBJECT_INCLUDED
#define CMINOR_PARSING_PARSING_OBJECT_INCLUDED
#include <cminor/pl/Scanner.hpp>
#include <memory>

namespace cminor { namespace parsing {

class Visitor;
class Scope;

class ParsingObject
{
public:
    static const int external = -1;
    ParsingObject(const std::string& name_);
    ParsingObject(const std::string& name_, Scope* enclosingScope_);
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
    const std::string& Name() const { return name; }
    void SetName(const std::string& name_) { name = name_; }
    virtual std::string FullName() const;
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
    std::string name;
    std::vector<std::unique_ptr<ParsingObject>> ownedObjects;
    int owner;
    bool isOwned;
    Scope* enclosingScope;
    Scope* scope;
    Span span;
};

} } // namespace cminor::parsing

#endif // CMINOR_PARSING_PARSING_OBJECT_INCLUDED
