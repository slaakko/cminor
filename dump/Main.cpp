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
#include <cminor/machine/Path.hpp>
#include <boost/filesystem.hpp>
#include <stdexcept>

using namespace cminor::machine;
using namespace cminor::symbols;
using namespace cminor::ast;
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

const char* version = "0.0.1";

void PrintHelp()
{
    std::cout << "Cminor assembly dump version " << version << "\n\n" <<
        "Usage: cminordump [options] assembly.cminora [outputfile]\n" <<
        "Dump information in assembly.cminora.\n" <<
        "Options:\n" <<
        "-a | --all       : dump all (default)\n" <<
        "-f | --functions : dump functions\n" <<
        "-h | --help      : print this help message" <<
        "-s | --symbols   : dump symbols\n" <<
        "-v | --verbose   : verbose output\n" <<
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
                else if (arg == "-f" || arg == "--functions")
                {
                    dumpOptions = dumpOptions | DumpOptions::functions;
                }
                else if (arg == "-s" || arg == "--symbols")
                {
                    dumpOptions = dumpOptions | DumpOptions::symbols;
                }
                else if (arg == "-a" || arg == "--all")
                {
                    dumpOptions = dumpOptions | DumpOptions::functions | DumpOptions::symbols;
                }
                else if (arg == "-v" || arg == "--verbose")
                {
                    verbose = true;
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
        const Assembly* rootAssembly = &assembly;
        SymbolReader symbolReader(assemblyFilePath);
        std::vector<CallInst*> callInstructions;
        std::vector<Fun2DlgInst*> fun2DlgInstructions;
        std::vector<MemFun2ClassDlgInst*> memFun2ClassDlgInstructions;
        std::vector<TypeInstruction*> typeInstructions;
        std::vector<SetClassDataInst*> setClassDataInstructions;
        std::vector<ClassTypeSymbol*> classTypes;
        std::string currentAssemblyDir = GetFullPath(boost::filesystem::path(assemblyFilePath).remove_filename().generic_string());
        std::unordered_set<std::string> importSet;
        std::unordered_set<utf32_string> classTemplateSpecializationNames;
        std::vector<Assembly*> assemblies;
        std::unordered_map<std::string, AssemblyDependency*> assemblyDependencyMap;
        assembly.BeginRead(symbolReader, LoadType::execute, rootAssembly, currentAssemblyDir, importSet, callInstructions, fun2DlgInstructions, memFun2ClassDlgInstructions, typeInstructions, 
            setClassDataInstructions, classTypes, classTemplateSpecializationNames, assemblies, assemblyDependencyMap);
        auto it = std::unique(assemblies.begin(), assemblies.end());
        assemblies.erase(it, assemblies.end());
        std::unordered_map<Assembly*, AssemblyDependency*> dependencyMap;
        for (const auto& p : assemblyDependencyMap)
        {
            dependencyMap[p.second->GetAssembly()] = p.second;
        }
        std::vector<Assembly*> finishReadOrder = CreateFinishReadOrder(assemblies, dependencyMap, rootAssembly);
        assembly.FinishReads(callInstructions, fun2DlgInstructions, memFun2ClassDlgInstructions, typeInstructions, setClassDataInstructions, classTypes, classTemplateSpecializationNames, 
            int(finishReadOrder.size() - 2), finishReadOrder, true);
        assembly.GetSymbolTable().MergeClassTemplateSpecializations();
        Link(callInstructions, fun2DlgInstructions, memFun2ClassDlgInstructions, typeInstructions, setClassDataInstructions, classTypes);
        callInstructions.clear();
        fun2DlgInstructions.clear();
        memFun2ClassDlgInstructions.clear();
        typeInstructions.clear();
        setClassDataInstructions.clear();
        classTypes.clear();
        classTemplateSpecializationNames.clear();
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
            dumpOptions = DumpOptions::functions | DumpOptions::symbols;
        }
        if (verbose)
        {
            std::string description;
            if (dumpOptions == DumpOptions::functions)
            {
                description = "functions";
            }
            else if (dumpOptions == DumpOptions::symbols)
            {
                description = "symbols";
            }
            else 
            {
                description = "functions and symbols";
            }
            std::string target = "standard output";
            if (!outputFileName.empty())
            {
                target = "file " + outputFileName;
            }
            std::cout << "Dumping " << description << " to " << target << std::endl;
        }
        CodeFormatter codeFormatter(*outputStream);
        assembly.Dump(codeFormatter, dumpOptions);
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
