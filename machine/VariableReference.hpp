// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_VARIABLE_REFERENCE_INCLUDED
#define CMINOR_MACHINE_VARIABLE_REFERENCE_INCLUDED
#include <stdint.h>

namespace cminor { namespace machine {

class VariableReference
{
public:
    VariableReference(int32_t id_, int32_t frameId_, int32_t localIndex_);
    int32_t Id() const { return id; }
    int32_t FrameId() const { return frameId; }
    int32_t LocalIndex() const { return localIndex; }
private:
    int32_t id;
    int32_t frameId;
    int32_t localIndex;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_VARIABLE_REFERENCE_INCLUDED
