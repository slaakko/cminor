// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/GlobalFlags.hpp>

namespace cminor { namespace symbols {

GlobalFlags globalFlags;

inline GlobalFlags operator|(GlobalFlags flags, GlobalFlags flag)
{
    return GlobalFlags(uint8_t(flags) | uint8_t(flag));
}

inline GlobalFlags operator&(GlobalFlags flags, GlobalFlags flag)
{
    return GlobalFlags(uint8_t(flags) & uint8_t(flag));
}

void SetGlobalFlag(GlobalFlags flag)
{
    globalFlags = globalFlags | flag;
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

} } // namespace cminor::symbols
