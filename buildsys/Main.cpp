// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <iostream>
#include <cminor/build/Build.hpp>
#include <cminor/ast/Project.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/Symbol.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/FileRegistry.hpp>
#include <Cm.Parsing/InitDone.hpp>
#include <Cm.Util/Path.hpp>
#include <boost/filesystem.hpp>

using namespace Cm::Util;
using namespace cminor::machine;
using namespace cminor::symbols;
using namespace cminor::build;

struct InitDone
{
    InitDone()
    {
        FileRegistry::Init();
        FunctionTable::Init();
        InitSymbol();
        InitAssembly();
        Cm::Parsing::Init();
    }
    ~InitDone()
    {
        Cm::Parsing::Done();
        DoneAssembly();
        DoneSymbol();
    }
};

int main()
{
    try
    {
        InitDone initDone;
        Machine machine;
        std::unique_ptr<Assembly> systemCoreAssembly = CreateSystemCoreAssembly(machine, "debug");
        boost::filesystem::path obp(systemCoreAssembly->FilePath());
        obp.remove_filename();
        boost::filesystem::create_directories(obp);
        {
            SymbolWriter writer(systemCoreAssembly->FilePath());
            systemCoreAssembly->Write(writer);
        }
        boost::filesystem::path ssfp = CminorSystemDir();
        ssfp /= "System.cminors";
        std::string systemSolutionFilePath = GetFullPath(ssfp.generic_string());
        BuildSolution(systemSolutionFilePath);
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
