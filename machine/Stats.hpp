// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_STATS_INCLUDED
#define CMINOR_MACHINE_STATS_INCLUDED
#include <cminor/machine/MachineApi.hpp>
#include <stdint.h>


namespace cminor { namespace machine {

MACHINE_API void AddLoadTime(int64_t ms);
MACHINE_API void AddLinkTime(int64_t ms);
MACHINE_API void AddPrepareTime(int64_t ms);
MACHINE_API void AddRunTime(int64_t ms);
MACHINE_API void AddGcTime(int64_t ms, bool fullCollection);
MACHINE_API void AddTotalVmTime(int64_t ms);
MACHINE_API void PrintStats();

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_STATS_INCLUDED
