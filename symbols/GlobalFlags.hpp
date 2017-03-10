// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_GLOBAL_FLAGS_INCLUDED
#define CMINOR_SYMBOLS_GLOBAL_FLAGS_INCLUDED
#include <stdint.h>
#include <string>

namespace cminor { namespace symbols {

enum class GlobalFlags : uint16_t
{
    none = 0,
    verbose = 1 << 0,
    release = 1 << 1,
    debugParsing = 1 << 2,
    clean = 1 << 3,
    list = 1 << 4,
    native = 1 << 5,
    emitLlvm = 1 << 6,
    emitOptLlvm = 1 << 7,
    linkWithDebugMachine = 1 << 8,
    useMsLink = 1 << 9,
    readClassNodes = 1 << 10
};

void SetGlobalFlag(GlobalFlags flag);
void ResetGlobalFlag(GlobalFlags flag);
bool GetGlobalFlag(GlobalFlags flag);

std::string GetConfig();

int GetOptimizationLevel();
void SetOptimizationLevel(int level);

int GetInlineLimit();
void SetInlineLimit(int limit);

int GetInlineLocals();
void SetInlineLocals(int numLocals);

const std::string& GetDebugPassValue();
void SetDebugPassValue(const std::string& value);

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_GLOBAL_FLAGS_INCLUDED
