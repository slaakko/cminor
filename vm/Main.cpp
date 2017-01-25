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
#include <cminor/machine/Path.hpp>
#include <cminor/machine/TextUtils.hpp>
#include <boost/filesystem.hpp>
#include <stdexcept>
#include <chrono>
#include <cminor/machine/Unicode.hpp>
#include <cminor/machine/Log.hpp>
#include <sstream>

using namespace cminor::machine;
using namespace cminor::symbols;
using namespace cminor::ast;
using namespace cminor::vmlib;

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
        ObjectType* argsArrayObjectType = nullptr;
        if (mainFun->Arity() == 1)
        {
            TypeSymbol* argsParamType = mainFun->Parameters()[0]->GetType();
            ArrayTypeSymbol* argsArrayType = dynamic_cast<ArrayTypeSymbol*>(argsParamType);
            if (argsArrayType && argsArrayType->ElementType() == assembly.GetSymbolTable().GetType(U"System.String"))
            {
                argsArrayObjectType = argsArrayType->GetObjectType();
            }
            else
            {
                throw Exception("parameter type of program main function is not string[]", mainFun->GetSpan());
            }
        }
        machine.Run(programArguments, argsArrayObjectType);
        if (mainFun->ReturnType() == assembly.GetSymbolTable().GetType(U"System.Int32"))
        {
            IntegralValue programReturnValue = machine.MainThread().OpStack().Pop();
            return programReturnValue.AsInt();
        }
        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
