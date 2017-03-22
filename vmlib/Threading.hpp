// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_VMLIB_THREADING_INCLUDED
#define CMINOR_VMLIB_THREADING_INCLUDED
#include <cminor/machine/Object.hpp>
#include <chrono>

namespace cminor { namespace vmlib {

using namespace cminor::machine;

void EnterMonitor(ManagedAllocationHeader* header);
void ExitMonitor(ManagedAllocationHeader* header);

int32_t CreateConditionVariable();
void DestroyConditionVariable(int32_t conditionVariableId);

void NotifyOne(int32_t conditionVariableId);
void NotifyAll(int32_t conditionVariableId);

void WaitConditionVariable(int32_t conditionVariableId, int32_t lockId);

enum class CondVarStatus : uint8_t
{
    timeout, no_timeout
};

CondVarStatus WaitConditionVariable(int32_t conditionVariableId, int32_t lockId, std::chrono::nanoseconds duration);

void ThreadingInit();
void ThreadingDone();

} } // namespace cminor::vmlib

#endif // CMINOR_VMLIB_THREADING_INCLUDED
