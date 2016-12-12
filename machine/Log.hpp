// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_LOG_INCLUDED
#define CMINOR_MACHINE_LOG_INCLUDED
#include <string>

namespace cminor { namespace machine {

void OpenLog();
void CloseLog();
void LogMessage(const std::string& message);

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_LOG_INCLUDED
