// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <iostream>
#include <cminor/build/Build.hpp>
#include <cminor/emitter/Emitter.hpp>
#include <cminor/binder/BoundCompileUnit.hpp>
#include <cminor/binder/BoundClass.hpp>
#include <cminor/binder/BoundFunction.hpp>
#include <cminor/binder/TypeBinderVisitor.hpp>
#include <cminor/symbols/GlobalFlags.hpp>
#include <cminor/symbols/FunctionSymbol.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/Symbol.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/Value.hpp>
#include <cminor/symbols/Warning.hpp>
#include <cminor/ast/Project.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/FileRegistry.hpp>
#include <cminor/machine/Class.hpp>
#include <cminor/pl/InitDone.hpp>
#include <cminor/machine/Path.hpp>
#include <boost/filesystem.hpp>

using namespace cminor::build;
using namespace cminor::emitter;
using namespace cminor::binder;
using namespace cminor::symbols;
using namespace cminor::machine;

struct InitDone
{
    InitDone()
    {
        FileRegistry::Init();
        FunctionTable::Init();
        ClassDataTable::Init();
        TypeInit();
        NodeInit();
        InitSymbol();
        ValueInit();
        InitAssembly();
        InitWarning();
        cminor::parsing::Init();
    }
    ~InitDone()
    {
        cminor::parsing::Done();
        DoneWarning();
        DoneAssembly();
        ValueDone();
        DoneSymbol();
        NodeDone();
        TypeDone();
        ClassDataTable::Done();
        FunctionTable::Done();
    }
};

const char* version = "0.0.1";

void PrintHelp()
{
    std::cout <<
        "Cminor system library builder version " << version << "\n\n" <<
        "Usage: cminorbuildsys [options]\n" <<
        "Compile system library solution.\n" <<
        "Options:\n" <<
        "-v | --verbose : verbose output\n" <<
        "-h | --help    : print this help message\n" <<
        std::endl;
}

int main(int argc, const char** argv)
{
    try
    {
        InitDone initDone;
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            if (!arg.empty() && arg[0] == '-')
            {
                if (arg == "-v" || arg == "--verbose")
                {
                    SetGlobalFlag(GlobalFlags::verbose);
                }
                else if (arg == "-h" || arg == "--help")
                {
                    PrintHelp();
                    return 0;
                }
                else
                {
                    throw std::runtime_error("unknown argument '" + arg + "'");
                }
            }
            else
            {
                throw std::runtime_error("unknown argument '" + arg + "'");
            }
        }
        Machine machine;
        {
            std::unique_ptr<Assembly> systemCoreAssembly = CreateSystemCoreAssembly(machine, "debug");
            boost::filesystem::path obp(systemCoreAssembly->FilePath());
            obp.remove_filename();
            boost::filesystem::create_directories(obp);
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
