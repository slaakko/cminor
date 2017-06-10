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
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolReader.hpp>
#include <cminor/symbols/GlobalFlags.hpp>
#include <cminor/symbols/Value.hpp>
#include <cminor/util/Path.hpp>
#include <cminor/util/System.hpp>
#include <boost/filesystem.hpp>
#include <stdexcept>

using namespace cminor::machine;
using namespace cminor::symbols;
using namespace cminor::ast;
using namespace cminor::machine;
using namespace cminor::util;

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
    }
    ~InitDone()
    {
        DoneAssembly();
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
    std::cout << "Cminor assembly dump version " << version << "\n\n" <<
        "Usage: cminordump [options] assembly.cminora [output-file]\n" <<
        "Dump information in assembly.cminora to standard output, or to given output file.\n" <<
        "Options:\n" <<
        "-h | --help      : print this help message" <<
        "-v | --verbose   : verbose output\n" <<
        "-q | --quiet     : no error output, just return exit code\n"
        "-a | --all       : dump all (default)\n" <<
        "-e | --header    : dump assembly header\n" <<
        "-c | --constants : dump constant pool\n" <<
        "-f | --functions : dump function table\n" <<
        "-s | --symbols   : dump symbol table\n" <<
        "-m | --mappings  : dump mappings\n" <<
        std::endl;
}

int main(int argc, const char** argv)
{
    try
    {
        DumpOptions dumpOptions = DumpOptions::none;
        std::string assemblyFileName;
        std::string outputFileName;
        bool verbose = false;
        if (argc < 2)
        {
            PrintHelp();
            return 0;
        }
        InitDone initDone;
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            if (!arg.empty() && arg[0] == '-')
            {
                if (arg == "-h" || arg == "--help")
                {
                    PrintHelp();
                    return 0;
                }
                else if (arg == "-v" || arg == "--verbose")
                {
                    verbose = true;
                }
                else if (arg == "-q" || arg == "--quiet")
                {
                    SetGlobalFlag(GlobalFlags::quiet);
                }
                else if (arg == "-a" || arg == "--all")
                {
                    dumpOptions = DumpOptions::all;
                }
                else if (arg == "-e" || arg == "--header")
                {
                    dumpOptions = dumpOptions | DumpOptions::header;
                }
                else if (arg == "-c" || arg == "--constants")
                {
                    dumpOptions = dumpOptions | DumpOptions::constants;
                }
                else if (arg == "-f" || arg == "--functions")
                {
                    dumpOptions = dumpOptions | DumpOptions::functions;
                }
                else if (arg == "-s" || arg == "--symbols")
                {
                    dumpOptions = dumpOptions | DumpOptions::symbols;
                }
                else if (arg == "-m" || arg == "--mappings")
                {
                    dumpOptions = dumpOptions | DumpOptions::mappings;
                }
                else
                {
                    throw std::runtime_error("unknown argument '" + arg + "'");
                }
            }
            else
            {
                if (assemblyFileName.empty())
                {
                    assemblyFileName = arg;
                }
                else
                {
                    outputFileName = arg;
                }
            }
        }
        if (assemblyFileName.empty())
        {
            throw std::runtime_error("no assembly file given");
        }
        boost::filesystem::path afp = assemblyFileName;
        if (afp.extension() != ".cminora")
        {
            throw std::runtime_error("argument '" + afp.generic_string() + "' has invalid extension (not .cminora)");
        }
        std::string assemblyFilePath = GetFullPath(afp.generic_string());
        if (!boost::filesystem::exists(afp))
        {
            throw std::runtime_error("assembly file '" + assemblyFilePath + "' not found");
        }
        Machine machine;
        Assembly assembly(machine);
        assembly.Load(assemblyFilePath);
        FunctionTable::Done();
        ClassDataTable::Done();
        TypeTable::Done();
        std::ostream* outputStream = &std::cout;
        std::ofstream outputFileStream;
        if (!outputFileName.empty())
        {
            outputFileStream.open(outputFileName);
            outputStream = &outputFileStream;
        }
        if (dumpOptions == DumpOptions::none)
        {
            dumpOptions = DumpOptions::all;
        }
        if (verbose)
        {
            std::string description;
            if ((dumpOptions & DumpOptions::header) != DumpOptions::none)
            {
                description = "header";
            }
            if ((dumpOptions & DumpOptions::constants) != DumpOptions::none)
            {
                if (!description.empty())
                {
                    description.append(" and ");
                }
                description = "constants";
            }
            if ((dumpOptions & DumpOptions::functions) != DumpOptions::none)
            {
                if (!description.empty())
                {
                    description.append(" and ");
                }
                description = "functions";
            }
            if ((dumpOptions & DumpOptions::symbols) != DumpOptions::none)
            {
                if (!description.empty())
                {
                    description.append(" and ");
                }
                description = "symbols";
            }
            if ((dumpOptions & DumpOptions::mappings) != DumpOptions::none)
            {
                if (!description.empty())
                {
                    description.append(" and ");
                }
                description = "mappings";
            }
            std::string target = "standard output";
            if (!outputFileName.empty())
            {
                target = "file " + outputFileName;
            }
            std::cout << "dumping " << description << " to " << target << std::endl;
        }
        CodeFormatter codeFormatter(*outputStream);
        assembly.Dump(codeFormatter, dumpOptions);
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
