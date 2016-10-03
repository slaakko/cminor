// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_GLOBAL_FLAGS_INCLUDED
#define CMINOR_SYMBOLS_GLOBAL_FLAGS_INCLUDED
#include <stdint.h>

namespace cminor { namespace symbols {

enum class GlobalFlags : uint8_t
{
    none = 0,
    verbose = 1 << 0
};

void SetGlobalFlag(GlobalFlags flag);
bool GetGlobalFlag(GlobalFlags flag);

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_GLOBAL_FLAGS_INCLUDED
