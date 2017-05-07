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
#include <cminor/util/System.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <stdexcept>
#include <vector>
#ifdef _WIN32
#pragma warning(disable:4267)
#pragma warning(disable:4244)
#pragma warning(disable:4141)
#pragma warning(disable:4624)
#pragma warning(disable:4291)
#endif
#include <llvm/Support/CommandLine.h>
#ifdef _WIN32
#pragma warning(default:4267)
#pragma warning(default:4244)
#pragma warning(default:4141)
#pragma warning(default:4624)
#pragma warning(default:4291)
#endif

using namespace cminor::build;
using namespace cminor::symbols;
using namespace cminor::machine;
using namespace cminor::util;
using namespace llvm;

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
        cminor::util::unicode::Init();
    }
    ~InitDone()
    {
        cminor::util::unicode::Done();
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

const char* version = "0.2.0";

void PrintHelp()
{
    std::cout << "\n" <<
        "Cminor compiler version " << version << "\n\n" <<
        "Usage: cminorc [options] { SOLUTION.cminors | PROJECT.cminorp }\n\n" <<
        "Compile given solutions and projects.\n\n" <<
        "options:\n\n" <<
        "   --help (-h)\n" <<
        "       Print this help message.\n" <<
        "   --verbose (-v)\n" <<
        "       Verbose output.\n" <<
        "   --quiet (-q)\n" <<
        "       Generate no error output, just return exit code.\n" <<
        "   --config=CONFIG (-c=CONFIG)\n" <<
        "       Use CONFIG configuration. CONFIG can be debug or release. Default is debug.\n" <<
        "   --clean (-e)\n" <<
        "       Clean given projects and solutions.\n" <<
        "   --debug-parse (-p)\n" <<
        "       Debug parsing to stdout.\n" <<
        "   --native (-n)\n" <<
        "       Compile to native object code. Generates DLLs in Windows and shared objects in Linux.\n" <<
        "   --optimization-level=LEVEL (-O=LEVEL)\n" <<
        "       Set optimization level to LEVEL=0-3 (used with --native).\n" <<
        "       Defaults:\n" <<
        "           0 for debug configuration\n" <<
        "           2 for release configuration\n" <<
        "   --inline-limit=LIMIT (-i=LIMIT)\n" <<
        "       Set inline limit to LIMIT intermediate instructions (used with --native).\n" <<
        "       Functions that have LIMIT or fewer intermediate instructions are chosen to be inline candidates.\n" <<
        "       Eventually LLVM will decide which functions to inline.\n" <<
        "       Defaults:\n" <<
        "           for optimization level 0: 0 (no inlining)\n" <<
        "           for optimization level 1: 0 (no inlining)\n" <<
        "           for optimization level 2: 16 instructions\n" <<
        "           for optimization level 3: 32 instructions\n" <<
        "   --inline-locals=NUMLOCALS (-a=NUMLOCALS)\n" <<
        "       Set maximum number of locals for inline function (used with --native).\n" <<
        "       Defaults:\n" <<
        "           for optimization level 0: 0 (no inlining)\n" <<
        "           for optimization level 1: 0 (no inlining)\n" <<
        "           for optimization level 2: 4 locals\n" <<
        "           for optimization level 3: 8 locals\n" <<
        "   --debug-pass=VALUE (-s=VALUE)\n" <<
        "       Generate debug output for LLVM passes to stderr (used with --native).\n" <<
        "       VALUE can be Arguments, Structure, Executions or Details:\n" <<
        "           Arguments: print pass arguments to pass to 'opt'\n" <<
        "           Structure: print pass structure before run()\n" <<
        "           Executions: print pass name before it is executed\n" <<
        "           Details: print pass details when it is executed\n" <<
        "   --debug-llvm (-b)\n" <<
        "       Generate LLVM debug output to stderr (used with --native).\n" <<
        "   --list (-l)\n" <<
        "       Generate listing to ASSEMBLY_NAME.list (used with --native)\n" <<
        "   --emit-llvm (-m)\n" <<
        "       Emit LLVM intermediate code to ASSEMBLY_NAME.ll (used with --native)\n" <<
        "   --emit-opt-llvm (-t)\n" <<
        "       Emit optimized LLVM intermediate code to ASSEMBLY_NAME.opt.ll (used with --native)\n" <<
        "   --use-ms-link (-u)\n" <<
        "       Use Microsoft's link.exe as a linker if in path (used with --native)\n" <<
        "       Otherwise uses LLVM's lld-link as a linker.\n" <<
        std::endl;
}

int main(int argc, const char** argv)
{
    try
    {
        InitDone initDone;        
        std::vector<std::string> projectsAndSolutions;
        bool debugLlvm = false;
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            if (!arg.empty() && arg[0] == '-')
            {
                if (arg == "-v" || arg == "--verbose")
                {
                    SetGlobalFlag(GlobalFlags::verbose);
                }
                else if (arg == "-q" || arg == "--quiet")
                {
                    SetGlobalFlag(GlobalFlags::quiet);
                }
                else if (arg == "-h" || arg == "--help")
                {
                    PrintHelp();
                    return 0;
                }
                else if (arg == "-p" || arg == "--debug-parse")
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
                else if (arg == "-u" || arg == "--use-ms-link")
                {
                    SetGlobalFlag(GlobalFlags::useMsLink);
                }
                else if (arg == "--link-with-debug-machine")
                {
                    SetGlobalFlag(GlobalFlags::linkWithDebugMachine);
                }
                else if (arg == "-b" || arg == "--debug-llvm")
                {
                    debugLlvm = true;
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
                        else if (components[0] == "-i" || components[0] == "--inline-limit")
                        {
                            int inlineLimit = boost::lexical_cast<int>(components[1]);
                            SetInlineLimit(inlineLimit);
                        }
                        else if (components[0] == "-a" || components[0] == "--inline-locals")
                        {
                            int inlineLocals = boost::lexical_cast<int>(components[1]);
                            SetInlineLocals(inlineLocals);
                        }
                        else if (components[0] == "-s" || components[0] == "--debug-pass")
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
        if (projectsAndSolutions.empty())
        {
            PrintHelp();
            return 0;
        }
        if (GetGlobalFlag(GlobalFlags::native))
        {
            const int maxArgsPlusNull = 4;
            const char* argvForLlvm[maxArgsPlusNull];
            for (int i = 0; i < maxArgsPlusNull; ++i)
            {
                argvForLlvm[i] = nullptr;
            }
            std::vector<std::string> llvmArgs;
#ifdef NDEBUG
            llvmArgs.push_back("cminorc");
#else
            llvmArgs.push_back("cminorcd");
#endif
            const std::string& debugPassValue = GetDebugPassValue();
            if (!debugPassValue.empty())
            {
                llvmArgs.push_back("-debug-pass=" + debugPassValue);
            }
            if (debugLlvm)
            {
#ifdef NDEBUG
                std::cerr << "Warning: LLVM debug output not available for release mode compiler (use cminorcd).";
#else 
                llvmArgs.push_back("-debug");
#endif
            }
            int n = int(llvmArgs.size());
            if (n >= maxArgsPlusNull)
            {
                throw std::runtime_error("increase max args");
            }
            for (int i = 0; i < n; ++i)
            {
                argvForLlvm[i] = llvmArgs[i].c_str();
            }
            cl::ParseCommandLineOptions(n, argvForLlvm);
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
        if (!GetGlobalFlag(GlobalFlags::quiet))
        {
            std::cerr << ex.What() << std::endl;
        }
        return 1;
    }
    catch (const ProcessFailure& ex)
    {
        if (!GetGlobalFlag(GlobalFlags::quiet))
        {
            std::cerr << ex.what() << std::endl;
        }
        return ex.ExitCode();
    }
    catch (const std::exception& ex)
    {
        if (!GetGlobalFlag(GlobalFlags::quiet))
        {
            std::cerr << ex.what() << std::endl;
        }
        return 1;
    }
    return 0;
}
