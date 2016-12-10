// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_GLOBAL_FLAGS_INCLUDED
#define CMINOR_SYMBOLS_GLOBAL_FLAGS_INCLUDED
#include <stdint.h>
#include <string>

namespace cminor { namespace symbols {

enum class GlobalFlags : uint8_t
{
    none = 0,
    verbose = 1 << 0,
    release = 1 << 1,
    debugParsing = 1 << 2,
    clean = 1 << 3
};

void SetGlobalFlag(GlobalFlags flag);
bool GetGlobalFlag(GlobalFlags flag);

std::string GetConfig();

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_GLOBAL_FLAGS_INCLUDED
