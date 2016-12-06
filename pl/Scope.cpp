/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#include <cminor/pl/Scope.hpp>
#include <cminor/pl/Namespace.hpp>
#include <cminor/pl/Utility.hpp>
#include <cminor/machine/TextUtils.hpp>
#include <stdexcept>

namespace cminor { namespace parsing {

Scope::Scope(const std::string& name_, Scope* enclosingScope_): ParsingObject(name_, enclosingScope_), fullNameComputed(false)
{
}

std::string Scope::FullName() const
{
    if (!fullNameComputed)
    {
        Scope* parentScope = EnclosingScope();
        std::string parentScopeFullName = parentScope ? parentScope->FullName() : "";
        fullName = parentScopeFullName.empty() ? Name() : parentScopeFullName + "." + Name();
        fullNameComputed = true;
    }
    return fullName;
}

void Scope::Accept(Visitor& visitor)
{
    FullNameMapIt e = fullNameMap.end();
    for (FullNameMapIt i = fullNameMap.begin(); i != e; ++i)
    {
        ParsingObject* object = i->second;
        object->Accept(visitor);
    }
}

void Scope::Add(ParsingObject* object)
{
    Own(object);
    std::string objectFullName = object->FullName();
    if (!fullNameMap.insert(std::make_pair(objectFullName, object)).second)
    {
        std::string msg = "object '" + objectFullName + "' already exists (detected in scope '" + FullName() + "')";
        throw std::runtime_error(msg);
    }
    if (!shortNameMap.insert(std::make_pair(object->Name(), object)).second)
    {
        std::string msg = "object '" + objectFullName + "' already exists (detected in scope '" + FullName() + "')";
        throw std::runtime_error(msg);
    }
}

Scope* Scope::GetGlobalScope() const
{
    Scope* globalScope = const_cast<Scope*>(this);
    while (globalScope->EnclosingScope())
    {
        globalScope = globalScope->EnclosingScope();
    }
    return globalScope;
}

void Scope::AddNamespace(Namespace* nsToAdd)
{
    Own(nsToAdd);
    Namespace* parent = GetGlobalScope()->Ns();
    std::vector<std::string> nameComponents = cminor::machine::Split(nsToAdd->FullName(), '.');
    if (nameComponents.empty())
    {
        throw std::runtime_error("namespace components empty");
    }
    int n = int(nameComponents.size());
    for (int i = 0; i < n - 1; ++i)
    {
        const std::string& namespaceName = nameComponents[i];
        Scope* parentScope = parent->GetScope();
        ParsingObject* object = parentScope->Get(namespaceName);
        if (object)
        {
            if (object->IsNamespace())
            {
                parent = static_cast<Namespace*>(object);
            }
            else
            {
                throw std::runtime_error("object '" + namespaceName + "' does not denote a namespace");
            }
        }
        else
        {
            parent = new Namespace(namespaceName, parentScope);
            Own(parent);
            parent->GetScope()->SetName(namespaceName);
            parent->GetScope()->SetNs(parent);
            parentScope->Add(parent);
        }
    }
    std::string commonName = nameComponents[n - 1];
    nsToAdd->SetName(commonName);
    nsToAdd->GetScope()->SetName(commonName);
    nsToAdd->SetEnclosingScope(parent->GetScope());
    nsToAdd->GetScope()->SetEnclosingScope(parent->GetScope());
    parent->GetScope()->Add(nsToAdd);
}

ParsingObject* Scope::GetQualifiedObject(const std::string& qualifiedObjectName) const
{
    std::vector<std::string> components = cminor::machine::Split(qualifiedObjectName, '.');
    int n = int(components.size());
    Scope* s = const_cast<Scope*>(this);
    while (s)
    {
        Scope* subScope = s;
        int i = 0;
        ShortNameMapIt it = subScope->shortNameMap.find(components[i]);
        while (it != subScope->shortNameMap.end())
        {
            ParsingObject* object = it->second;
            if (i == n - 1)
            {
                return object;
            }
            ++i;
            subScope = object->GetScope();
            it = subScope->shortNameMap.find(components[i]);
        }
        s = s->EnclosingScope();
    }
    return nullptr;
}

ParsingObject* Scope::Get(const std::string& objectName) const
{
    if (objectName.find('.') != std::string::npos)
    {
        FullNameMapIt i = fullNameMap.find(objectName);
        if (i != fullNameMap.end())
        {
            return i->second;
        }
        else
        {
            ParsingObject* object = GetQualifiedObject(objectName);
            if (object)
            {
                return object;
            }
        }
    }
    else
    {
        ShortNameMapIt i = shortNameMap.find(objectName);
        if (i != shortNameMap.end())
        {
            return i->second;
        }
    }
    return nullptr;
}

Namespace* Scope::GetNamespace(const std::string& fullNamespaceName) const
{
    ParsingObject* object = GetQualifiedObject(fullNamespaceName);
    if (object)
    {
        if (object->IsNamespace())
        {
            return static_cast<Namespace*>(object);
        }
        else
        {
            throw std::runtime_error("object '" + fullNamespaceName + "' is not a namespace");
        }
    }
    return nullptr;
}

} } // namespace Cm::Parsing
