// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/pl/ParsingObject.hpp>
#include <cminor/pl/Scope.hpp>
#include <cminor/pl/Exception.hpp>

namespace cminor { namespace parsing {

ParsingObject::ParsingObject(const std::string& name_): name(name_), owner(external), isOwned(false), enclosingScope(nullptr), scope(nullptr)
{
}

ParsingObject::ParsingObject(const std::string& name_, Scope* enclosingScope_): name(name_), owner(external), isOwned(false), enclosingScope(enclosingScope_), scope(nullptr)
{
}

ParsingObject::~ParsingObject()
{
}

void SetEnclosingScope(Scope* enclosingScope_)
{
}


std::string ParsingObject::FullName() const
{
    std::string scopeName = "";
    if (enclosingScope)
    {
        scopeName = enclosingScope->FullName();
    }
    if (scopeName.empty())
    {
        return name;
    }
    return scopeName + "." + name;
}

void ParsingObject::Own(ParsingObject* object)
{
    if (object)
    {
        if (!object->IsOwned())
        {
            object->SetOwned();
            ownedObjects.push_back(std::unique_ptr<ParsingObject>(object));
        }
    }
}

void ParsingObject::SetScope(Scope* scope_)
{
    scope = scope_;
    Own(scope);
}

void ParsingObject::AddToScope()
{
    if (enclosingScope)
    {
        try
        {
            enclosingScope->Add(this);
        }
        catch (std::exception& ex)
        {
            ThrowException(ex.what(), span);
        }
        if (scope)
        {
            scope->SetEnclosingScope(enclosingScope);
        }
    }

}

} } // namespace cminor::parsing

