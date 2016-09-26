// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Frame.hpp>

namespace cminor { namespace machine {

Frame::Frame(int32_t numLocals) : locals(numLocals), pc(0)
{
}

} } // namespace cminor::machine
