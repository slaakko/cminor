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
#include <cminor/vmlib/Threading.hpp>
#include <cminor/db/Shell.hpp>
#include <cminor/pl/InitDone.hpp>
#include <cminor/util/Path.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
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
        ThreadingInit();
        cminor::parsing::Init();
    }
    ~InitDone()
    {
        cminor::parsing::Done();
        ThreadingDone();
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

const char* version = "0.1.0";

void PrintHelp()
{
    std::cout << "\n" <<
        "Cminor debugger version " << version << "\n\n" <<
        "Usage: cminordb [options] PROGRAM.cminora [program-arguments]\n\n" <<
        "Debug PROGRAM.cminora with given program-arguments.\n\n" <<
        "options:\n\n" <<
        "   --help (-h)\n" <<
        "       Print this help message.\n" <<
        "   --segment-size=SEGMENT-SIZE (-s=SEGMENT-SIZE)\n" <<
        "       SEGMENT-SIZE is the size of the garbage collected memory segment in megabytes.\n" <<
        "       Default is 16 MB.\n" <<
        "   --pool-threshold=POOL-THRESHOLD (-p=POOL-THRESHOLD)\n" <<
        "       POOL-THRESHOLD is the grow threshold of the managed memory pool in megabytes.\n" <<
        "       Default is 16 MB.\n" <<
        "   --thread-pages=N (-t=N)\n" <<
        "       Set the number of thread-specific memory allocation context pages to N.\n" <<
        "       Default is 2 pages (for 4K system memory page size this is 8K).\n" <<
        "       When N > 0, memory allocator of the virtual machine allocates extra memory\n" <<
        "       whose size is N * <system memory page size> for the thread making the allocation.\n" <<
        "       Thread can consume this extra memory without any further locking.\n" <<
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
                                segmentSizeMB = boost::lexical_cast<uint64_t>(components[1]);
                                SetSegmentSize(segmentSizeMB * 1024 * 1024);
                            }
                            else if (components[0] == "-p" || components[0] == "--pool-threshold")
                            {
                                poolThresholdMB = boost::lexical_cast<uint64_t>(components[1]);
                                SetPoolThreshold(poolThresholdMB * 1024 * 1024);
                            }
                            else if (components[0] == "-t" || components[0] == "--thread-pages")
                            {
                                uint8_t threadPages = boost::lexical_cast<uint8_t>(components[1]);
                                SetNumAllocationContextPages(threadPages);
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
