// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <iostream>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Function.hpp>
#include <cminor/machine/FileRegistry.hpp>
#include <cminor/machine/Class.hpp>
#include <cminor/symbols/Symbol.hpp>
#include <cminor/symbols/Value.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolReader.hpp>
#include <cminor/symbols/GlobalFlags.hpp>
#include <cminor/symbols/VariableSymbol.hpp>
#include <cminor/ast/Project.hpp>
#include <cminor/vmlib/VmFunction.hpp>
#include <cminor/vmlib/File.hpp>
#include <cminor/util/Path.hpp>
#include <cminor/util/TextUtils.hpp>
#include <cminor/jit/JitCompiler.hpp>
#include <llvm/Support/TargetSelect.h>
#include <boost/filesystem.hpp>
#include <stdexcept>
#include <chrono>
#include <cminor/util/Unicode.hpp>
#include <cminor/machine/Log.hpp>
#include <sstream>

using namespace cminor::machine;
using namespace cminor::jit;
using namespace cminor::symbols;
using namespace cminor::ast;
using namespace cminor::vmlib;
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
        VmFunctionTable::Init();
        InitSymbol();
        ValueInit();
        InitAssembly();
        InitVmFunctions(vmFunctionNamePool);
        FileInit();
#ifdef GC_LOGGING
        OpenLog();
#endif
    }
    ~InitDone()
    {
#ifdef GC_LOGGING
        CloseLog();
#endif
        FileDone();
        DoneVmFunctions();
        DoneAssembly();
        ValueDone();
        DoneSymbol();
        VmFunctionTable::Done();
        NodeDone();
        TypeDone();
        ClassDataTable::Done();
        FunctionTable::Done();
    }
    ConstantPool vmFunctionNamePool;
};

const char* version = "0.0.1";

void PrintHelp()
{
    std::cout << "Cminor virtual machine version " << version << "\n\n" <<
        "Usage: cminorvm [options] program.cminora [arguments]\n" <<
        "Run program.cminora with given arguments.\n" <<
        "Options:\n" <<
        "-h | --help     : print this help message\n" <<
        "-j | --jit      : use just in time compilation\n" <<
        "-s=SEGMENT-SIZE | --segment-size=SEGMENT-SIZE:\n" <<
        "       SEGMENT-SIZE is the size of the garbage collected memory\n" <<
        "       segment in megabytes. The default is 16 MB.\n" << 
        "-p=POOL-THRESHOLD | --pool-threshold=POOL-THRESHOLD:\n" <<
        "       POOL-THRESHOLD is the grow threshold of the managed\n" <<
        "       memory pool in megabytes. The default is 16 MB.\n" <<
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
        uint64_t segmentSizeMB = 0;
        uint64_t poolThresholdMB = 0;
        bool jit = false;
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
                        else if (arg == "-j" || arg == "--jit")
                        {
                            jit = true;
                        }
                        else if (arg.find('=', 0) != std::string::npos)
                        {
                            std::vector<std::string> components = Split(arg, '=');
                            if (components.size() != 2)
                            {
                                throw std::runtime_error("invalid argument '" + arg + "'");
                            }
                            if (components[0] == "-s" || components[0] == "--segment-size")
                            {
                                std::stringstream s;
                                s.str(components[1]);
                                if (!(s >> segmentSizeMB))
                                {
                                    throw std::runtime_error("segment size not of an integer type: " + arg);
                                }
                                else
                                {
                                    SetSegmentSize(segmentSizeMB * 1024 * 1024);
                                }
                            }
                            else if (components[0] == "-p" || components[0] == "--pool-threshold")
                            {
                                std::stringstream s;
                                s.str(components[1]);
                                if (!(s >> poolThresholdMB))
                                {
                                    throw std::runtime_error("pool threshold not of an integer type: " + arg);
                                }
                                else
                                {
                                    SetPoolThreshold(poolThresholdMB * 1024 * 1024);
                                }
                            }
                            else
                            {
                                throw std::runtime_error("unknown run option '" + arg + "'");
                            }
                        }
                        else
                        {
                            throw std::runtime_error("unknown run option '" + arg + "'");
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
        assembly.Load(assemblyFilePath);
        std::vector<utf32_string> programArguments;
        for (const std::string& arg : arguments)
        {
            programArguments.push_back(ToUtf32(arg));
        }
        if (!jit)
        {
            return assembly.RunIntermediateCode(programArguments);
        }
        if (jit)
        {
            InitializeNativeTarget();
            InitializeNativeTargetAsmPrinter();
            InitializeNativeTargetAsmParser();
            Function* main = FunctionTable::GetMain();
            JitCompiler jitCompiler(*main);
            return jitCompiler.ProgramReturnValue();
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
