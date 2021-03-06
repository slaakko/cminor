// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_BOUND_FUNCTION_INCLUDED
#define CMINOR_BINDER_BOUND_FUNCTION_INCLUDED
#include <cminor/binder/BoundStatement.hpp>
#include <memory>

namespace cminor { namespace binder {

class BoundFunction : public BoundNode
{
public:
    BoundFunction(FunctionSymbol* functionSymbol_);
    FunctionSymbol* GetFunctionSymbol() const { return functionSymbol; }
    void SetBody(std::unique_ptr<BoundCompoundStatement>&& body_);
    BoundCompoundStatement* Body() const { return body.get(); }
    void Accept(BoundNodeVisitor& visitor) override;
    std::string NextTempVarName();
    void SetHasGotos() { hasGotos = true; }
    bool HasGotos() const { return hasGotos; }
    void SetContainerScope(ContainerScope* containerScope_) { containerScope = containerScope_; }
    ContainerScope* GetContainerScope() const { return containerScope; }
private:
    FunctionSymbol* functionSymbol;
    std::unique_ptr<BoundCompoundStatement> body;
    int nextTempVarNumber;
    bool hasGotos;
    ContainerScope* containerScope;
};

} } // namespace cminor::binder

#endif // CMINOR_BINDER_BOUND_FUNCTION_INCLUDED
