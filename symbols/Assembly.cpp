// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolReader.hpp>
#include <cminor/symbols/GlobalFlags.hpp>
#include <cminor/symbols/VariableSymbol.hpp>
#include <cminor/ast/Project.hpp>
#include <cminor/machine/Class.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Runtime.hpp>
#include <cminor/machine/OsInterface.hpp>
#include <cminor/machine/Stats.hpp>
#include <boost/filesystem.hpp>
#include <cminor/util/Path.hpp>
#include <cminor/util/TextUtils.hpp>
#include <cminor/util/Prime.hpp>
#include <cminor/util/Sha1.hpp>
#include <iostream>

namespace cminor { namespace symbols {

using namespace cminor::ast;
using namespace cminor::machine;

const char* cminorAssemblyTag = "CMNA";

class CminorSystemAssemblyNameCollection
{
public:
    static void Init();
    static void Done();
    static CminorSystemAssemblyNameCollection& Instance() { Assert(instance, "system assembly names not set"); return *instance; }
    bool Find(StringPtr assemblyName) const;
private:
    static std::unique_ptr<CminorSystemAssemblyNameCollection> instance;
    std::vector<utf32_string> systemAssemblyNames;
    std::unordered_set<StringPtr, StringPtrHash> systemAssemblyNameMap;
    CminorSystemAssemblyNameCollection();
};

std::unique_ptr<CminorSystemAssemblyNameCollection> CminorSystemAssemblyNameCollection::instance;

void CminorSystemAssemblyNameCollection::Init()
{
    instance.reset(new CminorSystemAssemblyNameCollection());
}

void CminorSystemAssemblyNameCollection::Done()
{
    instance.reset();
}

CminorSystemAssemblyNameCollection::CminorSystemAssemblyNameCollection()
{
    systemAssemblyNames.push_back(U"System");
    systemAssemblyNames.push_back(U"System.Core");
    systemAssemblyNames.push_back(U"System.Base");
    systemAssemblyNames.push_back(U"System.Text.Parsing");
    systemAssemblyNames.push_back(U"System.Text.Parsing.CodeDom");
    systemAssemblyNames.push_back(U"System.Json");
    systemAssemblyNames.push_back(U"System.Net.Sockets");
    systemAssemblyNames.push_back(U"System.Net.Http");
    systemAssemblyNames.push_back(U"System.Xml");
    for (const utf32_string& systemAssemblyName : systemAssemblyNames)
    {
        systemAssemblyNameMap.insert(StringPtr(systemAssemblyName.c_str()));
    }
}

bool CminorSystemAssemblyNameCollection::Find(StringPtr assemblyName) const
{
    return systemAssemblyNameMap.find(assemblyName) != systemAssemblyNameMap.cend();
}

class AssemblyTag
{
public:
    AssemblyTag();
    void Write(Writer& writer);
    void Read(Reader& reader);
    char operator[](int index) const { return tag[index]; }
private:
    char tag[4];
};

AssemblyTag::AssemblyTag()
{
    tag[0] = cminorAssemblyTag[0];
    tag[1] = cminorAssemblyTag[1];
    tag[2] = cminorAssemblyTag[2];
    tag[3] = cminorAssemblyTag[3];
}

void AssemblyTag::Write(Writer& writer)
{
    writer.Put(static_cast<uint8_t>(tag[0]));
    writer.Put(static_cast<uint8_t>(tag[1]));
    writer.Put(static_cast<uint8_t>(tag[2]));
    writer.Put(static_cast<uint8_t>(tag[3]));
}

void AssemblyTag::Read(Reader& reader)
{
    tag[0] = static_cast<char>(reader.GetByte());
    tag[1] = static_cast<char>(reader.GetByte());
    tag[2] = static_cast<char>(reader.GetByte());
    tag[3] = static_cast<char>(reader.GetByte());
}

inline bool operator==(const AssemblyTag& left, const AssemblyTag& right)
{
    return left[0] == right[0] && left[1] == right[1] && left[2] == right[2] && left[3] == right[3];
}

inline bool operator!=(const AssemblyTag& left, const AssemblyTag& right)
{
    return !(left == right);
}

AssemblyDependency::AssemblyDependency(Assembly* assembly_) : assembly(assembly_)
{
}

void AssemblyDependency::AddReferencedAssembly(Assembly* referencedAssembly)
{
    if (std::find(referencedAssemblies.cbegin(), referencedAssemblies.cend(), referencedAssembly) == referencedAssemblies.cend())
    {
        referencedAssemblies.push_back(referencedAssembly);
    }
}

void AssemblyDependency::Dump(CodeFormatter& formatter)
{
    formatter.IncIndent();
    if (referencedAssemblies.empty())
    {
        formatter.WriteLine("none");
    }
    else
    {
        for (Assembly* referencedAssembly : referencedAssemblies)
        {
            formatter.WriteLine(ToUtf8(referencedAssembly->Name().Value()));
            referencedAssembly->GetAssemblyDependency()->Dump(formatter);
        }
    }
    formatter.DecIndent();
}

void Visit(std::vector<Assembly*>& finishReadOrder, Assembly* assembly, std::unordered_set<Assembly*>& visited, std::unordered_set<Assembly*>& tempVisit,
    std::unordered_map<Assembly*, AssemblyDependency*>& dependencyMap, const Assembly* rootAssembly)
{
    if (tempVisit.find(assembly) == tempVisit.cend())
    {
        if (visited.find(assembly) == visited.cend())
        {
            tempVisit.insert(assembly);
            auto i = dependencyMap.find(assembly);
            if (i != dependencyMap.cend())
            {
                AssemblyDependency* dependency = i->second;
                for (Assembly* dependentAssembly : dependency->ReferencedAssemblies())
                {
                    Visit(finishReadOrder, dependentAssembly, visited, tempVisit, dependencyMap, rootAssembly);
                }
                tempVisit.erase(assembly);
                visited.insert(assembly);
                finishReadOrder.push_back(assembly);
            }
            else
            {
                throw std::runtime_error("assembly '" + ToUtf8(assembly->Name().Value()) + "' not found in dependencies of assembly '" + ToUtf8(rootAssembly->Name().Value()) + "'");
            }
        }
    }
    else
    {
        throw std::runtime_error("circular assembly dependency '" + ToUtf8(assembly->Name().Value()) + "' detected in dependencies of assembly '" + ToUtf8(rootAssembly->Name().Value()) + "'");
    }
}

std::vector<Assembly*> CreateFinishReadOrder(const std::vector<Assembly*>& assemblies, std::unordered_map<Assembly*, AssemblyDependency*>& dependencyMap, const Assembly* rootAssembly)
{
    std::vector<Assembly*> finishReadOrder;
    std::unordered_set<Assembly*> visited;
    std::unordered_set<Assembly*> tempVisit;
    for (Assembly* assembly : assemblies)
    {
        if (visited.find(assembly) == visited.cend())
        {
            Visit(finishReadOrder, assembly, visited, tempVisit, dependencyMap, rootAssembly);
        }
    }
    return finishReadOrder;
}

std::string AssemblyFlagsStr(AssemblyFlags flags)
{
    std::string s;
    if (flags == AssemblyFlags::none)
    {
        s.append("none");
    }
    else
    {
        if ((flags & AssemblyFlags::core) != AssemblyFlags::none)
        {
            s.append("core");
        }
        if ((flags & AssemblyFlags::system) != AssemblyFlags::none)
        {
            if (!s.empty())
            {
                s.append(" ");
            }
            s.append("system");
        }
        if ((flags & AssemblyFlags::native) != AssemblyFlags::none)
        {
            if (!s.empty())
            {
                s.append(" ");
            }
            s.append("native");
        }
        if ((flags & AssemblyFlags::release) != AssemblyFlags::none)
        {
            if (!s.empty())
            {
                s.append(" ");
            }
            s.append("release");
        }
        if ((flags & AssemblyFlags::linkedWithDebugMachine) != AssemblyFlags::none)
        {
            if (!s.empty())
            {
                s.append(" ");
            }
            s.append("linkedWithDebugMachine");
        }
    }
    return s;
}

Assembly::Assembly(Machine& machine_) : machine(machine_), flags(AssemblyFlags::none), assemblyFormat(currentAssemblyFormat), originalFilePath(), constantPool(), symbolTable(this), name(), id(-1), 
    finishReadPos(0), assemblyDependency(this), transientFlags(TransientAssemblyFlags::none), sharedLibraryHandle(nullptr), mainEntryPointAddress(nullptr)
{
    if (GetGlobalFlag(GlobalFlags::release))
    {
        SetRelease();
    }
}

Assembly::Assembly(Machine& machine_, const utf32_string& name_, const std::string& filePath_) : machine(machine_), flags(AssemblyFlags::none), assemblyFormat(currentAssemblyFormat), 
    originalFilePath(filePath_), constantPool(), symbolTable(this), name(constantPool.GetConstant(constantPool.Install(StringPtr(name_.c_str())))), id(-1), finishReadPos(0), 
    assemblyDependency(this), transientFlags(TransientAssemblyFlags::none), sharedLibraryHandle(nullptr), mainEntryPointAddress(nullptr)
{
    bool isSystemAssemblyName = CminorSystemAssemblyNameCollection::Instance().Find(StringPtr(name.Value().AsStringLiteral()));
    if (isSystemAssemblyName)
    {
        SetSystemAssembly();
    }
    if (GetGlobalFlag(GlobalFlags::release))
    {
        SetRelease();
    }
    hash = GetSha1MessageDigest(ToUtf8(name.Value().AsStringLiteral()));
}

Assembly::~Assembly()
{
    if (sharedLibraryHandle)
    {
        FreeSharedLibrary(sharedLibraryHandle);
    }
}

void Assembly::Load(const std::string& assemblyFilePath)
{
    auto startLoad = std::chrono::system_clock::now();
    const Assembly* rootAssembly = this;
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
    std::unordered_map<std::string, Assembly*> readMap;
    BeginRead(symbolReader, LoadType::execute, rootAssembly, currentAssemblyDir, importSet, callInstructions, fun2DlgInstructions, memFun2ClassDlgInstructions,
        typeInstructions, setClassDataInstructions, classTypes, classTemplateSpecializationNames, assemblies, assemblyDependencyMap, readMap);
    auto it = std::unique(assemblies.begin(), assemblies.end());
    assemblies.erase(it, assemblies.end());
    std::unordered_map<Assembly*, AssemblyDependency*> dependencyMap;
    for (const auto& p : assemblyDependencyMap)
    {
        dependencyMap[p.second->GetAssembly()] = p.second;
    }
    std::vector<Assembly*> finishReadOrder = CreateFinishReadOrder(assemblies, dependencyMap, rootAssembly);
    FinishReads(callInstructions, fun2DlgInstructions, memFun2ClassDlgInstructions, typeInstructions, setClassDataInstructions, classTypes,
        classTemplateSpecializationNames, int(finishReadOrder.size() - 2), finishReadOrder, true);
    symbolTable.MergeClassTemplateSpecializations();
    auto startLink = std::chrono::system_clock::now();
    Link(callInstructions, fun2DlgInstructions, memFun2ClassDlgInstructions, typeInstructions, setClassDataInstructions, classTypes);
    auto endLink = std::chrono::system_clock::now();
    auto endLoad = std::chrono::system_clock::now();
    auto loadDuration = endLoad - startLoad;
    auto linkDuration = endLink - startLink;
    int64_t msLoad = std::chrono::duration_cast<std::chrono::milliseconds>(loadDuration).count();
    int64_t msLink = std::chrono::duration_cast<std::chrono::milliseconds>(linkDuration).count();
    AddLoadTime(msLoad);
    AddLinkTime(msLink);
}

void Assembly::PrepareForCompilation(const std::vector<std::string>& projectAssemblyReferences)
{
    AssemblyTable::Instance().AddAssembly(this);
    const Assembly* rootAssembly = this;
    std::vector<CallInst*> callInstructions;
    std::vector<Fun2DlgInst*> fun2DlgInstructions;
    std::vector<MemFun2ClassDlgInst*> memFun2ClassDlgInstructions;
    std::vector<TypeInstruction*> typeInstructions;
    std::vector<SetClassDataInst*> setClassDataInstructions;
    std::vector<ClassTypeSymbol*> classTypes;
    std::unordered_set<utf32_string> classTemplateSpecializationNames;
    std::vector<Assembly*> assemblies;
    std::unordered_map<std::string, AssemblyDependency*> assemblyDependencyMap;
    std::unordered_map<std::string, Assembly*> readMap;
    std::string currentAssemblyDir = GetFullPath(boost::filesystem::path(OriginalFilePath()).remove_filename().generic_string());
    std::unordered_set<std::string> importSet;
    ImportAssemblies(projectAssemblyReferences, LoadType::build, rootAssembly, currentAssemblyDir, importSet, callInstructions, fun2DlgInstructions,
        memFun2ClassDlgInstructions, typeInstructions, setClassDataInstructions, classTypes, classTemplateSpecializationNames, assemblies, assemblyDependencyMap, readMap);
    assemblies.push_back(this);
    auto it = std::unique(assemblies.begin(), assemblies.end());
    assemblies.erase(it, assemblies.end());
    assemblyDependencyMap[OriginalFilePath()] = &assemblyDependency;
    std::unordered_map<Assembly*, AssemblyDependency*> dependencyMap;
    for (const auto& p : assemblyDependencyMap)
    {
        dependencyMap[p.second->GetAssembly()] = p.second;
    }
    std::vector<Assembly*> finishReadOrder = CreateFinishReadOrder(assemblies, dependencyMap, rootAssembly);
    FinishReads(callInstructions, fun2DlgInstructions, memFun2ClassDlgInstructions, typeInstructions, setClassDataInstructions, classTypes,
        classTemplateSpecializationNames, int(finishReadOrder.size() - 2), finishReadOrder, false);
    symbolTable.MergeClassTemplateSpecializations();
}

int Assembly::RunIntermediateCode(const std::vector<utf32_string>& programArguments)
{
    int returnValue = 0;
    FunctionSymbol* mainFun = symbolTable.GetMainFunction();
    if (!mainFun)
    {
        throw std::runtime_error("assembly '" + ToUtf8(Name().Value()) + "' has no main function");
    }
    ObjectType* argsArrayObjectType = nullptr;
    bool runWithArgs = false;
    if (mainFun->Arity() != 1 && mainFun->Arity() != 0)
    {
        throw std::runtime_error("main function has invalid arity " + std::to_string(mainFun->Arity()));
    }
    if (mainFun->Arity() == 1)
    {
        TypeSymbol* argsParamType = mainFun->Parameters()[0]->GetType();
        ArrayTypeSymbol* argsArrayType = dynamic_cast<ArrayTypeSymbol*>(argsParamType);
        if (argsArrayType && argsArrayType->ElementType() == symbolTable.GetType(U"System.String"))
        {
            argsArrayObjectType = argsArrayType->GetObjectType();
        }
        else
        {
            throw Exception("parameter type of program main function is not string[]", mainFun->GetSpan());
        }
        runWithArgs = true;
    }
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "running intermediate code of program '" + ToUtf8(Name().Value()) + "'..." << std::endl;
    }
    auto startRun = std::chrono::system_clock::now();
    machine.RunMain(runWithArgs, programArguments, argsArrayObjectType);
    auto endRun = std::chrono::system_clock::now();
    auto runDuration = endRun - startRun;
    int64_t runMs = std::chrono::duration_cast<std::chrono::milliseconds>(runDuration).count();
    AddRunTime(runMs);
    if (mainFun->ReturnType() == symbolTable.GetType(U"System.Int32"))
    {
        IntegralValue programReturnValue = machine.MainThread().OpStack().Pop();
        returnValue = programReturnValue.AsInt();
        if (GetGlobalFlag(GlobalFlags::verbose))
        {
            std::cout << "program returned exit code " << returnValue << std::endl;
        }
    }
    else
    {
        if (GetGlobalFlag(GlobalFlags::verbose))
        {
            std::cout << "program exited normally" << std::endl;
        }
    }
    return returnValue;
}

int Assembly::RunNative(const std::vector<utf32_string>& programArguments)
{
    if (!IsNative())
    {
        throw std::runtime_error("program '" + ToUtf8(Name().Value()) + "' is not built with --native option");
    }
#ifdef NDEBUG
    if (LinkedWithDebugMachine())
    {
        throw std::runtime_error("Cannot run program '" + ToUtf8(Name().Value()) + 
            "' using release mode virtual machine (cminorvm) because it is linked with the debug version of the Cminor virtual machine (cminormachined.dll). " + 
            "Rebuild the program and system libraries without the --link-with-debug-machine option.");
    }
#endif
    FunctionSymbol* mainFun = symbolTable.GetMainFunction();
    if (!mainFun)
    {
        throw std::runtime_error("assembly '" + ToUtf8(Name().Value()) + "' has no main function");
    }
    auto startPrepare = std::chrono::system_clock::now();
    PrepareForNativeExecution();
    auto endPrepare = std::chrono::system_clock::now();
    auto prepareDuration = endPrepare - startPrepare;
    int64_t prepareMs = std::chrono::duration_cast<std::chrono::milliseconds>(prepareDuration).count();
    AddPrepareTime(prepareMs);
    if (!mainEntryPointAddress)
    {
        throw std::runtime_error("main entry point not resolved");
    }
    int returnValue = 0;
    ObjectType* argsArrayObjectType = nullptr;
    bool startWithArgs = false;
    if (mainFun->Arity() != 1 && mainFun->Arity() != 0)
    {
        throw std::runtime_error("main function has invalid arity " + std::to_string(mainFun->Arity()));
    }
    if (mainFun->Arity() == 1)
    {
        TypeSymbol* argsParamType = mainFun->Parameters()[0]->GetType();
        ArrayTypeSymbol* argsArrayType = dynamic_cast<ArrayTypeSymbol*>(argsParamType);
        if (argsArrayType && argsArrayType->ElementType() == symbolTable.GetType(U"System.String"))
        {
            argsArrayObjectType = argsArrayType->GetObjectType();
        }
        else
        {
            throw Exception("parameter type of program main function is not string[]", mainFun->GetSpan());
        }
        startWithArgs = true;
    }
    SetRunningNativeCode();
    machine.Start(startWithArgs, programArguments, argsArrayObjectType);
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "running native code of program '" + ToUtf8(Name().Value()) + "'..." << std::endl;
    }
    try
    {
        if (startWithArgs)
        {
            IntegralValue argumentArrayValue = machine.MainThread().OpStack().Pop();
            if (argumentArrayValue.GetType() != ValueType::objectReference)
            {
                throw std::runtime_error("object reference expected");
            }
            ObjectReference args(argumentArrayValue.Value());
            if (!mainFun->ReturnType())
            {
                throw std::runtime_error("main function has no return type");
            }
            if (mainFun->ReturnType()->IsVoidType())
            {
                typedef void(*MainFunctionType)(uint64_t);
                MainFunctionType main = reinterpret_cast<MainFunctionType>(mainEntryPointAddress);
                GetCurrentThread().SetState(ThreadState::running);
                auto startRun = std::chrono::system_clock::now();
                {
                    ThreadExitSetter exitSetter(GetMachine().MainThread());
                    main(args.Value());
                }
                auto endRun = std::chrono::system_clock::now();
                auto runDuration = endRun - startRun;
                int64_t runMs = std::chrono::duration_cast<std::chrono::milliseconds>(runDuration).count();
                AddRunTime(runMs);
                if (GetGlobalFlag(GlobalFlags::verbose))
                {
                    std::cout << "program exited normally" << std::endl;
                }
            }
            else
            {
                if (mainFun->ReturnType() == symbolTable.GetType(U"System.Int32"))
                {
                    typedef int32_t(*MainFunctionType)(uint64_t);
                    MainFunctionType main = reinterpret_cast<MainFunctionType>(mainEntryPointAddress);
                    GetCurrentThread().SetState(ThreadState::running);
                    auto startRun = std::chrono::system_clock::now();
                    {
                        ThreadExitSetter exitSetter(GetMachine().MainThread());
                        returnValue = main(args.Value());
                    }
                    auto endRun = std::chrono::system_clock::now();
                    auto runDuration = endRun - startRun;
                    int64_t runMs = std::chrono::duration_cast<std::chrono::milliseconds>(runDuration).count();
                    AddRunTime(runMs);
                    if (GetGlobalFlag(GlobalFlags::verbose))
                    {
                        std::cout << "program returned exit code " << returnValue << std::endl;
                    }
                }
                else
                {
                    throw std::runtime_error("main function has invalid return type");
                }
            }
        }
        else
        {
            if (!mainFun->ReturnType())
            {
                throw std::runtime_error("main function has no return type");
            }
            if (mainFun->ReturnType()->IsVoidType())
            {
                typedef void(*MainFunctionType)(void);
                MainFunctionType main = reinterpret_cast<MainFunctionType>(mainEntryPointAddress);
                GetCurrentThread().SetState(ThreadState::running);
                auto startRun = std::chrono::system_clock::now();
                {
                    ThreadExitSetter exitSetter(GetMachine().MainThread());
                    main();
                }
                auto endRun = std::chrono::system_clock::now();
                auto runDuration = endRun - startRun;
                int64_t runMs = std::chrono::duration_cast<std::chrono::milliseconds>(runDuration).count();
                AddRunTime(runMs);
                if (GetGlobalFlag(GlobalFlags::verbose))
                {
                    std::cout << "program exited normally" << std::endl;
                }
            }
            else
            {
                if (mainFun->ReturnType() == symbolTable.GetType(U"System.Int32"))
                {
                    typedef int32_t(*MainFunctionType)(void);
                    MainFunctionType main = reinterpret_cast<MainFunctionType>(mainEntryPointAddress);
                    GetCurrentThread().SetState(ThreadState::running);
                    auto startRun = std::chrono::system_clock::now();
                    {
                        ThreadExitSetter exitSetter(GetMachine().MainThread());
                        returnValue = main();
                    }
                    auto endRun = std::chrono::system_clock::now();
                    auto runDuration = endRun - startRun;
                    int64_t runMs = std::chrono::duration_cast<std::chrono::milliseconds>(runDuration).count();
                    AddRunTime(runMs);
                    if (GetGlobalFlag(GlobalFlags::verbose))
                    {
                        std::cout << "program returned exit code " << returnValue << std::endl;
                    }
                }
                else
                {
                    throw std::runtime_error("main function has invalid return type (not int)");
                }
            }
        }
    }
    catch (const CminorException& ex)
    {
        std::string errorMessage;
        ObjectReference exceptionObjectReference(ex.ExceptionObjectReference());
        if (exceptionObjectReference.IsNull())
        {
            throw std::runtime_error("unhandled exception escaped from main (exception object is null)");
        }
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        void* exceptionObject = memoryPool.GetObject(exceptionObjectReference);
        ManagedAllocationHeader* header = GetAllocationHeader(exceptionObject);
        ObjectHeader* objectHeader = &header->objectHeader;
        Type* type = objectHeader->GetType();
        errorMessage.append(ToUtf8(type->Name().Value()));
        if (memoryPool.GetFieldCount(exceptionObjectReference) < 2)
        {
            throw std::runtime_error("unhandled exception escaped from main (exception object '" + ToUtf8(type->Name().Value()) + "' has fewer than two fields)");
        }
        IntegralValue messageValue = memoryPool.GetField(exceptionObjectReference, 1);
        if (messageValue.GetType() != ValueType::objectReference)
        {
            throw std::runtime_error("unhandled exception escaped from main (message field of exception object '" + ToUtf8(type->Name().Value()) + "' not of object type)");
        }
        ObjectReference messageReference(messageValue.Value());
        if (!messageReference.IsNull())
        {
            std::string message = GetManagedMemoryPool().GetUtf8String(messageReference);
            if (!message.empty())
            {
                errorMessage.append(": ").append(message);
            }
        }
        IntegralValue stackTraceValue = memoryPool.GetField(exceptionObjectReference, 2);
        if (stackTraceValue.GetType() != ValueType::objectReference)
        {
            throw std::runtime_error("unhandled exception escaped from main (stack trace field of exception object '" + ToUtf8(type->Name().Value()) + "' not of object type)");
        }
        ObjectReference stackTraceReference(stackTraceValue.Value());
        if (!stackTraceReference.IsNull())
        {
            std::string stackTrace = GetManagedMemoryPool().GetUtf8String(stackTraceReference);
            if (!stackTrace.empty())
            {
                errorMessage.append("\n").append(stackTrace);
            }
        }
        throw std::runtime_error("unhandled exception escaped from main: " + errorMessage);
    }
    return returnValue;
}

void Assembly::Dump(CodeFormatter& formatter, DumpOptions dumpOptions)
{
    if ((dumpOptions & DumpOptions::header) != DumpOptions::none)
    {
        DumpHeader(formatter);
    }
    if ((dumpOptions & DumpOptions::constants) != DumpOptions::none)
    {
        constantPool.Dump(formatter);
    }
    if ((dumpOptions & DumpOptions::functions) != DumpOptions::none)
    {
        machineFunctionTable.Dump(formatter);
    }
    if ((dumpOptions & DumpOptions::symbols) != DumpOptions::none)
    {
        symbolTable.Dump(formatter);
    }
    if ((dumpOptions & DumpOptions::mappings) != DumpOptions::none)
    {
        DumpMappings(formatter);
    }
}

void Assembly::DumpHeader(CodeFormatter& formatter)
{
    formatter.WriteLine("ASSEMBLY HEADER");
    formatter.WriteLine();
    formatter.WriteLine("assembly name: " + ToUtf8(Name().Value()));
    formatter.WriteLine("assembly hash: " + hash);
    formatter.WriteLine("assembly format: " + CharStr(char(assemblyFormat)));
    formatter.WriteLine("assembly flags: " + AssemblyFlagsStr(flags));
    formatter.WriteLine("original file path: " + originalFilePath);
    formatter.WriteLine("file path read from: " + filePathReadFrom);
    if (IsNative())
    {
        formatter.WriteLine("native target triple: " + nativeTargetTriple);
        formatter.WriteLine("native shared library file name: " + nativeSharedLibraryFileName);
        formatter.WriteLine("native import library file name: " + nativeImportLibraryFileName);
    }
    formatter.WriteLine("assembly dependencies:");
    formatter.IncIndent();
    formatter.WriteLine(ToUtf8(Name().Value()));
    assemblyDependency.Dump(formatter);
    formatter.DecIndent();
    formatter.WriteLine("referenced assemblies: ");
    formatter.IncIndent();
    for (const std::unique_ptr<Assembly>& referencedAssembly : referencedAssemblies)
    {
        formatter.WriteLine(ToUtf8(referencedAssembly->Name().Value()));
        formatter.IncIndent();
        formatter.WriteLine("original file path: " + referencedAssembly->OriginalFilePath());
        formatter.WriteLine("file path read from: " + referencedAssembly->FilePathReadFrom());
        formatter.DecIndent();
    }
    formatter.DecIndent();
    formatter.WriteLine();
}

void Assembly::DumpMappings(CodeFormatter& formatter)
{
    formatter.WriteLine("ASSEMBLY MAPPINGS");
    formatter.WriteLine();
    if (!exportedFunctions.empty())
    {
        formatter.WriteLine("native exported functions:");
        formatter.IncIndent();
        for (Constant exportedFunctionName : exportedFunctions)
        {
            formatter.WriteLine(ToUtf8(exportedFunctionName.Value().AsStringLiteral()));
        }
        formatter.DecIndent();
    }
    if (!functionPtrVarMappings.empty())
    {
        formatter.WriteLine("native function pointer variable mappings:");
        formatter.IncIndent();
        for (const FunctionPtrVarFunctionName& m : functionPtrVarMappings)
        {
            formatter.WriteLine(ToUtf8(m.functionPtrVarName.Value().AsStringLiteral()) + " : " + ToUtf8(m.functionName.Value().AsStringLiteral()));
        }
        formatter.DecIndent();
    }
    if (!classDatatPtrVarMappings.empty())
    {
        formatter.WriteLine("native class data pointer variable mappings:");
        formatter.IncIndent();
        for (const ClassDataPtrVarClassDataName& m : classDatatPtrVarMappings)
        {
            formatter.WriteLine(ToUtf8(m.classDataPtrVarName.Value().AsStringLiteral()) + " : " + ToUtf8(m.classDataName.Value().AsStringLiteral()));
        }
        formatter.DecIndent();
    }
    if (!typePtrVarMappings.empty())
    {
        formatter.WriteLine("native type pointer variable mappings:");
        formatter.IncIndent();
        for (const TypePtrVarTypeName& m : typePtrVarMappings)
        {
            formatter.WriteLine(ToUtf8(m.typePtrVarName.Value().AsStringLiteral()) + " : " + ToUtf8(m.typeName.Value().AsStringLiteral()));
        }
        formatter.DecIndent();
    }
    formatter.WriteLine();
}

void Assembly::Write(SymbolWriter& writer)
{
    AssemblyTag tag;
    tag.Write(writer);
    writer.SetAssembly(this);
    writer.AsMachineWriter().Put(assemblyFormat);
    writer.AsMachineWriter().Put(uint8_t(flags));
    writer.AsMachineWriter().Put(originalFilePath);
    writer.AsMachineWriter().Put(nativeTargetTriple);
    writer.AsMachineWriter().Put(nativeSharedLibraryFileName);
    writer.AsMachineWriter().Put(nativeImportLibraryFileName);
    writer.AsMachineWriter().Put(hash);
    uint32_t n = uint32_t(referencedAssemblies.size());
    writer.AsMachineWriter().PutEncodedUInt(n);
    for (uint32_t i = 0; i < n; ++i)
    {
        const std::unique_ptr<Assembly>& referencedAssembly = referencedAssemblies[i];
        writer.AsMachineWriter().Put(referencedAssembly->OriginalFilePath());
    }
    constantPool.Write(writer);
    writer.SetConstantPool(&constantPool);
    ConstantId nameId = constantPool.GetIdFor(name);
    Assert(nameId != noConstantId, "constant id for name constant not found");
    nameId.Write(writer);
    machineFunctionTable.Write(writer);
    symbolTable.Write(writer);
    WriteExportedFunctions(writer);
    WriteFunctionVarMappings(writer);
    WriteClassDataVarMappings(writer);
    WriteTypePtrVarMappings(writer);
}

void Assembly::BeginRead(SymbolReader& reader, LoadType loadType, const Assembly* rootAssembly, const std::string& currentAssemblyDir, std::unordered_set<std::string>& importSet,
    std::vector<CallInst*>& callInstructions, std::vector<Fun2DlgInst*>& fun2DlgInstructions,
    std::vector<MemFun2ClassDlgInst*>& memFun2ClassDlgInstructions, std::vector<TypeInstruction*>& typeInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions, 
    std::vector<ClassTypeSymbol*>& classTypeSymbols, std::unordered_set<utf32_string>& classTemplateSpecializationNames, std::vector<Assembly*>& assemblies,
    std::unordered_map<std::string, AssemblyDependency*>& dependencyMap, std::unordered_map<std::string, Assembly*>& readMap)
{
    reader.SetMachine(machine);
    AssemblyTag defaultTag;
    AssemblyTag tagRead;
    tagRead.Read(reader);
    if (tagRead != defaultTag)
    {
        throw std::runtime_error("Invalid assembly tag read from file '" + reader.FilePath() + "'. (Not a Cminor assembly file?)");
    }
    assemblyFormat = reader.GetByte();
    if (assemblyFormat != currentAssemblyFormat)
    {
        throw std::runtime_error("Assembly format mismatch reading from file '" + reader.FilePath() + "'. " +
            "Required assembly format: " + CharStr(char(currentAssemblyFormat)) + ". File assembly format: " + CharStr(char(assemblyFormat)) + ".");
    }
    reader.SetAssembly(this);
    flags = AssemblyFlags(reader.GetByte());
    originalFilePath = reader.GetUtf8String();
    if (dependencyMap.find(originalFilePath) == dependencyMap.cend())
    {
        assemblies.push_back(this);
        dependencyMap[originalFilePath] = &assemblyDependency;
    }
    filePathReadFrom = reader.FilePath();
    if (rootAssembly == this)
    {
        if ((flags & AssemblyFlags::release) != AssemblyFlags::none)
        {
            SetGlobalFlag(GlobalFlags::release);
        }
    }
    else
    {
        if (GetGlobalFlag(GlobalFlags::release) && (flags & AssemblyFlags::release) == AssemblyFlags::none)
        {
            throw std::runtime_error("root assembly '" + ToUtf8(rootAssembly->Name().Value()) + "' (" + rootAssembly->FilePathReadFrom() + ") is a release mode assembly, but assembly '" +
                ToUtf8(Name().Value()) + "' (" + filePathReadFrom + ") being read is not a release mode assembly.");
        }
    }
    nativeTargetTriple = reader.GetUtf8String();
    nativeSharedLibraryFileName = reader.GetUtf8String();
    nativeImportLibraryFileName = reader.GetUtf8String();
    hash = reader.GetUtf8String();
    uint32_t n = reader.GetEncodedUInt();
    for (uint32_t i = 0; i < n; ++i)
    {
        std::string referenceFilePath = reader.GetUtf8String();
        referenceFilePaths.push_back(referenceFilePath);
    }
    constantPool.Read(reader);
    reader.SetConstantPool(&constantPool);
    ConstantId nameId;
    nameId.Read(reader);
    name = constantPool.GetConstant(nameId);
    machineFunctionTable.Read(reader);
    finishReadPos = reader.Pos();
    ImportAssemblies(loadType, rootAssembly, currentAssemblyDir, importSet, callInstructions, fun2DlgInstructions, memFun2ClassDlgInstructions, typeInstructions, 
        setClassDataInstructions, classTypeSymbols, classTemplateSpecializationNames, assemblies, dependencyMap, readMap);
    callInstructions.insert(callInstructions.end(), reader.CallInstructions().cbegin(), reader.CallInstructions().cend());
    fun2DlgInstructions.insert(fun2DlgInstructions.end(), reader.Fun2DlgInstructions().cbegin(), reader.Fun2DlgInstructions().cend());
    memFun2ClassDlgInstructions.insert(memFun2ClassDlgInstructions.end(), reader.MemFun2ClassDlgInstructions().cbegin(), reader.MemFun2ClassDlgInstructions().cend());
    typeInstructions.insert(typeInstructions.end(), reader.TypeInstructions().cbegin(), reader.TypeInstructions().cend());
    setClassDataInstructions.insert(setClassDataInstructions.end(), reader.SetClassDataInstructions().cbegin(), reader.SetClassDataInstructions().cend());
    classTypeSymbols.insert(classTypeSymbols.end(), reader.ClassTypeSymbols().cbegin(), reader.ClassTypeSymbols().cend());
}

void Assembly::FinishReads(std::vector<CallInst*>& callInstructions, std::vector<Fun2DlgInst*>& fun2DlgInstructions,
    std::vector<MemFun2ClassDlgInst*>& memFun2ClassDlgInstructions, std::vector<TypeInstruction*>& typeInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions, 
    std::vector<ClassTypeSymbol*>& classTypeSymbols, std::unordered_set<utf32_string>& classTemplateSpecializationNames, int prevAssemblyIndex, const std::vector<Assembly*>& finishReadOrder, 
    bool readSymbolTable)
{
    Assembly* prevAssembly = nullptr;
    if (prevAssemblyIndex >= 0)
    {
        prevAssembly = finishReadOrder[prevAssemblyIndex];
    }
    if (prevAssembly)
    {
        prevAssembly->FinishReads(callInstructions, fun2DlgInstructions, memFun2ClassDlgInstructions, typeInstructions, setClassDataInstructions, classTypeSymbols, 
            classTemplateSpecializationNames, prevAssemblyIndex - 1, finishReadOrder, readSymbolTable);
        symbolTable.Import(prevAssembly->symbolTable);
    }
    if (prevAssemblyIndex != int(finishReadOrder.size() - 2) || readSymbolTable)
    {
        SymbolReader reader(filePathReadFrom);
        reader.SetAssembly(this);
        reader.SetConstantPool(&constantPool);
        reader.SetClassTemplateSpecializationNames(&classTemplateSpecializationNames);
        reader.Skip(finishReadPos);
        symbolTable.Read(reader);
        ReadExportedFunctions(reader);
        ReadFunctionVarMappings(reader);
        ReadClasDataVarMappings(reader);
        ReadTypePtrVarMappings(reader);
        callInstructions.insert(callInstructions.end(), reader.CallInstructions().cbegin(), reader.CallInstructions().cend());
        fun2DlgInstructions.insert(fun2DlgInstructions.end(), reader.Fun2DlgInstructions().cbegin(), reader.Fun2DlgInstructions().cend());
        memFun2ClassDlgInstructions.insert(memFun2ClassDlgInstructions.end(), reader.MemFun2ClassDlgInstructions().cbegin(), reader.MemFun2ClassDlgInstructions().cend());
        typeInstructions.insert(typeInstructions.end(), reader.TypeInstructions().cbegin(), reader.TypeInstructions().cend());
        setClassDataInstructions.insert(setClassDataInstructions.end(), reader.SetClassDataInstructions().cbegin(), reader.SetClassDataInstructions().cend());
        classTypeSymbols.insert(classTypeSymbols.end(), reader.ClassTypeSymbols().cbegin(), reader.ClassTypeSymbols().cend());
    }
}

void Assembly::Read(SymbolReader& reader, LoadType loadType, const Assembly* rootAssembly, const std::string& currentAssemblyDir, std::unordered_set<std::string>& importSet, 
    std::vector<CallInst*>& callInstructions, std::vector<Fun2DlgInst*>& fun2DlgInstructions,
    std::vector<MemFun2ClassDlgInst*>& memFun2ClassDlgInstructions, std::vector<TypeInstruction*>& typeInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions, 
    std::vector<ClassTypeSymbol*>& classTypeSymbols, std::unordered_set<utf32_string>& classTemplateSpecializationNames, std::vector<Assembly*>& assemblies, 
    std::unordered_map<std::string, AssemblyDependency*>& dependencyMap, std::unordered_map<std::string, Assembly*>& readMap)
{
    reader.SetMachine(machine);
    reader.SetClassTemplateSpecializationNames(&classTemplateSpecializationNames);
    AssemblyTag defaultTag;
    AssemblyTag tagRead;
    tagRead.Read(reader);
    if (tagRead != defaultTag)
    {
        throw std::runtime_error("Invalid assembly tag read from file '" + reader.FilePath() + "'. (Not a Cminor assembly file?)");
    }
    assemblyFormat = reader.GetByte();
    if (assemblyFormat != currentAssemblyFormat)
    {
        throw std::runtime_error("Assembly format mismatch reading from file '" + reader.FilePath() + "'. " +
            "Required assembly format: " + CharStr(char(currentAssemblyFormat)) + ". File assembly format: " + CharStr(char(assemblyFormat)) + ".");
    }
    flags = AssemblyFlags(reader.GetByte());
    reader.SetAssembly(this);
    originalFilePath = reader.GetUtf8String();
    if (dependencyMap.find(originalFilePath) == dependencyMap.cend())
    {
        assemblies.push_back(this);
        dependencyMap[originalFilePath] = &assemblyDependency;
    }
    filePathReadFrom = reader.FilePath();
    if (rootAssembly == this)
    {
        if ((flags & AssemblyFlags::release) != AssemblyFlags::none)
        {
            SetGlobalFlag(GlobalFlags::release);
        }
    }
    else
    {
        if (GetGlobalFlag(GlobalFlags::release) && (flags & AssemblyFlags::release) == AssemblyFlags::none)
        {
            throw std::runtime_error("root assembly '" + ToUtf8(rootAssembly->Name().Value()) + "' (" + rootAssembly->FilePathReadFrom() + ") is a release mode assembly, but assembly '" +
                ToUtf8(Name().Value()) + "' (" + filePathReadFrom + ") being read is not a release mode assembly.");
        }
    }
    nativeTargetTriple = reader.GetUtf8String();
    nativeSharedLibraryFileName = reader.GetUtf8String();
    nativeImportLibraryFileName = reader.GetUtf8String();
    hash = reader.GetUtf8String();
    uint32_t n = reader.GetEncodedUInt();
    for (uint32_t i = 0; i < n; ++i)
    {
        std::string referenceFilePath = reader.GetUtf8String();
        referenceFilePaths.push_back(referenceFilePath);
    }
    constantPool.Read(reader);
    reader.SetConstantPool(&constantPool);
    ConstantId nameId;
    nameId.Read(reader);
    name = constantPool.GetConstant(nameId);
    machineFunctionTable.Read(reader);
    ImportAssemblies(loadType, rootAssembly, currentAssemblyDir, importSet, callInstructions, fun2DlgInstructions, memFun2ClassDlgInstructions, typeInstructions, 
        setClassDataInstructions, classTypeSymbols, classTemplateSpecializationNames, assemblies, dependencyMap, readMap);
    ImportSymbolTables();
    symbolTable.Read(reader);
    ReadExportedFunctions(reader);
    ReadFunctionVarMappings(reader);
    ReadClasDataVarMappings(reader);
    ReadTypePtrVarMappings(reader);
    callInstructions.insert(callInstructions.end(), reader.CallInstructions().cbegin(), reader.CallInstructions().cend());
    fun2DlgInstructions.insert(fun2DlgInstructions.end(), reader.Fun2DlgInstructions().cbegin(), reader.Fun2DlgInstructions().cend());
    memFun2ClassDlgInstructions.insert(memFun2ClassDlgInstructions.end(), reader.MemFun2ClassDlgInstructions().cbegin(), reader.MemFun2ClassDlgInstructions().cend());
    typeInstructions.insert(typeInstructions.end(), reader.TypeInstructions().cbegin(), reader.TypeInstructions().cend());
    setClassDataInstructions.insert(setClassDataInstructions.end(), reader.SetClassDataInstructions().cbegin(), reader.SetClassDataInstructions().cend());
    classTypeSymbols.insert(classTypeSymbols.end(), reader.ClassTypeSymbols().cbegin(), reader.ClassTypeSymbols().cend());
};

void Assembly::SetNativeTargetTriple(const std::string& nativeTargetTriple_)
{
    nativeTargetTriple = nativeTargetTriple_;
}

void Assembly::SetNativeSharedLibraryFileName(const std::string& nativeSharedLibraryFileName_)
{
    nativeSharedLibraryFileName = nativeSharedLibraryFileName_;
}

std::string Assembly::NativeSharedLibraryFilePath() const
{
    return Path::Combine(Path::GetDirectoryName(filePathReadFrom), nativeSharedLibraryFileName);
}

void Assembly::SetNativeImportLibraryFileName(const std::string& nativeImportLibraryFileName_)
{
    nativeImportLibraryFileName = nativeImportLibraryFileName_;
}

void Assembly::ImportAssemblies(LoadType loadType, const Assembly* rootAssembly, const std::string& currentAssemblyDir, std::unordered_set<std::string>& importSet, 
    std::vector<CallInst*>& callInstructions, std::vector<Fun2DlgInst*>& fun2DlgInstructions,
    std::vector<MemFun2ClassDlgInst*>& memFun2ClassDlgInstructions, std::vector<TypeInstruction*>& typeInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions, 
    std::vector<ClassTypeSymbol*>& classTypeSymbols, std::unordered_set<utf32_string>& classTemplateSpecializationNames, std::vector<Assembly*>& assemblies, 
    std::unordered_map<std::string, AssemblyDependency*>& dependencyMap, std::unordered_map<std::string, Assembly*>& readMap)
{
    ImportAssemblies(referenceFilePaths, loadType, rootAssembly, currentAssemblyDir, importSet, callInstructions, fun2DlgInstructions, memFun2ClassDlgInstructions, 
        typeInstructions, setClassDataInstructions, classTypeSymbols, classTemplateSpecializationNames, assemblies, dependencyMap, readMap);
}

void Assembly::ImportSymbolTables()
{
    for (const std::unique_ptr<Assembly>& referencedAssembly : referencedAssemblies)
    {
        symbolTable.Import(referencedAssembly->symbolTable);
    }
}

void Assembly::ImportAssemblies(const std::vector<std::string>& assemblyReferences, LoadType loadType, const Assembly* rootAssembly, const std::string& currentAssemblyDir, 
    std::unordered_set<std::string>& importSet, std::vector<CallInst*>& callInstructions, std::vector<Fun2DlgInst*>& fun2DlgInstructions,
    std::vector<MemFun2ClassDlgInst*>& memFun2ClassDlgInstructions, std::vector<TypeInstruction*>& typeInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions, 
    std::vector<ClassTypeSymbol*>& classTypeSymbols, std::unordered_set<utf32_string>& classTemplateSpecializationNames, std::vector<Assembly*>& assemblies, 
    std::unordered_map<std::string, AssemblyDependency*>& dependencyMap, std::unordered_map<std::string, Assembly*>& readMap)
{
    std::vector<std::string> allAssemblyReferences;
    allAssemblyReferences.insert(allAssemblyReferences.end(), assemblyReferences.cbegin(), assemblyReferences.cend());
    if (!IsSystemAssembly())
    {
        allAssemblyReferences.push_back(CminorSystemAssemblyFilePath(GetConfig()));
    }
    Import(allAssemblyReferences, loadType, rootAssembly, importSet, currentAssemblyDir, callInstructions, fun2DlgInstructions, memFun2ClassDlgInstructions, typeInstructions, 
        setClassDataInstructions, classTypeSymbols, classTemplateSpecializationNames, assemblies, dependencyMap, readMap);
}

void Assembly::Import(const std::vector<std::string>& assemblyReferences, LoadType loadType, const Assembly* rootAssembly, std::unordered_set<std::string>& importSet, 
    const std::string& currentAssemblyDir, std::vector<CallInst*>& callInstructions, std::vector<Fun2DlgInst*>& fun2DlgInstructions,
    std::vector<MemFun2ClassDlgInst*>& memFun2ClassDlgInstructions, std::vector<TypeInstruction*>& typeInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions,
    std::vector<ClassTypeSymbol*>& classTypeSymbols, std::unordered_set<utf32_string>& classTemplateSpecializationNames, std::vector<Assembly*>& assemblies, 
    std::unordered_map<std::string, AssemblyDependency*>& dependencyMap, std::unordered_map<std::string, Assembly*>& readMap)
{
    for (const std::string& assemblyReference : assemblyReferences)
    {
        if (importSet.find(assemblyReference) == importSet.cend())
        {
            importSet.insert(assemblyReference);
            std::unique_ptr<Assembly> referencedAssembly(new Assembly(machine));
            AssemblyTable::Instance().AddAssembly(referencedAssembly.get());
            std::string config = GetConfig();
            boost::filesystem::path afn = boost::filesystem::path(assemblyReference).filename();
            boost::filesystem::path afp;
            std::string searchedDirectories;
            if (loadType == LoadType::build)
            {
                if (!rootAssembly->IsSystemAssembly())
                {
                    afp = CminorSystemAssemblyDir(config);
                    searchedDirectories.append("\n").append(afp.generic_string());
                    afp /= afn;
                    if (!boost::filesystem::exists(afp))
                    {
                        afp = assemblyReference;
                        if (!boost::filesystem::exists(afp))
                        {
                            boost::filesystem::path ard = afp;
                            ard.remove_filename();
                            searchedDirectories.append("\n").append(ard.generic_string());
                            throw std::runtime_error("Could not find assembly reference '" + afn.generic_string() + "'.\nDirectories searched:\n" + searchedDirectories);
                        }
                    }
                }
                else
                {
                    afp = assemblyReference;
                    if (!boost::filesystem::exists(afp))
                    {
                        boost::filesystem::path ard = afp;
                        ard.remove_filename();
                        searchedDirectories.append(ard.generic_string());
                        throw std::runtime_error("Could not find assembly reference '" + afn.generic_string() + "'.\nDirectories searched:\n" + searchedDirectories);
                    }
                }
            }
            else if (loadType == LoadType::execute)
            {
                afp = currentAssemblyDir;
                searchedDirectories = currentAssemblyDir;
                afp /= afn;
                if (!boost::filesystem::exists(afp))
                {
                    if (!rootAssembly->IsSystemAssembly())
                    {
                        afp = CminorSystemAssemblyDir(config);
                        searchedDirectories.append("\n").append(afp.generic_string());
                        afp /= afn;
                        if (!boost::filesystem::exists(afp))
                        {
                            afp = assemblyReference;
                            if (!boost::filesystem::exists(afp))
                            {
                                boost::filesystem::path ard = afp;
                                ard.remove_filename();
                                searchedDirectories.append("\n").append(ard.generic_string());
                                throw std::runtime_error("Could not find assembly reference '" + afn.generic_string() + "'.\nDirectories searched:\n" + searchedDirectories);
                            }
                        }
                    }
                    else
                    {
                        afp = assemblyReference;
                        if (!boost::filesystem::exists(afp))
                        {
                            boost::filesystem::path ard = afp;
                            ard.remove_filename();
                            searchedDirectories.append("\n").append(ard.generic_string());
                            throw std::runtime_error("Could not find assembly reference '" + afn.generic_string() + "'.\nDirectories searched:\n" + searchedDirectories);
                        }
                    }
                }
            }
            std::string assemblyFilePath = GetFullPath(afp.generic_string());
            readMap[assemblyFilePath] = referencedAssembly.get();
            importSet.insert(assemblyFilePath);
            SymbolReader reader(assemblyFilePath);
            reader.SetMachine(machine);
            referencedAssembly->BeginRead(reader, loadType, rootAssembly, currentAssemblyDir, importSet, callInstructions, fun2DlgInstructions, memFun2ClassDlgInstructions, 
                typeInstructions, setClassDataInstructions, classTypeSymbols, classTemplateSpecializationNames, assemblies, dependencyMap, readMap);
            Assembly* referencedAssemblyPtr = referencedAssembly.get();
            assemblyDependency.AddReferencedAssembly(referencedAssemblyPtr);
            referencedAssemblies.push_back(std::move(referencedAssembly));
            Import(referencedAssemblyPtr->referenceFilePaths, loadType, rootAssembly, importSet, currentAssemblyDir, callInstructions, fun2DlgInstructions, memFun2ClassDlgInstructions, 
                typeInstructions, setClassDataInstructions, classTypeSymbols, classTemplateSpecializationNames, assemblies, dependencyMap, readMap);
        }
        else
        {
            std::string config = GetConfig();
            boost::filesystem::path afn = boost::filesystem::path(assemblyReference).filename();
            boost::filesystem::path afp;
            std::string searchedDirectories;
            if (loadType == LoadType::build)
            {
                if (!rootAssembly->IsSystemAssembly())
                {
                    afp = CminorSystemAssemblyDir(config);
                    afp /= afn;
                    if (!boost::filesystem::exists(afp))
                    {
                        afp = assemblyReference;
                        if (!boost::filesystem::exists(afp))
                        {
                            boost::filesystem::path ard = afp;
                            ard.remove_filename();
                            searchedDirectories.append("\n").append(ard.generic_string());
                            throw std::runtime_error("Could not find assembly reference '" + afn.generic_string() + "'.\nDirectories searched:\n" + searchedDirectories);
                        }
                    }
                }
                else
                {
                    afp = assemblyReference;
                    if (!boost::filesystem::exists(afp))
                    {
                        boost::filesystem::path ard = afp;
                        ard.remove_filename();
                        searchedDirectories.append(ard.generic_string());
                        throw std::runtime_error("Could not find assembly reference '" + afn.generic_string() + "'.\nDirectories searched:\n" + searchedDirectories);
                    }
                }
            }
            else if (loadType == LoadType::execute)
            {
                afp = currentAssemblyDir;
                searchedDirectories = currentAssemblyDir;
                afp /= afn;
                if (!boost::filesystem::exists(afp))
                {
                    if (!rootAssembly->IsSystemAssembly())
                    {
                        afp = CminorSystemAssemblyDir(config);
                        searchedDirectories.append("\n").append(afp.generic_string());
                        afp /= afn;
                        if (!boost::filesystem::exists(afp))
                        {
                            afp = assemblyReference;
                            if (!boost::filesystem::exists(afp))
                            {
                                boost::filesystem::path ard = afp;
                                ard.remove_filename();
                                searchedDirectories.append("\n").append(ard.generic_string());
                                throw std::runtime_error("Could not find assembly reference '" + afn.generic_string() + "'.\nDirectories searched:\n" + searchedDirectories);
                            }
                        }
                    }
                    else
                    {
                        afp = assemblyReference;
                        if (!boost::filesystem::exists(afp))
                        {
                            boost::filesystem::path ard = afp;
                            ard.remove_filename();
                            searchedDirectories.append("\n").append(ard.generic_string());
                            throw std::runtime_error("Could not find assembly reference '" + afn.generic_string() + "'.\nDirectories searched:\n" + searchedDirectories);
                        }
                    }
                }
            }
            std::string assemblyFilePath = GetFullPath(afp.generic_string());
            auto it = readMap.find(assemblyFilePath);
            if (it != readMap.cend())
            {
                Assembly* referencedAssembly = it->second;
                assemblyDependency.AddReferencedAssembly(referencedAssembly);
            }
            else
            {
                throw std::runtime_error("assembly file path '" + assemblyFilePath + "' not found from assembly read map for assembly " + ToUtf8(Name().Value()));
            }
        }
    }
}

Constant Assembly::RegisterSourceFilePath(const std::string& sourceFilePath)
{
    utf32_string sfp = ToUtf32(sourceFilePath);
    return constantPool.GetConstant(constantPool.Install(StringPtr(sfp.c_str())));
}

void Assembly::AddExportedFunction(Constant exportedFunction)
{
    exportedFunctions.push_back(exportedFunction);
}

void Assembly::AddFunctionVarMapping(Constant functionPtrVarName, Constant functionName)
{
    functionPtrVarMappings.push_back(FunctionPtrVarFunctionName(functionPtrVarName, functionName));
}

void Assembly::AddClassDataVarMapping(Constant classDataPtrVarName, Constant classDataName)
{
    classDatatPtrVarMappings.push_back(ClassDataPtrVarClassDataName(classDataPtrVarName, classDataName));
}

void Assembly::AddTypePtrVarMapping(Constant typePtrVarName, Constant typeName)
{
    typePtrVarMappings.push_back(TypePtrVarTypeName(typePtrVarName, typeName));
}

void Assembly::WriteExportedFunctions(Writer& writer)
{
    uint32_t n = uint32_t(exportedFunctions.size());
    writer.PutEncodedUInt(n);
    for (uint32_t i = 0; i < n; ++i)
    {
        Constant exportedFunction = exportedFunctions[i];
        ConstantId exportedFunctionId = constantPool.GetIdFor(exportedFunction);
        exportedFunctionId.Write(writer);
    }
}

void Assembly::ReadExportedFunctions(Reader& reader)
{
    uint32_t n = reader.GetEncodedUInt();
    for (uint32_t i = 0; i < n; ++i)
    {
        ConstantId exportedFunctionId;
        exportedFunctionId.Read(reader);
        Constant exportedFunction = constantPool.GetConstant(exportedFunctionId);
        exportedFunctions.push_back(exportedFunction);
    }
}

void Assembly::WriteFunctionVarMappings(Writer& writer)
{
    uint32_t n = uint32_t(functionPtrVarMappings.size());
    writer.PutEncodedUInt(n);
    for (uint32_t i = 0; i < n; ++i)
    {
        const FunctionPtrVarFunctionName& s = functionPtrVarMappings[i];
        ConstantId functionPtrVarId = constantPool.GetIdFor(s.functionPtrVarName);
        functionPtrVarId.Write(writer);
        ConstantId functionNameId = constantPool.GetIdFor(s.functionName);
        functionNameId.Write(writer);
    }
}

void Assembly::ReadFunctionVarMappings(Reader& reader)
{
    uint32_t n = reader.GetEncodedUInt();
    for (uint32_t i = 0; i < n; ++i)
    {
        ConstantId functionPtrVarId;
        functionPtrVarId.Read(reader);
        Constant functionPtrVarName = constantPool.GetConstant(functionPtrVarId);
        ConstantId functionNameId;
        functionNameId.Read(reader);
        Constant functionName = constantPool.GetConstant(functionNameId);
        functionPtrVarMappings.push_back(FunctionPtrVarFunctionName(functionPtrVarName, functionName));
    }
}

void Assembly::WriteClassDataVarMappings(Writer& writer)
{
    uint32_t n = uint32_t(classDatatPtrVarMappings.size());
    writer.PutEncodedUInt(n);
    for (uint32_t i = 0; i < n; ++i)
    {
        const ClassDataPtrVarClassDataName& s = classDatatPtrVarMappings[i];
        ConstantId classDataPtrVarId = constantPool.GetIdFor(s.classDataPtrVarName);
        classDataPtrVarId.Write(writer);
        ConstantId classDataNameId = constantPool.GetIdFor(s.classDataName);
        classDataNameId.Write(writer);
    }
}

void Assembly::ReadClasDataVarMappings(Reader& reader)
{
    uint32_t n = reader.GetEncodedUInt();
    for (uint32_t i = 0; i < n; ++i)
    {
        ConstantId classDataPtrVarId;
        classDataPtrVarId.Read(reader);
        Constant classDataPtrVarName = constantPool.GetConstant(classDataPtrVarId);
        ConstantId classDataNameId;
        classDataNameId.Read(reader);
        Constant classDataName = constantPool.GetConstant(classDataNameId);
        classDatatPtrVarMappings.push_back(ClassDataPtrVarClassDataName(classDataPtrVarName, classDataName));
    }
}

void Assembly::WriteTypePtrVarMappings(Writer& writer)
{
    uint32_t n = uint32_t(typePtrVarMappings.size());
    writer.PutEncodedUInt(n);
    for (uint32_t i = 0; i < n; ++i)
    {
        const TypePtrVarTypeName& s = typePtrVarMappings[i];
        ConstantId typePtrVarId = constantPool.GetIdFor(s.typePtrVarName);
        typePtrVarId.Write(writer);
        ConstantId typeNameId = constantPool.GetIdFor(s.typeName);
        typeNameId.Write(writer);
    }
}

void Assembly::ReadTypePtrVarMappings(Reader& reader)
{
    uint32_t n = reader.GetEncodedUInt();
    for (uint32_t i = 0; i < n; ++i)
    {
        ConstantId typePtrVarId;
        typePtrVarId.Read(reader);
        Constant typePtrVarName = constantPool.GetConstant(typePtrVarId);
        ConstantId typeNameId;
        typeNameId.Read(reader);
        Constant typeName = constantPool.GetConstant(typeNameId);
        typePtrVarMappings.push_back(TypePtrVarTypeName(typePtrVarName, typeName));
    }
}

void LoadSharedLibrariesIntoMemory(Assembly* assembly)
{
    if (assembly->IsCore()) return;
    if (assembly->SharedLibraryLoaded()) return;
    assembly->SetSharedLibraryLoaded();
    if (assembly->NativeSharedLibraryFileName().empty())
    {
        throw std::runtime_error("native shared library file name of assembly '" + ToUtf8(assembly->Name().Value()) + "' (" + assembly->FilePathReadFrom() + ") is empty");
    }
    for (const std::unique_ptr<Assembly>& referencedAssembly : assembly->ReferencedAssemblies())
    {
        LoadSharedLibrariesIntoMemory(referencedAssembly.get());
    }
    std::string sharedLibraryFilePath = assembly->NativeSharedLibraryFilePath();
    try
    {
        void* sharedLibraryHandle = LoadSharedLibrary(sharedLibraryFilePath);
        assembly->SetSharedLibraryHandle(sharedLibraryHandle);
    }
    catch (const std::runtime_error& ex)
    {
        throw std::runtime_error("loading native shared library of assembly '" + ToUtf8(assembly->Name().Value()) + "' (" + assembly->FilePathReadFrom() + ") failed: " + ex.what());
    }
}

void ResolveAddressesOfExportedFunctions(Assembly* assembly)
{
    if (assembly->IsCore()) return;
    if (assembly->ExportedFunctionsResolved()) return;
    assembly->SetExportedFunctionsResolved();
    for (const std::unique_ptr<Assembly>& referencedAssembly : assembly->ReferencedAssemblies())
    {
        ResolveAddressesOfExportedFunctions(referencedAssembly.get());
    }
    void* sharedLibraryHandle = assembly->SharedLibraryHandle();
    if (!sharedLibraryHandle)
    {
        throw std::runtime_error("shared library handle of assembly '" + ToUtf8(assembly->Name().Value()) + "' (" + assembly->FilePathReadFrom() + ") is not set");
    }
    for (Constant exportedFunctionNameConstant : assembly->ExportedFunctions())
    {
        Function* exportedFunction = FunctionTable::GetFunction(StringPtr(exportedFunctionNameConstant.Value().AsStringLiteral()));
        if (exportedFunction->MangledName().empty())
        {
            throw std::runtime_error("mangled name of exported function '" + ToUtf8(exportedFunction->CallName().Value().AsStringLiteral()) + "' of assembly '" +
                ToUtf8(assembly->Name().Value()) + "' (" + assembly->FilePathReadFrom() + ") is empty");
        }
        try
        {
            void* symbolAddress = ResolveSymbolAddress(sharedLibraryHandle, assembly->NativeSharedLibraryFilePath(), exportedFunction->MangledName());
            exportedFunction->SetAddress(symbolAddress);
        }
        catch (const std::runtime_error& ex)
        {
            throw std::runtime_error("resolving address of exported function '" + ToUtf8(exportedFunction->CallName().Value().AsStringLiteral()) + "' of assembly '" + 
                ToUtf8(assembly->Name().Value()) + "' (" + assembly->FilePathReadFrom() + ") failed: " + ex.what());
        }
    }
}

void ResolveFunctionPtrVarMappings(Assembly* assembly)
{
    if (assembly->IsCore()) return;
    if (assembly->FunctionPtrVarMappingsResolved()) return;
    assembly->SetFunctionPtrVarMappingsResolved();
    for (const std::unique_ptr<Assembly>& referencedAssembly : assembly->ReferencedAssemblies())
    {
        ResolveFunctionPtrVarMappings(referencedAssembly.get());
    }
    void* sharedLibraryHandle = assembly->SharedLibraryHandle();
    if (!sharedLibraryHandle)
    {
        throw std::runtime_error("shared library handle of assembly '" + ToUtf8(assembly->Name().Value()) + "' (" + assembly->FilePathReadFrom() + ") is not set");
    }
    for (const FunctionPtrVarFunctionName& mapping : assembly->FunctionPtrVarMappings())
    {
        Constant functionPtrVarName = mapping.functionPtrVarName;
        Constant functionName = mapping.functionName;
        Function* function = FunctionTable::GetFunction(StringPtr(functionName.Value().AsStringLiteral()));
        std::string ptrVarName = ToUtf8(functionPtrVarName.Value().AsStringLiteral());
        try
        {
            void* symbolAddress = ResolveSymbolAddress(sharedLibraryHandle, assembly->NativeSharedLibraryFilePath(), ptrVarName);
            void* functionAddress = static_cast<void*>(function);
            void** functionVariablePtr = reinterpret_cast<void**>(symbolAddress);
            *functionVariablePtr = functionAddress;
        }
        catch (const std::runtime_error& ex)
        {
            throw std::runtime_error("resolving address of function pointer variable for function '" + ToUtf8(functionName.Value().AsStringLiteral()) + "' of assembly '" +
                ToUtf8(assembly->Name().Value()) + "' (" + assembly->FilePathReadFrom() + ") failed: " + ex.what());
        }
    }
}

void ResolveClassDataPtrVarMappings(Assembly* assembly)
{
    if (assembly->IsCore()) return;
    if (assembly->ClassDataPtrVarMappingsResolved()) return;
    assembly->SetClassDataPtrVarMappingsResolved();
    for (const std::unique_ptr<Assembly>& referencedAssembly : assembly->ReferencedAssemblies())
    {
        ResolveClassDataPtrVarMappings(referencedAssembly.get());
    }
    void* sharedLibraryHandle = assembly->SharedLibraryHandle();
    if (!sharedLibraryHandle)
    {
        throw std::runtime_error("shared library handle of assembly '" + ToUtf8(assembly->Name().Value()) + "' (" + assembly->FilePathReadFrom() + ") is not set");
    }
    for (const ClassDataPtrVarClassDataName& mapping : assembly->ClassDataPtrVarMappings())
    {
        Constant classDataPtrVarName = mapping.classDataPtrVarName;
        Constant classDataName = mapping.classDataName;
        ClassData* classData = ClassDataTable::GetClassData(StringPtr(classDataName.Value().AsStringLiteral()));
        std::string ptrVarName = ToUtf8(classDataPtrVarName.Value().AsStringLiteral());
        try
        {
            void* symbolAddress = ResolveSymbolAddress(sharedLibraryHandle, assembly->NativeSharedLibraryFilePath(), ptrVarName);
            void* classDataAddress = static_cast<void*>(classData);
            void** classDataVariablePtr = reinterpret_cast<void**>(symbolAddress);
            *classDataVariablePtr = classDataAddress;
        }
        catch (const std::runtime_error& ex)
        {
            throw std::runtime_error("resolving address of class data pointer variable for class '" + ToUtf8(classDataName.Value().AsStringLiteral()) + "' of assembly '" +
                ToUtf8(assembly->Name().Value()) + "' (" + assembly->FilePathReadFrom() + ") failed: " + ex.what());
        }
    }
}

void ResolveTypePtrVarMappings(Assembly* assembly)
{
    if (assembly->IsCore()) return;
    if (assembly->TypePtrVarMappingsResolved()) return;
    assembly->SetTypePtrVarMappingsResolved();
    for (const std::unique_ptr<Assembly>& referencedAssembly : assembly->ReferencedAssemblies())
    {
        ResolveTypePtrVarMappings(referencedAssembly.get());
    }
    void* sharedLibraryHandle = assembly->SharedLibraryHandle();
    if (!sharedLibraryHandle)
    {
        throw std::runtime_error("shared library handle of assembly '" + ToUtf8(assembly->Name().Value()) + "' (" + assembly->FilePathReadFrom() + ") is not set");
    }
    for (const TypePtrVarTypeName& mapping : assembly->TypePtrVarMappings())
    {
        Constant typePtrVarName = mapping.typePtrVarName;
        Constant typeName = mapping.typeName;
        Type* type = TypeTable::GetType(StringPtr(typeName.Value().AsStringLiteral()));
        std::string ptrVarName = ToUtf8(typePtrVarName.Value().AsStringLiteral());
        try
        {
            void* symbolAddress = ResolveSymbolAddress(sharedLibraryHandle, assembly->NativeSharedLibraryFilePath(), ptrVarName);
            void* typeAddress = static_cast<void*>(type);
            void** typeVariablePtr = reinterpret_cast<void**>(symbolAddress);
            *typeVariablePtr = typeAddress;
        }
        catch (const std::runtime_error& ex)
        {
            throw std::runtime_error("resolving address of type pointer variable for type '" + ToUtf8(typeName.Value().AsStringLiteral()) + "' of assembly '" +
                ToUtf8(assembly->Name().Value()) + "' (" + assembly->FilePathReadFrom() + ") failed: " + ex.what());
        }
    }
}

void SetExportedSharedLibraryVariables(Assembly* assembly)
{
    if (assembly->IsCore()) return;
    if (assembly->ExportedSharedLibraryVariablesSet()) return;
    assembly->SetExportedSharedLibraryVariablesSet();
    for (const std::unique_ptr<Assembly>& referencedAssembly : assembly->ReferencedAssemblies())
    {
        SetExportedSharedLibraryVariables(referencedAssembly.get());
    }
    void* sharedLibraryHandle = assembly->SharedLibraryHandle();
    if (!sharedLibraryHandle)
    {
        throw std::runtime_error("shared library handle of assembly '" + ToUtf8(assembly->Name().Value()) + "' (" + assembly->FilePathReadFrom() + ") is not set");
    }
    std::string constantPoolVarName = "__constant_pool_" + assembly->Hash();
    try
    {
        void* symbolAddress = ResolveSymbolAddress(sharedLibraryHandle, assembly->NativeSharedLibraryFilePath(), constantPoolVarName);
        void* constantPoolAddress = static_cast<void*>(&assembly->GetConstantPool());
        void** constantPoolVariablePtr = reinterpret_cast<void**>(symbolAddress);
        *constantPoolVariablePtr = constantPoolAddress;
    }
    catch (const std::runtime_error& ex)
    {
        throw std::runtime_error("resolving address of '__constant_pool_" + assembly->Hash() + "' variable of assembly '" + ToUtf8(assembly->Name().Value()) + "' (" + assembly->FilePathReadFrom() + ") failed: " + ex.what());
    }
}

void ResolveMainEntryPointAddress(Assembly* assembly)
{
    void* sharedLibraryHandle = assembly->SharedLibraryHandle();
    if (!sharedLibraryHandle)
    {
        throw std::runtime_error("shared library handle of assembly '" + ToUtf8(assembly->Name().Value()) + "' (" + assembly->FilePathReadFrom() + ") is not set");
    }
    std::string mainEntryPointName = "main_entry_point";
    void* mainEntryPointAddress = ResolveSymbolAddress(sharedLibraryHandle, assembly->NativeSharedLibraryFilePath(), mainEntryPointName);
    assembly->SetMainEntryPointAddress(mainEntryPointAddress);
}

void Assembly::PrepareForNativeExecution()
{
    Assert(IsNative(), "not native");
    LoadSharedLibrariesIntoMemory(this);
    ResolveAddressesOfExportedFunctions(this);
    ResolveFunctionPtrVarMappings(this);
    ResolveClassDataPtrVarMappings(this);
    ResolveTypePtrVarMappings(this);
    SetExportedSharedLibraryVariables(this);
    ResolveMainEntryPointAddress(this);
}

int NumberOfAncestors(ClassTypeSymbol* classType)
{
    int numAncestors = 0;
    ClassTypeSymbol* baseClass = classType->BaseClass();
    while (baseClass != nullptr)
    {
        ++numAncestors;
        baseClass = baseClass->BaseClass();
    }
    return numAncestors;
}

struct PriorityGreater
{
    bool operator()(ClassTypeSymbol* left, ClassTypeSymbol* right) const
    {
        if (left->Level() < right->Level()) return true;
        return false;
    }
};

void AssignKeys(std::vector<ClassTypeSymbol*>& classesByPriority)
{
    uint64_t key = 2;
    for (ClassTypeSymbol* cls : classesByPriority)
    {
        cls->SetKey(key);
        key = cminor::util::NextPrime(key + 1);
    }
}

uint64_t ComputeCid(ClassTypeSymbol* classType)
{
    uint64_t cid = classType->Key();
    ClassTypeSymbol* base = classType->BaseClass();
    while (base)
    {
        cid *= base->Key();
        base = base->BaseClass();
    }
    return cid;
}

void AssignCids(std::vector<ClassTypeSymbol*>& classesByPriority)
{
    for (ClassTypeSymbol* classType : classesByPriority)
    {
        classType->SetCid(ComputeCid(classType));
    }
}

void AssignClassTypeIds(const std::vector<ClassTypeSymbol*>& classTypes)
{
    for (ClassTypeSymbol* classType : classTypes)
    {
        if (classType->IsClassDelegateType()) continue;
        classType->SetLevel(NumberOfAncestors(classType));
    }
    std::vector<ClassTypeSymbol*> classesByPriority;
    for (ClassTypeSymbol* classType : classTypes)
    {
        if (classType->IsClassDelegateType()) continue;
        classesByPriority.push_back(classType);
    }
    std::sort(classesByPriority.begin(), classesByPriority.end(), PriorityGreater());
    AssignKeys(classesByPriority);
    AssignCids(classesByPriority);
#ifdef DEBUG_CLASS_IDS
    std::ofstream cls("C:\\Temp\\cls.txt");
    for (ClassTypeSymbol* classType : classesByPriority)
    {
        cls << ToUtf8(classType->FullName()) <<
            " : level = " << classType->Level() <<
            " : key = " << classType->Key() <<
            " : cid = " << classType->Cid() << 
            std::endl;
    }
#endif
}

void Link(const std::vector<CallInst*>& callInstructions, const std::vector<Fun2DlgInst*>& fun2DlgInstructions,
    const std::vector<MemFun2ClassDlgInst*>& memFun2ClassDlgInstructions, const std::vector<TypeInstruction*>& typeInstructions, const std::vector<SetClassDataInst*>& setClassDataInstructions, 
    const std::vector<ClassTypeSymbol*>& classTypes)
{
    for (CallInst* call : callInstructions)
    {
        Function* fun = FunctionTable::GetFunction(call->GetFunctionCallName());
        call->SetFunction(fun);
    }
    for (Fun2DlgInst* fun2Dlg : fun2DlgInstructions)
    {
        Function* fun = FunctionTable::GetFunction(fun2Dlg->GetFunctionName());
        fun2Dlg->SetFunction(fun);
    }
    for (MemFun2ClassDlgInst* memFun2ClassDlgInst : memFun2ClassDlgInstructions)
    {
        Function* fun = FunctionTable::GetFunction(memFun2ClassDlgInst->GetFunctionName());
        memFun2ClassDlgInst->SetFunction(fun);
    }
    for (TypeInstruction* typeInst : typeInstructions)
    {
        Type* type = TypeTable::GetType(typeInst->GetTypeName());
        typeInst->SetType(type);
    }
    for (SetClassDataInst* setClassDataInst : setClassDataInstructions)
    {
        ClassData* cd = ClassDataTable::GetClassData(setClassDataInst->GetClassName());
        setClassDataInst->SetClassData(cd);
    }
    for (ClassTypeSymbol* classType : classTypes)
    {
        classType->LinkVmt();
        classType->LinkImts();
    }
    AssignClassTypeIds(classTypes);
    FunctionTable::ResolveExceptionVarTypes();
}

void AssemblyTable::Init()
{
    instance.reset(new AssemblyTable);
}

void AssemblyTable::Done()
{
    instance.reset();
}

AssemblyTable& AssemblyTable::Instance()
{
    Assert(instance, "assembly table instance not set");
    return *instance;
}

std::unique_ptr<AssemblyTable> AssemblyTable::instance;

void AssemblyTable::AddAssembly(Assembly* assembly)
{
    uint32_t assemblyId = uint32_t(assemblies.size());
    assembly->SetId(assemblyId);
    assemblies.push_back(assembly);
}

Assembly* AssemblyTable::GetAssembly(uint32_t assemblyId) const
{
    Assert(assemblyId < assemblies.size(), "invalid assembly id");
    return assemblies[assemblyId];
}

Assembly* AssemblyTable::GetAssembly(const std::string& assemblyName) const
{
    std::u32string aname = ToUtf32(assemblyName);
    for (Assembly* assembly : assemblies)
    {
        if (assembly->Name() == StringPtr(aname.c_str()))
        {
            return assembly;
        }
    }
    throw std::runtime_error("assembly '" + assemblyName + "' not found");
}

void InitAssembly()
{
    CminorSystemAssemblyNameCollection::Init();
    AssemblyTable::Init();
}

void DoneAssembly()
{
    AssemblyTable::Done();
    CminorSystemAssemblyNameCollection::Done();
}

} } // namespace cminor::symbols
