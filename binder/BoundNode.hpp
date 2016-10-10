// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_BOUND_NODE_INCLUDED
#define CMINOR_BINDER_BOUND_NODE_INCLUDED
#include <cminor/symbols/Assembly.hpp>

namespace cminor { namespace binder {

using namespace cminor::symbols;

class BoundNodeVisitor;

class BoundNode
{
public:
    BoundNode(Assembly& assembly_);
    virtual ~BoundNode();
    Assembly& GetAssembly() { return assembly; }
    virtual void Accept(BoundNodeVisitor& visitor) = 0;
private:
    Assembly& assembly;
};

} } // namespace cminor::binder

#endif // CMINOR_BINDER_BOUND_NODE_INCLUDED
