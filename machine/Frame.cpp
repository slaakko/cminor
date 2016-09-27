// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Frame.hpp>

namespace cminor { namespace machine {

Frame::Frame(ObjectPool& objectPool_, int32_t numLocals) : locals(numLocals), objectPool(objectPool_), pc(0)
{
}

} } // namespace cminor::machine
