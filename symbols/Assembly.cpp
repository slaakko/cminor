// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolReader.hpp>
#include <cminor/symbols/GlobalFlags.hpp>
#include <cminor/ast/Project.hpp>
#include <cminor/machine/Class.hpp>
#include <boost/filesystem.hpp>
#include <cminor/machine/Path.hpp>
#include <cminor/machine/Prime.hpp>

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

Assembly::Assembly(Machine& machine_) : machine(machine_), filePath(), constantPool(), symbolTable(this), name(), id(-1), finishReadPos(0), assemblyDependency(this)
{
}

Assembly::Assembly(Machine& machine_, const utf32_string& name_, const std::string& filePath_) : machine(machine_), filePath(filePath_), constantPool(), symbolTable(this),
    name(constantPool.GetConstant(constantPool.Install(StringPtr(name_.c_str())))), id(-1), finishReadPos(0), assemblyDependency(this)
{
}

void Assembly::Write(SymbolWriter& writer)
{
    AssemblyTag tag;
    tag.Write(writer);
    writer.SetAssembly(this);
    writer.AsMachineWriter().Put(filePath);
    uint32_t n = uint32_t(referencedAssemblies.size());
    writer.AsMachineWriter().PutEncodedUInt(n);
    for (uint32_t i = 0; i < n; ++i)
    {
        const std::unique_ptr<Assembly>& referencedAssembly = referencedAssemblies[i];
        writer.AsMachineWriter().Put(referencedAssembly->FilePath());
    }
    constantPool.Write(writer);
    writer.SetConstantPool(&constantPool);
    ConstantId nameId = constantPool.GetIdFor(name);
    Assert(nameId != noConstantId, "constant id for name constant not found");
    nameId.Write(writer);
    machineFunctionTable.Write(writer);
    symbolTable.Write(writer);
    WriteSymbolIdMapping(writer);
}

void Assembly::BeginRead(SymbolReader& reader, LoadType loadType, const Assembly* rootAssembly, const std::string& currentAssemblyDir, std::unordered_set<std::string>& importSet,
    std::vector<CallInst*>& callInstructions, std::vector<Fun2DlgInst*>& fun2DlgInstructions, std::vector<MemFun2ClassDlgInst*>& memFun2ClassDlgInstructions, 
    std::vector<TypeInstruction*>& typeInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions, std::vector<ClassTypeSymbol*>& classTypeSymbols, 
    std::unordered_set<utf32_string>& classTemplateSpecializationNames, std::vector<Assembly*>& assemblies,
    std::unordered_map<std::string, AssemblyDependency*>& dependencyMap)
{
    reader.SetMachine(machine);
    AssemblyTag defaultTag;
    AssemblyTag tagRead;
    tagRead.Read(reader);
    if (tagRead != defaultTag)
    {
        throw std::runtime_error("invalid cminor assembly tag read from file '" + filePath + "'. (not an assembly file?)");
    }
    reader.SetAssembly(this);
    filePath = reader.GetUtf8String();
    if (dependencyMap.find(filePath) == dependencyMap.cend())
    {
        assemblies.push_back(this);
        dependencyMap[filePath] = &assemblyDependency;
    }
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
    ImportAssemblies(loadType, rootAssembly, currentAssemblyDir, importSet, callInstructions, fun2DlgInstructions, memFun2ClassDlgInstructions, typeInstructions, setClassDataInstructions, 
        classTypeSymbols, classTemplateSpecializationNames, assemblies, dependencyMap);
    callInstructions.insert(callInstructions.end(), reader.CallInstructions().cbegin(), reader.CallInstructions().cend());
    fun2DlgInstructions.insert(fun2DlgInstructions.end(), reader.Fun2DlgInstructions().cbegin(), reader.Fun2DlgInstructions().cend());
    memFun2ClassDlgInstructions.insert(memFun2ClassDlgInstructions.end(), reader.MemFun2ClassDlgInstructions().cbegin(), reader.MemFun2ClassDlgInstructions().cend());
    typeInstructions.insert(typeInstructions.end(), reader.TypeInstructions().cbegin(), reader.TypeInstructions().cend());
    setClassDataInstructions.insert(setClassDataInstructions.end(), reader.SetClassDataInstructions().cbegin(), reader.SetClassDataInstructions().cend());
    classTypeSymbols.insert(classTypeSymbols.end(), reader.ClassTypeSymbols().cbegin(), reader.ClassTypeSymbols().cend());
}

void Assembly::FinishReads(std::vector<CallInst*>& callInstructions, std::vector<Fun2DlgInst*>& fun2DlgInstructions, std::vector<MemFun2ClassDlgInst*>& memFun2ClassDlgInstructions, 
    std::vector<TypeInstruction*>& typeInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions, std::vector<ClassTypeSymbol*>& classTypeSymbols, 
    std::unordered_set<utf32_string>& classTemplateSpecializationNames, int prevAssemblyIndex, const std::vector<Assembly*>& finishReadOrder, bool readSymbolTable)
{
    Assembly* prevAssembly = nullptr;
    if (prevAssemblyIndex >= 0)
    {
        prevAssembly = finishReadOrder[prevAssemblyIndex];
    }
    if (prevAssembly)
    {
        prevAssembly->FinishReads(callInstructions, fun2DlgInstructions, memFun2ClassDlgInstructions, typeInstructions, setClassDataInstructions, classTypeSymbols, classTemplateSpecializationNames, 
            prevAssemblyIndex - 1, finishReadOrder, readSymbolTable);
        symbolTable.Import(prevAssembly->symbolTable);
    }
    if (prevAssemblyIndex != int(finishReadOrder.size() - 2) || readSymbolTable)
    {
        SymbolReader reader(filePath);
        reader.SetAssembly(this);
        reader.SetConstantPool(&constantPool);
        reader.SetClassTemplateSpecializationNames(&classTemplateSpecializationNames);
        reader.Skip(finishReadPos);
        symbolTable.Read(reader);
        ReadSymbolIdMapping(reader);
        callInstructions.insert(callInstructions.end(), reader.CallInstructions().cbegin(), reader.CallInstructions().cend());
        fun2DlgInstructions.insert(fun2DlgInstructions.end(), reader.Fun2DlgInstructions().cbegin(), reader.Fun2DlgInstructions().cend());
        memFun2ClassDlgInstructions.insert(memFun2ClassDlgInstructions.end(), reader.MemFun2ClassDlgInstructions().cbegin(), reader.MemFun2ClassDlgInstructions().cend());
        typeInstructions.insert(typeInstructions.end(), reader.TypeInstructions().cbegin(), reader.TypeInstructions().cend());
        setClassDataInstructions.insert(setClassDataInstructions.end(), reader.SetClassDataInstructions().cbegin(), reader.SetClassDataInstructions().cend());
        classTypeSymbols.insert(classTypeSymbols.end(), reader.ClassTypeSymbols().cbegin(), reader.ClassTypeSymbols().cend());
    }
}

void Assembly::Read(SymbolReader& reader, LoadType loadType, const Assembly* rootAssembly, const std::string& currentAssemblyDir, std::unordered_set<std::string>& importSet, 
    std::vector<CallInst*>& callInstructions, std::vector<Fun2DlgInst*>& fun2DlgInstructions, std::vector<MemFun2ClassDlgInst*>& memFun2ClassDlgInstructions, 
    std::vector<TypeInstruction*>& typeInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions, std::vector<ClassTypeSymbol*>& classTypeSymbols, 
    std::unordered_set<utf32_string>& classTemplateSpecializationNames, std::vector<Assembly*>& assemblies, std::unordered_map<std::string, AssemblyDependency*>& dependencyMap)
{
    reader.SetMachine(machine);
    reader.SetClassTemplateSpecializationNames(&classTemplateSpecializationNames);
    AssemblyTag defaultTag;
    AssemblyTag tagRead;
    tagRead.Read(reader);
    if (tagRead != defaultTag)
    {
        throw std::runtime_error("invalid cminor assembly tag read from file '" + filePath + "'. (not an assembly file?)");
    }
    reader.SetAssembly(this);
    filePath = reader.GetUtf8String();
    if (dependencyMap.find(filePath) == dependencyMap.cend())
    {
        assemblies.push_back(this);
        dependencyMap[filePath] = &assemblyDependency;
    }
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
    ImportAssemblies(loadType, rootAssembly, currentAssemblyDir, importSet, callInstructions, fun2DlgInstructions, memFun2ClassDlgInstructions, typeInstructions, setClassDataInstructions, 
        classTypeSymbols, classTemplateSpecializationNames, assemblies, dependencyMap);
    ImportSymbolTables();
    symbolTable.Read(reader);
    ReadSymbolIdMapping(reader);
    callInstructions.insert(callInstructions.end(), reader.CallInstructions().cbegin(), reader.CallInstructions().cend());
    fun2DlgInstructions.insert(fun2DlgInstructions.end(), reader.Fun2DlgInstructions().cbegin(), reader.Fun2DlgInstructions().cend());
    memFun2ClassDlgInstructions.insert(memFun2ClassDlgInstructions.end(), reader.MemFun2ClassDlgInstructions().cbegin(), reader.MemFun2ClassDlgInstructions().cend());
    typeInstructions.insert(typeInstructions.end(), reader.TypeInstructions().cbegin(), reader.TypeInstructions().cend());
    setClassDataInstructions.insert(setClassDataInstructions.end(), reader.SetClassDataInstructions().cbegin(), reader.SetClassDataInstructions().cend());
    classTypeSymbols.insert(classTypeSymbols.end(), reader.ClassTypeSymbols().cbegin(), reader.ClassTypeSymbols().cend());
};

bool Assembly::IsSystemAssembly() const
{
    return CminorSystemAssemblyNameCollection::Instance().Find(StringPtr(name.Value().AsStringLiteral()));
}

void Assembly::ImportAssemblies(LoadType loadType, const Assembly* rootAssembly, const std::string& currentAssemblyDir, std::unordered_set<std::string>& importSet, 
    std::vector<CallInst*>& callInstructions, std::vector<Fun2DlgInst*>& fun2DlgInstructions, std::vector<MemFun2ClassDlgInst*>& memFun2ClassDlgInstructions, 
    std::vector<TypeInstruction*>& typeInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions, std::vector<ClassTypeSymbol*>& classTypeSymbols, 
    std::unordered_set<utf32_string>& classTemplateSpecializationNames, std::vector<Assembly*>& assemblies, std::unordered_map<std::string, AssemblyDependency*>& dependencyMap)
{
    ImportAssemblies(referenceFilePaths, loadType, rootAssembly, currentAssemblyDir, importSet, callInstructions, fun2DlgInstructions, memFun2ClassDlgInstructions, typeInstructions, 
        setClassDataInstructions, classTypeSymbols, classTemplateSpecializationNames, assemblies, dependencyMap);
}

void Assembly::ImportSymbolTables()
{
    for (const std::unique_ptr<Assembly>& referencedAssembly : referencedAssemblies)
    {
        symbolTable.Import(referencedAssembly->symbolTable);
    }
}

void Assembly::ImportAssemblies(const std::vector<std::string>& assemblyReferences, LoadType loadType, const Assembly* rootAssembly, const std::string& currentAssemblyDir, 
    std::unordered_set<std::string>& importSet, std::vector<CallInst*>& callInstructions, std::vector<Fun2DlgInst*>& fun2DlgInstructions, std::vector<MemFun2ClassDlgInst*>& memFun2ClassDlgInstructions, 
    std::vector<TypeInstruction*>& typeInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions, std::vector<ClassTypeSymbol*>& classTypeSymbols, 
    std::unordered_set<utf32_string>& classTemplateSpecializationNames, std::vector<Assembly*>& assemblies, std::unordered_map<std::string, AssemblyDependency*>& dependencyMap)
{
    std::vector<std::string> allAssemblyReferences;
    allAssemblyReferences.insert(allAssemblyReferences.end(), assemblyReferences.cbegin(), assemblyReferences.cend());
    if (!IsSystemAssembly())
    {
        allAssemblyReferences.push_back(CminorSystemAssemblyFilePath(GetConfig()));
    }
    Import(allAssemblyReferences, loadType, rootAssembly, importSet, currentAssemblyDir, callInstructions, fun2DlgInstructions, memFun2ClassDlgInstructions, typeInstructions, 
        setClassDataInstructions, classTypeSymbols, classTemplateSpecializationNames, assemblies, dependencyMap);
}

void Assembly::Import(const std::vector<std::string>& assemblyReferences, LoadType loadType, const Assembly* rootAssembly, std::unordered_set<std::string>& importSet, 
    const std::string& currentAssemblyDir, std::vector<CallInst*>& callInstructions, std::vector<Fun2DlgInst*>& fun2DlgInstructions, std::vector<MemFun2ClassDlgInst*>& memFun2ClassDlgInstructions, 
    std::vector<TypeInstruction*>& typeInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions, std::vector<ClassTypeSymbol*>& classTypeSymbols, 
    std::unordered_set<utf32_string>& classTemplateSpecializationNames, std::vector<Assembly*>& assemblies, std::unordered_map<std::string, AssemblyDependency*>& dependencyMap)
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
            importSet.insert(assemblyFilePath);
            SymbolReader reader(assemblyFilePath);
            reader.SetMachine(machine);
            referencedAssembly->BeginRead(reader, loadType, rootAssembly, currentAssemblyDir, importSet, callInstructions, fun2DlgInstructions, memFun2ClassDlgInstructions, typeInstructions, 
                setClassDataInstructions, classTypeSymbols, classTemplateSpecializationNames, assemblies, dependencyMap);
            Assembly* referencedAssemblyPtr = referencedAssembly.get();
            assemblyDependency.AddReferencedAssembly(referencedAssemblyPtr);
            referencedAssemblies.push_back(std::move(referencedAssembly));
            Import(referencedAssemblyPtr->referenceFilePaths, loadType, rootAssembly, importSet, currentAssemblyDir, callInstructions, fun2DlgInstructions, memFun2ClassDlgInstructions, 
                typeInstructions, setClassDataInstructions, classTypeSymbols, classTemplateSpecializationNames, assemblies, dependencyMap);
        }
    }
}

void Assembly::Dump(CodeFormatter& formatter, DumpOptions dumpOptions)
{
    if ((dumpOptions & DumpOptions::functions) != DumpOptions::none)
    {
        machineFunctionTable.Dump(formatter);
    }
    if ((dumpOptions & DumpOptions::symbols) != DumpOptions::none)
    {
        symbolTable.Dump(formatter);
    }
}

void Assembly::AddSymbolIdMapping(const std::string& assemblyName, uint32_t assemblySymbolId, uint32_t mySymbolId)
{
    std::pair<std::string, uint32_t> key = std::make_pair(assemblyName, assemblySymbolId);
    symbolIdMapping[key] = mySymbolId;
    utf32_string an = ToUtf32(assemblyName);
    constantPool.Install(StringPtr(an.c_str()));
}

uint32_t Assembly::GetSymbolIdMapping(const std::string& assemblyName, uint32_t assemblySymbolId) const
{
    std::pair<std::string, uint32_t> key = std::make_pair(assemblyName, assemblySymbolId);
    auto it = symbolIdMapping.find(key);
    if (it != symbolIdMapping.cend())
    {
        return it->second;
    }
    return noSymbolId;
}

void Assembly::WriteSymbolIdMapping(SymbolWriter& writer)
{
    uint32_t n = uint32_t(symbolIdMapping.size());
    writer.AsMachineWriter().PutEncodedUInt(n);
    for (const auto& x : symbolIdMapping)
    {
        const std::string& assemblyName = x.first.first;
        utf32_string an = ToUtf32(assemblyName);
        ConstantId id = constantPool.GetIdFor(an);
        Assert(id != noConstantId, "got no id for constant");
        id.Write(writer);
        writer.AsMachineWriter().PutEncodedUInt(x.first.second);
        writer.AsMachineWriter().PutEncodedUInt(x.second);
    }
}

void Assembly::ReadSymbolIdMapping(SymbolReader& reader)
{
    uint32_t n = reader.GetEncodedUInt();
    for (uint32_t i = 0; i < n; ++i)
    {
        ConstantId id;
        id.Read(reader);
        Constant constant = constantPool.GetConstant(id);
        Assert(constant.Value().GetType() == ValueType::stringLiteral, "string literal expected");
        std::string assemblyName = ToUtf8(constant.Value().AsStringLiteral());
        uint32_t assemblySymbolId = reader.GetEncodedUInt();
        uint32_t mySymbolId = reader.GetEncodedUInt();
        std::pair<std::string, uint32_t> key = std::make_pair(assemblyName, assemblySymbolId);
        symbolIdMapping[key] = mySymbolId;
    }
}

Constant Assembly::RegisterSourceFilePath(const std::string& sourceFilePath)
{
    utf32_string sfp = ToUtf32(sourceFilePath);
    return constantPool.GetConstant(constantPool.Install(StringPtr(sfp.c_str())));
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

void AssignPriorities(std::vector<ClassTypeSymbol*>& leaves)
{
    for (ClassTypeSymbol* leaf : leaves)
    {
        int priority = leaf->Level();
        leaf->SetPriority(priority);
        ClassTypeSymbol* base = leaf->BaseClass();
        while (base)
        {
            if (base->Priority() < priority)
            {
                base->SetPriority(priority);
            }
            else
            {
                priority = base->Priority();
            }
            base = base->BaseClass();
        }
    }
}

struct PriorityGreater
{
    bool operator()(ClassTypeSymbol* left, ClassTypeSymbol* right) const
    {
        if (left->Level() < right->Level())
        {
            return true;
        }
        else if (right->Level() < left->Level())
        {
            return false;
        }
        else if (left->Priority() > right->Priority())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
};

void AssignKeys(std::vector<ClassTypeSymbol*>& classesByPriority)
{
    uint64_t key = 2;
    uint64_t minLevelKey = key;
    uint64_t maxLevelKey = key;
    int predLevel = -1;
    std::unordered_set<ClassTypeSymbol*> bases;
    for (ClassTypeSymbol* cls : classesByPriority)
    {
        int level = cls->Level();
        if (level == 0)
        {
            cls->SetKey(key);
            key = cminor::machine::NextPrime(key + 1);
            maxLevelKey = key;
        }
        else
        {
            if (predLevel != level)
            {
                bases.clear();
                bases.insert(cls->BaseClass());
                key = cminor::machine::NextPrime(maxLevelKey + 1);
                minLevelKey = key;
                cls->SetKey(key);
                key = cminor::machine::NextPrime(key + 1);
                maxLevelKey = key;
            }
            else
            {
                if (bases.find(cls->BaseClass()) == bases.end())
                {
                    key = minLevelKey;
                }
                bases.insert(cls->BaseClass());
                cls->SetKey(key);
                key = cminor::machine::NextPrime(key + 1);
                if (key > maxLevelKey)
                {
                    maxLevelKey = key;
                }
            }
            predLevel = level;
        }
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
        classType->SetLevel(NumberOfAncestors(classType));
        if (classType->BaseClass())
        {
            classType->BaseClass()->SetNonLeaf();
        }
    }
    std::vector<ClassTypeSymbol*> leaves;
    for (ClassTypeSymbol* classType : classTypes)
    {
        if (!classType->IsNonLeaf())
        {
            leaves.push_back(classType);
        }
    }
    AssignPriorities(leaves);
    std::vector<ClassTypeSymbol*> classesByPriority;
    for (ClassTypeSymbol* classType : classTypes)
    {
        classesByPriority.push_back(classType);
    }
    std::sort(classesByPriority.begin(), classesByPriority.end(), PriorityGreater());
    AssignKeys(classesByPriority);
    AssignCids(classesByPriority);
}

void Link(const std::vector<CallInst*>& callInstructions, const std::vector<Fun2DlgInst*>& fun2DlgInstructions, const std::vector<MemFun2ClassDlgInst*>& memFun2ClassDlgInstructions, 
    const std::vector<TypeInstruction*>& typeInstructions, const std::vector<SetClassDataInst*>& setClassDataInstructions, const std::vector<ClassTypeSymbol*>& classTypes)
{
    for (CallInst* call : callInstructions)
    {
        Function* fun = FunctionTable::Instance().GetFunction(call->GetFunctionCallName());
        call->SetFunction(fun);
    }
    for (Fun2DlgInst* fun2Dlg : fun2DlgInstructions)
    {
        Function* fun = FunctionTable::Instance().GetFunction(fun2Dlg->GetFunctionName());
        fun2Dlg->SetFunction(fun);
    }
    for (MemFun2ClassDlgInst* memFun2ClassDlgInst : memFun2ClassDlgInstructions)
    {
        Function* fun = FunctionTable::Instance().GetFunction(memFun2ClassDlgInst->GetFunctionName());
        memFun2ClassDlgInst->SetFunction(fun);
    }
    for (TypeInstruction* typeInst : typeInstructions)
    {
        Type* type = TypeTable::Instance().GetType(typeInst->GetTypeName());
        typeInst->SetType(type);
    }
    for (SetClassDataInst* setClassDataInst : setClassDataInstructions)
    {
        ClassData* cd = ClassDataTable::Instance().GetClassData(setClassDataInst->GetClassName());
        setClassDataInst->SetClassData(cd);
    }
    for (ClassTypeSymbol* classType : classTypes)
    {
        classType->LinkVmt();
        classType->LinkImts();
    }
    AssignClassTypeIds(classTypes);
    FunctionTable::Instance().ResolveExceptionVarTypes();
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
