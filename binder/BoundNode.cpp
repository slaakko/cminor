// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/BoundNode.hpp>

namespace cminor { namespace binder {

BoundNode::BoundNode(Assembly& assembly_) : assembly(assembly_)
{
}

BoundNode::~BoundNode()
{
}

} } // namespace cminor::binder
