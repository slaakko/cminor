// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/parsing/Scope.hpp>
#include <cminor/parsing/Namespace.hpp>
#include <cminor/parsing/Utility.hpp>
#include <cminor/util/Util.hpp>
#include <cminor/util/Unicode.hpp>
#include <stdexcept>

namespace cminor { namespace parsing {

using namespace cminor::util;
using namespace cminor::unicode;

Scope::Scope(const std::u32string& name_, Scope* enclosingScope_): ParsingObject(name_, enclosingScope_), fullNameComputed(false)
{
}

std::u32string Scope::FullName() const
{
    if (!fullNameComputed)
    {
        Scope* parentScope = EnclosingScope();
        std::u32string parentScopeFullName = parentScope ? parentScope->FullName() : U"";
        fullName = parentScopeFullName.empty() ? Name() : parentScopeFullName + U"." + Name();
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
    std::u32string objectFullName = object->FullName();
    if (!fullNameMap.insert(std::make_pair(objectFullName, object)).second)
    {
        std::u32string msg = U"object '" + objectFullName + U"' already exists (detected in scope '" + FullName() + U"')";
        throw std::runtime_error(ToUtf8(msg));
    }
    if (!shortNameMap.insert(std::make_pair(object->Name(), object)).second)
    {
        std::u32string msg = U"object '" + objectFullName + U"' already exists (detected in scope '" + FullName() + U"')";
        throw std::runtime_error(ToUtf8(msg));
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
    std::vector<std::u32string> nameComponents = cminor::util::Split(nsToAdd->FullName(), '.');
    if (nameComponents.empty())
    {
        throw std::runtime_error("namespace components empty");
    }
    int n = int(nameComponents.size());
    for (int i = 0; i < n - 1; ++i)
    {
        const std::u32string& namespaceName = nameComponents[i];
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
                throw std::runtime_error("object '" + ToUtf8(namespaceName) + "' does not denote a namespace");
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
    std::u32string commonName = nameComponents[n - 1];
    nsToAdd->SetName(commonName);
    nsToAdd->GetScope()->SetName(commonName);
    nsToAdd->SetEnclosingScope(parent->GetScope());
    nsToAdd->GetScope()->SetEnclosingScope(parent->GetScope());
    parent->GetScope()->Add(nsToAdd);
}

ParsingObject* Scope::GetQualifiedObject(const std::u32string& qualifiedObjectName) const
{
    std::vector<std::u32string> components = cminor::util::Split(qualifiedObjectName, '.');
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

ParsingObject* Scope::Get(const std::u32string& objectName) const
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

Namespace* Scope::GetNamespace(const std::u32string& fullNamespaceName) const
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
            throw std::runtime_error("object '" + ToUtf8(fullNamespaceName) + "' is not a namespace");
        }
    }
    return nullptr;
}

} } // namespace cminor::parsing
