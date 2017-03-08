// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/GlobalFlags.hpp>

namespace cminor { namespace symbols {

GlobalFlags globalFlags;
int optimizationLevel = -1;
int inlineLimit = -1;
std::string debugPassValue;

inline GlobalFlags operator|(GlobalFlags flags, GlobalFlags flag)
{
    return GlobalFlags(uint16_t(flags) | uint16_t(flag));
}

inline GlobalFlags operator&(GlobalFlags flags, GlobalFlags flag)
{
    return GlobalFlags(uint16_t(flags) & uint16_t(flag));
}

inline GlobalFlags operator~(GlobalFlags flags)
{
    return GlobalFlags(~uint16_t(flags));
}

void SetGlobalFlag(GlobalFlags flag)
{
    globalFlags = globalFlags | flag;
}

void ResetGlobalFlag(GlobalFlags flag)
{
    globalFlags = globalFlags & ~flag;
}

bool GetGlobalFlag(GlobalFlags flag)
{
    return (globalFlags & flag) != GlobalFlags::none;
}

std::string GetConfig()
{
    std::string config = "debug";
    if (GetGlobalFlag(GlobalFlags::release))
    {
        config = "release";
    }
    return config;
}

int GetOptimizationLevel()
{
    if (optimizationLevel == -1)
    {
        if (GetGlobalFlag(GlobalFlags::release))
        {
            return 2;
        }
        else
        {
            return 0;
        }
    }
    return optimizationLevel;
}

void SetOptimizationLevel(int level)
{
    optimizationLevel = level;
}

void SetDebugPassValue(const std::string& value)
{
    debugPassValue = value;
}

const std::string& GetDebugPassValue()
{
    return debugPassValue;
}

int GetInlineLimit()
{
    if (inlineLimit == -1)
    {
        switch (GetOptimizationLevel())
        {
            case 0: return 0;
            case 1: return 0;
            case 2: return 8;
            case 3: return 16;
            default: return 0;
        }
    }
    return inlineLimit;
}

void SetInlineLimit(int limit)
{
    inlineLimit = limit;
}

} } // namespace cminor::symbols
