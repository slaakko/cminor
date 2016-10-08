// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/BoundFunction.hpp>

namespace cminor { namespace binder {

void BoundFunction::SetBody(std::unique_ptr<BoundCompoundStatement>&& body_)
{
    body.reset(body_.release());
}

} } // namespace cminor::binder
