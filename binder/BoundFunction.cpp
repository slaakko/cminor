// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/BoundFunction.hpp>
#include <cminor/binder/BoundNodeVisitor.hpp>
#include <cminor/symbols/FunctionSymbol.hpp>

namespace cminor { namespace binder {

BoundFunction::BoundFunction(FunctionSymbol* functionSymbol_) : BoundNode(*functionSymbol_->GetAssembly()), functionSymbol(functionSymbol_)
{
}

void BoundFunction::SetBody(std::unique_ptr<BoundCompoundStatement>&& body_)
{
    body.reset(body_.release());
}

void BoundFunction::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::binder
