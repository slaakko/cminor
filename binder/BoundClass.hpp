// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_BOUND_CLASS_INCLUDED
#define CMINOR_BINDER_BOUND_CLASS_INCLUDED
#include <cminor/binder/BoundFunction.hpp>

namespace cminor { namespace binder {

class BoundClass : public BoundNode
{
public:
    BoundClass(ClassTypeSymbol* classTypeSymbol_);
    ClassTypeSymbol* GetClassTypeSymbol() const { return classTypeSymbol; }
    void AddMember(std::unique_ptr<BoundNode>&& member);
    const std::vector<std::unique_ptr<BoundNode>>& Members() const { return members; }
    void Accept(BoundNodeVisitor& visitor) override;
private:
    ClassTypeSymbol* classTypeSymbol;
    std::vector<std::unique_ptr<BoundNode>> members;
};

} } // namespace cminor::binder

#endif // CMINOR_BINDER_BOUND_CLASS_INCLUDED
