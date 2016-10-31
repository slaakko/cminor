// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolReader.hpp>
#include <cminor/symbols/GlobalFlags.hpp>
#include <cminor/ast/Project.hpp>
#include <cminor/machine/Class.hpp>
#include <boost/filesystem.hpp>
#include <Cm.Util/Path.hpp>
#include <Cm.Util/Prime.hpp>

namespace cminor { namespace symbols {

using namespace cminor::ast;
using namespace Cm::Util;

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
    systemAssemblyNames.push_back(U"System.Basic");
    systemAssemblyNames.push_back(U"System.Collections");
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

Assembly::Assembly(Machine& machine_) : machine(machine_), filePath(), constantPool(), symbolTable(this), name()
{
}

Assembly::Assembly(Machine& machine_, const utf32_string& name_, const std::string& filePath_) : machine(machine_), filePath(filePath_), constantPool(), symbolTable(this),
    name(constantPool.GetConstant(constantPool.Install(StringPtr(name_.c_str()))))
{
}

void Assembly::Write(SymbolWriter& writer)
{
    AssemblyTag tag;
    tag.Write(writer);
    writer.SetAssembly(this);
    writer.AsMachineWriter().Put(filePath);
    int32_t n = int32_t(referencedAssemblies.size());
    writer.AsMachineWriter().Put(n);
    for (int32_t i = 0; i < n; ++i)
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
}

void Assembly::Read(SymbolReader& reader, LoadType loadType, const Assembly* rootAssembly, const std::string& currentAssemblyDir, std::unordered_set<std::string>& importSet, 
    std::vector<CallInst*>& callInstructions, std::vector<TypeInstruction*>& typeInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions,
    std::vector<ClassTypeSymbol*>& classTypeSymbols)
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
    int32_t n = reader.GetInt();
    for (int32_t i = 0; i < n; ++i)
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
    ImportAssemblies(loadType, rootAssembly, currentAssemblyDir, importSet, callInstructions, typeInstructions, setClassDataInstructions, classTypeSymbols);
    ImportSymbolTables();
    symbolTable.Read(reader);
    callInstructions.insert(callInstructions.end(), reader.CallInstructions().cbegin(), reader.CallInstructions().cend());
    typeInstructions.insert(typeInstructions.end(), reader.TypeInstructions().cbegin(), reader.TypeInstructions().cend());
    setClassDataInstructions.insert(setClassDataInstructions.end(), reader.SetClassDataInstructions().cbegin(), reader.SetClassDataInstructions().cend());
    classTypeSymbols.insert(classTypeSymbols.end(), reader.ClassTypeSymbols().cbegin(), reader.ClassTypeSymbols().cend());
};

bool Assembly::IsSystemAssembly() const
{
    return CminorSystemAssemblyNameCollection::Instance().Find(StringPtr(name.Value().AsStringLiteral()));
}

void Assembly::ImportAssemblies(LoadType loadType, const Assembly* rootAssembly, const std::string& currentAssemblyDir, std::unordered_set<std::string>& importSet, 
    std::vector<CallInst*>& callInstructions, std::vector<TypeInstruction*>& typeInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions,
    std::vector<ClassTypeSymbol*>& classTypeSymbols)
{
    ImportAssemblies(referenceFilePaths, loadType, rootAssembly, currentAssemblyDir, importSet, callInstructions, typeInstructions, setClassDataInstructions, classTypeSymbols);
}

void Assembly::ImportSymbolTables()
{
    for (const std::unique_ptr<Assembly>& referencedAssembly : referencedAssemblies)
    {
        symbolTable.Import(referencedAssembly->symbolTable);
    }
}

void Assembly::ImportAssemblies(const std::vector<std::string>& assemblyReferences, LoadType loadType, const Assembly* rootAssembly, const std::string& currentAssemblyDir, 
    std::unordered_set<std::string>& importSet, std::vector<CallInst*>& callInstructions, std::vector<TypeInstruction*>& typeInstructions,
    std::vector<SetClassDataInst*>& setClassDataInstructions, std::vector<ClassTypeSymbol*>& classTypeSymbols)
{
    std::vector<std::string> allAssemblyReferences;
    if (!IsSystemAssembly())
    {
        allAssemblyReferences.push_back(CminorSystemAssemblyFilePath(GetConfig()));
    }
    allAssemblyReferences.insert(allAssemblyReferences.end(), assemblyReferences.cbegin(), assemblyReferences.cend());
    Import(allAssemblyReferences, loadType, rootAssembly, importSet, currentAssemblyDir, callInstructions, typeInstructions, setClassDataInstructions, classTypeSymbols);
}

void Assembly::Import(const std::vector<std::string>& assemblyReferences, LoadType loadType, const Assembly* rootAssembly, std::unordered_set<std::string>& importSet, 
    const std::string& currentAssemblyDir, std::vector<CallInst*>& callInstructions, std::vector<TypeInstruction*>& typeInstructions,
    std::vector<SetClassDataInst*>& setClassDataInstructions, std::vector<ClassTypeSymbol*>& classTypeSymbols)
{
    for (const std::string& assemblyReference : assemblyReferences)
    {
        if (importSet.find(assemblyReference) == importSet.cend())
        {
            importSet.insert(assemblyReference);
            std::unique_ptr<Assembly> referencedAssembly(new Assembly(machine));
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
            SymbolReader reader(assemblyFilePath);
            reader.SetMachine(machine);
            referencedAssembly->Read(reader, loadType, rootAssembly, currentAssemblyDir, importSet, callInstructions, typeInstructions, setClassDataInstructions, classTypeSymbols);
            Assembly* referencedAssemblyPtr = referencedAssembly.get();
            referencedAssemblies.push_back(std::move(referencedAssembly));
            Import(referencedAssemblyPtr->referenceFilePaths, loadType, rootAssembly, importSet, currentAssemblyDir, callInstructions, typeInstructions, setClassDataInstructions,
                classTypeSymbols);
        }
    }
}

void Assembly::Dump(CodeFormatter& formatter)
{
    machineFunctionTable.Dump(formatter);
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
            key = Cm::Util::NextPrime(key + 1);
            maxLevelKey = key;
        }
        else
        {
            if (predLevel != level)
            {
                bases.clear();
                bases.insert(cls->BaseClass());
                key = Cm::Util::NextPrime(maxLevelKey + 1);
                minLevelKey = key;
                cls->SetKey(key);
                key = Cm::Util::NextPrime(key + 1);
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
                key = Cm::Util::NextPrime(key + 1);
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

void Link(const std::vector<CallInst*>& callInstructions, const std::vector<TypeInstruction*>& typeInstructions, const std::vector<SetClassDataInst*>& setClassDataInstructions,
    const std::vector<ClassTypeSymbol*>& classTypes)
{
    for (CallInst* call : callInstructions)
    {
        Function* fun = FunctionTable::Instance().GetFunction(call->GetFunctionCallName());
        call->SetFunction(fun);
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
    }
    AssignClassTypeIds(classTypes);
}

void InitAssembly()
{
    CminorSystemAssemblyNameCollection::Init();
}

void DoneAssembly()
{
    CminorSystemAssemblyNameCollection::Done();
}

} } // namespace cminor::symbols
