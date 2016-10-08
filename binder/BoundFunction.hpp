// =================================
// Copyright (c) 2016 Seppo Laakko
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
    void SetBody(std::unique_ptr<BoundCompoundStatement>&& body_);
private:
    std::unique_ptr<BoundCompoundStatement> body;
};

} } // namespace cminor::binder

#endif // CMINOR_BINDER_BOUND_FUNCTION_INCLUDED
