// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/BoundFunction.hpp>
#include <cminor/binder/BoundNodeVisitor.hpp>
#include <cminor/symbols/FunctionSymbol.hpp>

namespace cminor { namespace binder {

BoundFunction::BoundFunction(FunctionSymbol* functionSymbol_) : BoundNode(*functionSymbol_->GetAssembly()), functionSymbol(functionSymbol_), nextTempVarNumber(0), hasGotos(false)
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

std::string BoundFunction::NextTempVarName()
{
    return "@" + std::to_string(nextTempVarNumber++);
}

} } // namespace cminor::binder
