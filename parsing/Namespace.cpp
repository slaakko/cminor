// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/parsing/Namespace.hpp>
#include <cminor/parsing/Scope.hpp>
#include <cminor/parsing/Visitor.hpp>

namespace cminor { namespace parsing {

Namespace::Namespace(const std::u32string& name_, Scope* enclosingScope_): ParsingObject(name_, enclosingScope_)
{
    SetScope(new Scope(Name(), EnclosingScope())); 
}

void Namespace::Accept(Visitor& visitor)
{
    visitor.BeginVisit(*this);
    GetScope()->Accept(visitor);
    visitor.EndVisit(*this);
}

UsingObject::UsingObject(cminor::codedom::UsingObject* subject_, Scope* enclosingScope_): ParsingObject(subject_->Name(), enclosingScope_), subject(subject_)
{
    if (!subject->IsOwned())
    {
        subject->SetOwned();
        ownedSubject.reset(subject);
    }
}

void UsingObject::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::parsing
