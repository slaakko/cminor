// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_LOG_INCLUDED
#define CMINOR_MACHINE_LOG_INCLUDED
#include <string>
#include <cminor/machine/MachineApi.hpp>

namespace cminor { namespace machine {

MACHINE_API void OpenLog();
MACHINE_API void CloseLog();
MACHINE_API void LogMessage(const std::string& message);

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_LOG_INCLUDED
