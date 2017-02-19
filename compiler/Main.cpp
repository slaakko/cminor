// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/build/Build.hpp>
#include <cminor/symbols/GlobalFlags.hpp>
#include <cminor/symbols/Symbol.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/Value.hpp>
#include <cminor/symbols/Warning.hpp>
#include <cminor/machine/FileRegistry.hpp>
#include <cminor/machine/Function.hpp>
#include <cminor/util/Util.hpp>
#include <cminor/machine/Class.hpp>
#include <cminor/pl/InitDone.hpp>
#include <cminor/util/Path.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace cminor::build;
using namespace cminor::symbols;
using namespace cminor::machine;
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
        InitWarning();
        InitAssembly();
        cminor::parsing::Init();
    }
    ~InitDone()
    {
        cminor::parsing::Done();
        DoneAssembly();
        DoneWarning();
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
        "Cminor compiler version " << version << "\n\n" <<
        "Usage: cminorc [options] { solution.cminors | project.cminorp }\n" <<
        "Compile given solutions and projects.\n" <<
        "Options:\n" <<
        "-c=CONFIG | --config=CONFIG\n" <<
        "   Use CONFIG configuration. CONFIG can be debug or release.\n" <<
        "   The default is debug.\n" <<
        "-h | --help        : print this help message\n" <<
        "-v | --verbose     : verbose output\n" <<
        "-e | --clean       : clean given projects and solutions\n" <<
        "-d | --debug-parse : debug parsing to stdout\n" <<
        "-n | --native    : compile to native object code.\n" <<
        "-O=LEVEL | --optimization-level=LEVEL :\n" <<
        "                              set optimization level to LEVEL=0-3 (used with --native).\n" <<
        "                              Defaults: debug 0, release 2.\n" <<
        "-p=VALUE | --debug-pass=VALUE : Generate debug output for LLVM passes (used with --native).\n" <<
        "                               VALUE can be Arguments, Structure, Executions or Details.\n" <<
        "-l | --list      : generate listing to ASSEMBLY_NAME.list (used with --native)\n" <<
        "-m | --emit-llvm : emit LLVM intermediate code to ASSEMBLY_NAME.ll (used with --native)\n" <<
        "-t | --emit-opt-llvm : emit optimized LLVM intermediate code to ASSEMBLY_NAME.opt.ll (used with --native)\n" <<
        "-a | --emit-asm  : emit assembly code listing to ASSEMBLY_NAME.asm or ASSEMBLY_NAME.s (used with --native)\n" <<
        std::endl;
}

int main(int argc, const char** argv)
{
    try
    {
        InitDone initDone;        
        std::vector<std::string> projectsAndSolutions;
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
                else if (arg == "-d" || arg == "--debug-parse")
                {
                    SetGlobalFlag(GlobalFlags::debugParsing);
                }
                else if (arg == "-e" || arg == "--clean")
                {
                    SetGlobalFlag(GlobalFlags::clean);
                }
                else if (arg == "-l" || arg == "--list")
                {
                    SetGlobalFlag(GlobalFlags::list);
                }
                else if (arg == "-n" || arg == "--native")
                {
                    SetGlobalFlag(GlobalFlags::native);
                }
                else if (arg == "-m" || arg == "--emit-llvm")
                {
                    SetGlobalFlag(GlobalFlags::emitLlvm);
                }
                else if (arg == "-t" || arg == "--emit-opt-llvm")
                {
                    SetGlobalFlag(GlobalFlags::emitOptLlvm);
                }
                else if (arg == "-a" || arg == "--emit-asm")
                {
                    SetGlobalFlag(GlobalFlags::emitAsm);
                }
                else if (arg.find('=', 0) != std::string::npos)
                {
                    std::vector<std::string> components = Split(arg, '=');
                    if (components.size() != 2)
                    {
                        throw std::runtime_error("invalid argument '" + arg + "'");
                    }
                    else
                    {
                        if (components[0] == "-c" || components[0] == "--config")
                        {
                            if (components[1] == "release")
                            {
                                SetGlobalFlag(GlobalFlags::release);
                            }
                            else if (components[1] != "debug")
                            {
                                throw std::runtime_error("invalid configuration argument '" + arg + "'");
                            }
                        }
                        else if (components[0] == "-O" || components[0] == "--optimization-level")
                        {
                            std::string levelStr = components[1];
                            if (levelStr != "0" && levelStr != "1" && levelStr != "2" && levelStr != "3")
                            {
                                throw std::runtime_error("unknown optimization level " + levelStr);
                            }
                            else
                            {
                                int level = boost::lexical_cast<int>(levelStr);
                                SetOptimizationLevel(level);
                            }
                        }
                        else if (components[0] == "-p" || components[0] == "--debug-pass")
                        {
                            const std::string& value = components[1];
                            if (value != "Arguments" &&  value != "Structure" && value != "Executions" && value != "Details")
                            {
                                throw std::runtime_error("unknown LLVM debug pass value " + value);
                            }
                            else
                            {
                                SetDebugPassValue(value);
                            }
                        }
                        else
                        {
                            throw std::runtime_error("unknown argument '" + arg + "'");
                        }
                    }
                }
                else
                {
                    throw std::runtime_error("unknown argument '" + arg + "'");
                }
            }
            else
            {
                projectsAndSolutions.push_back(arg);
            }
        }
        if (GetGlobalFlag(GlobalFlags::list))
        {
            if (!GetGlobalFlag(GlobalFlags::native))
            {
                throw std::runtime_error("--list option requires --native option");
            }
        }
        if (GetGlobalFlag(GlobalFlags::emitLlvm))
        {
            if (!GetGlobalFlag(GlobalFlags::native))
            {
                throw std::runtime_error("--emit-llvm option requires --native option");
            }
        }
        if (GetGlobalFlag(GlobalFlags::emitOptLlvm))
        {
            if (!GetGlobalFlag(GlobalFlags::native))
            {
                throw std::runtime_error("--emit-opt-llvm option requires --native option");
            }
        }
        if (GetGlobalFlag(GlobalFlags::emitAsm))
        {
            if (!GetGlobalFlag(GlobalFlags::native))
            {
                throw std::runtime_error("--emit-asm option requires --native option");
            }
        }
        if (projectsAndSolutions.empty())
        {
            PrintHelp();
            return 0;
        }
        if (GetGlobalFlag(GlobalFlags::verbose))
        {
            std::cout << "Cminor compiler version " << version << std::endl;
        }
        for (const std::string& projectOrSolution : projectsAndSolutions)
        {
            boost::filesystem::path fp(projectOrSolution);
            if (fp.extension() == ".cminors")
            {
                if (!boost::filesystem::exists(fp))
                {
                    throw std::runtime_error("solution file '" + fp.generic_string() + " not found");
                }
                else
                {
                    BuildSolution(GetFullPath(fp.generic_string()));
                }
            }
            else if (fp.extension() == ".cminorp")
            {
                if (!boost::filesystem::exists(fp))
                {
                    throw std::runtime_error("project file '" + fp.generic_string() + " not found");
                }
                else
                {
                    std::set<AssemblyReferenceInfo> assemblyReferenceInfos;
                    BuildProject(GetFullPath(fp.generic_string()), assemblyReferenceInfos);
                }
            }
            else
            {
                throw std::runtime_error("Argument '" + fp.generic_string() + "' has invalid extension. Not Cminor solution (.cminors) or project (.cminorp) file.");
            }
        }
    }
    catch (const Exception& ex)
    {
        std::cerr << ex.What() << std::endl;
        return 1;
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
