#include <iostream>
#include <stdexcept>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Function.hpp>
#include <cminor/machine/FileRegistry.hpp>
#include <cminor/machine/Class.hpp>
#include <cminor/util/TextUtils.hpp>
#include <cminor/symbols/Symbol.hpp>
#include <cminor/symbols/Value.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolReader.hpp>
#include <cminor/symbols/GlobalFlags.hpp>
#include <cminor/symbols/VariableSymbol.hpp>
#include <cminor/ast/Project.hpp>
#include <cminor/vmlib/VmFunction.hpp>
#include <cminor/vmlib/File.hpp>
#include <cminor/db/Shell.hpp>
#include <cminor/pl/InitDone.hpp>
#include <cminor/util/Path.hpp>
#include <boost/filesystem.hpp>
#include <sstream>

using namespace cminor::machine;
using namespace cminor::symbols;
using namespace cminor::ast;
using namespace cminor::vmlib;
using namespace cminor::db;

struct InitDone
{
    InitDone()
    {
        SourceFileTable::Init();
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
        cminor::parsing::Init();
    }
    ~InitDone()
    {
        cminor::parsing::Done();
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
        SourceFileTable::Done();
    }
    ConstantPool vmFunctionNamePool;
};

const char* version = "0.0.1";

void PrintHelp()
{
    std::cout << "Cminor debugger version " << version << "\n\n" <<
        "Usage: cminordb [options] program.cminora [arguments]\n" <<
        "Debug program.cminora with given arguments.\n" <<
        "Options:\n" <<
        "-h | --help     : print this help message" <<
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
        SetDebugging();
        Machine machine;
        Assembly assembly(machine);
        assembly.Load(assemblyFilePath);
        std::vector<utf32_string> programArguments;
        for (const std::string& arg : arguments)
        {
            programArguments.push_back(ToUtf32(arg));
        }
        FunctionSymbol* mainFun = assembly.GetSymbolTable().GetMainFunction();
        if (!mainFun)
        {
            throw std::runtime_error("program has no main function");
        }
        Shell shell(machine);
        shell.Run(mainFun, assembly, programArguments);
        return 0;
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
