// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <iostream>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/Symbol.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/machine/Machine.hpp>
#include <boost/filesystem.hpp>

using namespace cminor::machine;
using namespace cminor::symbols;

struct InitDone
{
    InitDone()
    {
        InitSymbol();
    }
    ~InitDone()
    {
        DoneSymbol();
    }
};

int main()
{
    try
    {
        InitDone initDone;
        Machine machine;
        std::unique_ptr<Assembly> systemAssembly = CreateSystemAssembly(machine, "debug");
        boost::filesystem::path obp(systemAssembly->FilePath());
        obp.remove_filename();
        boost::filesystem::create_directories(obp);
        SymbolWriter writer(systemAssembly->FilePath());
        systemAssembly->Write(writer);
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
