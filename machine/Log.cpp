// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Log.hpp>
#include <fstream>
#include <chrono>
#include <mutex>

namespace cminor { namespace machine {

std::ofstream log;
const char* logFilePath = "C:\\Temp\\machine.log";
auto start = std::chrono::steady_clock::now();
bool logOpen = false;
std::mutex logMutex;

MACHINE_API void OpenLog()
{
    log.open(logFilePath);
    logOpen = true;
    log.close();
}

MACHINE_API void CloseLog()
{
}

MACHINE_API void LogMessage(const std::string& message)
{
    if (logOpen)
    {
        std::lock_guard<std::mutex> lock(logMutex);
        log.open(logFilePath, std::ios_base::app);
        auto timeStamp = std::chrono::steady_clock::now();
        auto dur = timeStamp - start;
        long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
        log << ms << " : " << message << std::endl;
        log.close();
    }
}

} } // namespace cminor::machine
