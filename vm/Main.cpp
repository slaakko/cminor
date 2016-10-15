// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <iostream>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Function.hpp>
#include <cminor/machine/FileRegistry.hpp>
#include <cminor/symbols/Symbol.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolReader.hpp>
#include <cminor/symbols/GlobalFlags.hpp>
#include <cminor/ast/Project.hpp>
#include <Cm.Util/Path.hpp>
#include <boost/filesystem.hpp>
#include <stdexcept>

using namespace cminor::machine;
using namespace cminor::symbols;
using namespace cminor::ast;
using namespace Cm::Util;

struct InitDone
{
    InitDone()
    {
        FileRegistry::Init();
        FunctionTable::Init();
        InitSymbol();
        InitAssembly();
    }
    ~InitDone()
    {
        DoneAssembly();
        DoneSymbol();
    }
};

const char* version = "0.0.1";

void PrintHelp()
{
    std::cout << "Cminor virtual machine version " << version << "\n\n" <<
        "Usage: cminorvm [options] program.cminora [arguments]\n" <<
        "Run program.cminora with given arguments.\n" <<
        "Options:\n" <<
        "-h | --help     : print this help message" <<
        "-s=SEGMENT-SIZE :\n" <<
        "       SEGMENT-SIZE is the size of the garbage collected memory\n" <<
        "       segment in megabytes. The default is 250. SEGMENT-SIZE is\n" <<
        "       also the maximum size of single continuous object.\n" <<
        std::endl;
}

enum class State
{
    vmOptions, arguments
};

int main(int argc, const char** argv)
{
    try
    {
        if (argc < 2)
        {
            PrintHelp();
            return 0;
        }
        InitDone initDone;
        State state = State::vmOptions;
        std::string programName;
        std::vector<std::string> arguments;
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            switch (state)
            {
                case State::vmOptions:
                {
                    if (!arg.empty() && arg[0] == '-')
                    {
                        if (arg == "-h" || arg == "--help")
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
                        programName = arg;
                        state = State::arguments;
                    }
                    break;
                }
                case State::arguments:
                {
                    arguments.push_back(arg);
                    break;
                }
            }
        }
        if (programName.empty())
        {
            throw std::runtime_error("no program assembly file given");
        }
        boost::filesystem::path pfp = programName;
        if (pfp.extension() != ".cminora")
        {
            throw std::runtime_error("argument '" + pfp.generic_string() + "' has invalid extension (not .cminora)");
        }
        std::string assemblyFilePath = GetFullPath(pfp.generic_string());
        if (!boost::filesystem::exists(pfp))
        {
            throw std::runtime_error("program assembly file '" + assemblyFilePath + "' not found");
        }
        Machine machine;
        Assembly assembly(machine);
        const Assembly* rootAssembly = &assembly;
        SymbolReader symbolReader(assemblyFilePath);
        std::vector<CallInst*> callInstructions;
        std::string currentAssemblyDir = GetFullPath(boost::filesystem::path(assemblyFilePath).remove_filename().generic_string());
        std::unordered_set<std::string> importSet;
        assembly.Read(symbolReader, LoadType::execute, rootAssembly, currentAssemblyDir, importSet, callInstructions);
        Link(callInstructions);
        machine.Run();
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}