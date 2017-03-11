// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Stats.hpp>
#include <string>
#include <iostream>
#include <iomanip>

namespace cminor { namespace machine {

int numberOfGen1GcPauses = 0;
int numberOfFullGcPauses = 0;
int numberOfGcPauses = 0;
int64_t loadTimeMs = 0;
int64_t linkTimeMs = 0;
int64_t prepareTimeMs = 0;
int64_t startupTimeMs = 0;
int64_t gen1GcTimeMs = 0;
int64_t fullGcTimeMs = 0;
int64_t gcTimeMs = 0;
int64_t runTimeMs = 0;
int64_t totalVmTimeMs = 0;

MACHINE_API void AddLoadTime(int64_t ms)
{
    loadTimeMs += ms;
    startupTimeMs += ms;
}

MACHINE_API void AddLinkTime(int64_t ms)
{
    linkTimeMs += ms;
    startupTimeMs += ms;
}

MACHINE_API void AddPrepareTime(int64_t ms)
{
    prepareTimeMs += ms;
    startupTimeMs += ms;
}

MACHINE_API void AddRunTime(int64_t ms)
{
    runTimeMs += ms;
}

MACHINE_API void AddGcTime(int64_t ms, bool fullCollection)
{
    if (fullCollection)
    {
        fullGcTimeMs += ms;
        ++numberOfFullGcPauses;
    }
    else
    {
        gen1GcTimeMs += ms;
        ++numberOfGen1GcPauses;
    }
    gcTimeMs += ms;
    ++numberOfGcPauses;
}

MACHINE_API void AddTotalVmTime(int64_t ms)
{
    totalVmTimeMs += ms;
}

std::string Percent(int64_t time, int64_t total)
{
    if (total == 0)
    {
        return "0.0 %";
    }
    double m = time;
    double t = total;
    double percent = 100.0 * m / t;
    int p = static_cast<int>(percent);
    double decimals = percent - p;
    int d = static_cast<int>(10.0 * decimals);
    return std::to_string(p) + "." + std::to_string(d) + " %";
}

MACHINE_API void PrintStats()
{
    std::cout << "\nSTATISTICS\n\n" <<
        "    load time : " << std::setw(5) << loadTimeMs << " ms (" << std::setw(7) << Percent(loadTimeMs, startupTimeMs) << " of startup time)\n" <<
        "    link time : " << std::setw(5) << linkTimeMs << " ms (" << std::setw(7) << Percent(linkTimeMs, startupTimeMs) << " of startup time)\n" <<
        " prepare time : " << std::setw(5) << prepareTimeMs << " ms (" << std::setw(7) << Percent(prepareTimeMs, startupTimeMs) << " of startup time)\n" <<
        " startup time : " << std::setw(5) << startupTimeMs << " ms (" << std::setw(7) << Percent(startupTimeMs, totalVmTimeMs) << " of total vm time)\n" <<
        "-------------------------------------------------------------------------------\n" <<
        " gen1 gc time : " << std::setw(5) << gen1GcTimeMs << " ms (" << std::setw(7) << Percent(gen1GcTimeMs, gcTimeMs) << " of total gc time) [" <<
            std::setw(3) << numberOfGen1GcPauses << " gc pauses]\n" <<
        " full gc time : " << std::setw(5) << fullGcTimeMs << " ms (" << std::setw(7) << Percent(fullGcTimeMs, gcTimeMs) << " of total gc time) [" <<
            std::setw(3) << numberOfFullGcPauses << " gc pauses]\n" <<
        "total gc time : " << std::setw(5) << gcTimeMs << " ms (" << std::setw(7) << Percent(gcTimeMs, runTimeMs) << " of run time) [" << 
            std::setw(3) << numberOfGcPauses << " gc pauses]\n" <<
        "     run time : " << std::setw(5) << runTimeMs << " ms (" << std::setw(7) << Percent(runTimeMs, totalVmTimeMs) << " of total vm time)\n" <<
        "-------------------------------------------------------------------------------\n" <<
        "total vm time : " << std::setw(5) << totalVmTimeMs << " ms (" << std::setw(7) << Percent(totalVmTimeMs, totalVmTimeMs) << " of startup time + run time + extra time)\n" <<
        std::endl;
}

} } // namespace cminor::machine
