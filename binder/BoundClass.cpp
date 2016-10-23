// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/BoundClass.hpp>
#include <cminor/binder/BoundNodeVisitor.hpp>

namespace cminor { namespace binder {

BoundClass::BoundClass(ClassTypeSymbol* classTypeSymbol_) : BoundNode(*classTypeSymbol_->GetAssembly()), classTypeSymbol(classTypeSymbol_)
{
}

void BoundClass::AddMember(std::unique_ptr<BoundNode>&& member)
{
    members.push_back(std::move(member));
}

void BoundClass::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::binder
