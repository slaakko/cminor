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
    quiet = 1 << 1,
    release = 1 << 2,
    debugParsing = 1 << 3,
    clean = 1 << 4,
    list = 1 << 5,
    native = 1 << 6,
    emitLlvm = 1 << 7,
    emitOptLlvm = 1 << 8,
    linkWithDebugMachine = 1 << 9,
    useMsLink = 1 << 10,
    readClassNodes = 1 << 11
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
