// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_ASSEMBLY_INCLUDED
#define CMINOR_SYMBOLS_ASSEMBLY_INCLUDED
#include <cminor/symbols/SymbolTable.hpp>
#include <cminor/symbols/MachineFunctionTable.hpp>

namespace cminor { namespace symbols {

using namespace cminor::machine;

class SymbolReader;
class SymbolWriter;

enum class DumpOptions : uint8_t
{
    none = 0, 
    header = 1 << 0, 
    constants = 1 << 1, 
    functions = 1 << 2, 
    symbols = 1 << 3,
    mappings = 1 << 4,
    all = header | constants | functions | symbols | mappings
};

inline DumpOptions operator&(DumpOptions left, DumpOptions right)
{
    return DumpOptions(uint8_t(left) & uint8_t(right));
}

inline DumpOptions operator|(DumpOptions left, DumpOptions right)
{
    return DumpOptions(uint8_t(left) | uint8_t(right));
}

enum class LoadType
{
    build, execute
};

void Link(const std::vector<CallInst*>& callInstructions, const std::vector<Fun2DlgInst*>& fun2DlgInstructions, 
    const std::vector<MemFun2ClassDlgInst*>& memFun2ClassDlgInstructions, const std::vector<TypeInstruction*>& typeInstructions, const std::vector<SetClassDataInst*>& setClassDataInstructions, 
    const std::vector<ClassTypeSymbol*>& classTypes);

struct SymbolIdHash
{
    size_t operator()(const std::pair<std::string, uint32_t>& p) const
    {
        return std::hash<std::string>()(p.first) ^ size_t(p.second);
    }
};

class Assembly;

class AssemblyDependency
{
public:
    AssemblyDependency(Assembly* assembly_);
    Assembly* GetAssembly() const { return assembly; }
    void AddReferencedAssembly(Assembly* referencedAssembly);
    const std::vector<Assembly*>& ReferencedAssemblies() const { return referencedAssemblies; }
    void Dump(CodeFormatter& formatter);
private:
    Assembly* assembly;
    std::vector<Assembly*> referencedAssemblies;
};

std::vector<Assembly*> CreateFinishReadOrder(const std::vector<Assembly*>& assemblies, std::unordered_map<Assembly*, AssemblyDependency*>& dependencyMap, const Assembly* rootAssembly);

struct FunctionPtrVarFunctionName
{
    FunctionPtrVarFunctionName(Constant functionPtrVarName_, Constant functionName_) : functionPtrVarName(functionPtrVarName_), functionName(functionName_) {}
    Constant functionPtrVarName;
    Constant functionName;
};

struct ClassDataPtrVarClassDataName
{
    ClassDataPtrVarClassDataName(Constant classDataPtrVarName_, Constant classDataName_) : classDataPtrVarName(classDataPtrVarName_), classDataName(classDataName_) {}
    Constant classDataPtrVarName;
    Constant classDataName;
};

struct TypePtrVarTypeName
{
    TypePtrVarTypeName(Constant typePtrVarName_, Constant typeName_) : typePtrVarName(typePtrVarName_), typeName(typeName_) {}
    Constant typePtrVarName;
    Constant typeName;
};

enum class AssemblyFlags : uint8_t
{
    none = 0,
    core = 1 << 0,
    system = 1 << 1,
    native = 1 << 2,
    linkedWithDebugMachine = 1 << 3
};

std::string AssemblyFlagsStr(AssemblyFlags flags);

inline AssemblyFlags operator|(AssemblyFlags left, AssemblyFlags right)
{
    return AssemblyFlags(uint8_t(left) | uint8_t(right));
}

inline AssemblyFlags operator&(AssemblyFlags left, AssemblyFlags right)
{
    return AssemblyFlags(uint8_t(left) & uint8_t(right));
}

enum class TransientAssemblyFlags : uint8_t
{
    none = 0,
    imported = 1 << 0,
    readClassNodes = 1 << 1,
    sharedLibraryLoaded = 1 << 2,
    exportedFunctionsResolved = 1 << 3,
    functionPtrVarMappingsResolved = 1 << 4,
    classDataPtrVarMappingsResolved = 1 << 5,
    typePtrVarMappingsResolved = 1 << 6,
    constantPoolVariableSet = 1 << 7
};

inline TransientAssemblyFlags operator|(TransientAssemblyFlags left, TransientAssemblyFlags right)
{
    return TransientAssemblyFlags(uint8_t(left) | uint8_t(right));
}

inline TransientAssemblyFlags operator&(TransientAssemblyFlags left, TransientAssemblyFlags right)
{
    return TransientAssemblyFlags(uint8_t(left) & uint8_t(right));
}

const uint8_t assemblyFormat_1 = uint8_t('1');
const uint8_t currentAssemblyFormat = assemblyFormat_1;

class Assembly
{
public:
    Assembly(Machine& machine_);
    Assembly(Machine& machine_, const utf32_string& name_, const std::string& filePath_);
    ~Assembly();
    void Load(const std::string& assemblyFilePath);
    void PrepareForCompilation(const std::vector<std::string>& projectAssemblyReferences);
    int RunIntermediateCode(const std::vector<utf32_string>& programArguments);
    int RunNative(const std::vector<utf32_string>& programArguments);
    void SetId(uint32_t id_) { id = id_; }
    uint32_t Id() const { return id; }
    Machine& GetMachine() { return machine; }
    bool IsCore() const { return GetFlag(AssemblyFlags::core); }
    void SetCore() { SetFlag(AssemblyFlags::core); }
    bool IsSystemAssembly() const { return GetFlag(AssemblyFlags::system); }
    void SetSystemAssembly() { SetFlag(AssemblyFlags::system); }
    bool IsNative() const { return GetFlag(AssemblyFlags::native); }
    void SetNative() { SetFlag(AssemblyFlags::native); }
    bool LinkedWithDebugMachine() const { return GetFlag(AssemblyFlags::linkedWithDebugMachine); }
    void SetLinkedWithDebugMachine() { SetFlag(AssemblyFlags::linkedWithDebugMachine); }
    void Dump(CodeFormatter& formatter, DumpOptions dumpOptions);
    void DumpHeader(CodeFormatter& formatter);
    void DumpMappings(CodeFormatter& formatter);
    void Write(SymbolWriter& writer);
    void Read(SymbolReader& reader, LoadType loadType, const Assembly* rootAssembly, const std::string& currentAssemblyDir, std::unordered_set<std::string>& importSet, 
        std::vector<CallInst*>& callInstructions, std::vector<Fun2DlgInst*>& fun2DlgInstructions,
        std::vector<MemFun2ClassDlgInst*>& memFun2ClassDlgInstructions, std::vector<TypeInstruction*>& typeInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions, 
        std::vector<ClassTypeSymbol*>& classTypeSymbols, std::unordered_set<utf32_string>& classTemplateSpecializationNames, std::vector<Assembly*>& assemblies, 
        std::unordered_map<std::string, AssemblyDependency*>& dependencyMap, std::unordered_map<std::string, Assembly*>& readMap);
    void BeginRead(SymbolReader& reader, LoadType loadType, const Assembly* rootAssembly, const std::string& currentAssemblyDir, std::unordered_set<std::string>& importSet,
        std::vector<CallInst*>& callInstructions, std::vector<Fun2DlgInst*>& fun2DlgInstructions, std::vector<MemFun2ClassDlgInst*>& memFun2ClassDlgInstructions, 
        std::vector<TypeInstruction*>& typeInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions, 
        std::vector<ClassTypeSymbol*>& classTypeSymbols, std::unordered_set<utf32_string>& classTemplateSpecializationNames, std::vector<Assembly*>& assemblies,
        std::unordered_map<std::string, AssemblyDependency*>& dependencyMap, std::unordered_map<std::string, Assembly*>& readMap);
    void FinishReads(std::vector<CallInst*>& callInstructions, std::vector<Fun2DlgInst*>& fun2DlgInstructions,
        std::vector<MemFun2ClassDlgInst*>& memFun2ClassDlgInstructions, std::vector<TypeInstruction*>& typeInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions, 
        std::vector<ClassTypeSymbol*>& classTypeSymbols, std::unordered_set<utf32_string>& classTemplateSpecializationNames, int prevAssemblyIndex, const std::vector<Assembly*>& finishReadOrder, 
        bool readSymbolTable);
    const std::string& OriginalFilePath() const { return originalFilePath; }
    const std::string& FilePathReadFrom() const { return filePathReadFrom; }
    StringPtr Name() const { return StringPtr(name.Value().AsStringLiteral()); }
    ConstantPool& GetConstantPool() { return constantPool; }
    MachineFunctionTable& GetMachineFunctionTable() { return machineFunctionTable; }
    SymbolTable& GetSymbolTable() { return symbolTable; }
    void SetNativeTargetTriple(const std::string& nativeTargetTriple_);
    const std::string& NativeTargetTriple() const { return nativeTargetTriple; }
    void SetNativeSharedLibraryFileName(const std::string& nativeSharedLibraryFileName_);
    const std::string& NativeSharedLibraryFileName() const { return nativeSharedLibraryFileName; }
    std::string NativeSharedLibraryFilePath() const;
    void SetNativeImportLibraryFileName(const std::string& nativeImportLibraryFileName_);
    const std::string& NativeImportLibraryFileName() const { return nativeImportLibraryFileName; }
    void ImportAssemblies(LoadType loadType, const Assembly* rootAssembly, const std::string& currentAssemblyDir, std::unordered_set<std::string>& importSet,
        std::vector<CallInst*>& callInstructions, std::vector<Fun2DlgInst*>& fun2DlgInstructions,
        std::vector<MemFun2ClassDlgInst*>& memFun2ClassDlgInstructions, std::vector<TypeInstruction*>& typeInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions, 
        std::vector<ClassTypeSymbol*>& classTypeSymbols, std::unordered_set<utf32_string>& classTemplateSpecializationNames, std::vector<Assembly*>& assemblies, 
        std::unordered_map<std::string, AssemblyDependency*>& dependencyMap, std::unordered_map<std::string, Assembly*>& readMap);
    void ImportAssemblies(const std::vector<std::string>& assemblyReferences, LoadType loadType, const Assembly* rootAssembly, const std::string& currentAssemblyDir, 
        std::unordered_set<std::string>& importSet, std::vector<CallInst*>& callInstructions, 
        std::vector<Fun2DlgInst*>& fun2DlgInstructions, std::vector<MemFun2ClassDlgInst*>& memFun2ClassDlgInstructions, 
        std::vector<TypeInstruction*>& typeInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions, std::vector<ClassTypeSymbol*>& classTypeSymbols, 
        std::unordered_set<utf32_string>& classTemplateSpecializationNames, std::vector<Assembly*>& assemblies, std::unordered_map<std::string, AssemblyDependency*>& dependencyMap,
        std::unordered_map<std::string, Assembly*>& readMap);
    void ImportSymbolTables();
    const std::vector<std::unique_ptr<Assembly>>& ReferencedAssemblies() const { return referencedAssemblies; }
    void AddSymbolIdMapping(const std::string& assemblyName, uint32_t assemblySymbolId, uint32_t mySymbolId);
    uint32_t GetSymbolIdMapping(const std::string& assemblyName, uint32_t assemblySymbolId) const;
    AssemblyDependency* GetAssemblyDependency() { return &assemblyDependency; }
    Constant RegisterSourceFilePath(const std::string& sourceFilePath);
    void AddExportedFunction(Constant exportedFunction);
    const std::vector<Constant>& ExportedFunctions() const { return exportedFunctions; }
    void AddFunctionVarMapping(Constant functionPtrVarName, Constant functionName);
    const std::vector<FunctionPtrVarFunctionName>& FunctionPtrVarMappings() const { return functionPtrVarMappings; }
    void AddClassDataVarMapping(Constant classDataPtrVarName, Constant classDataName);
    const std::vector<ClassDataPtrVarClassDataName>& ClassDataPtrVarMappings() const { return classDatatPtrVarMappings; }
    void AddTypePtrVarMapping(Constant typePtrVarName, Constant typeName);
    const std::vector<TypePtrVarTypeName>& TypePtrVarMappings() const { return typePtrVarMappings; }
    bool ReadClassNodes() const { return GetTransientFlag(TransientAssemblyFlags::readClassNodes); }
    void SetReadClassNodes() { SetTransientFlag(TransientAssemblyFlags::readClassNodes); }
    bool Imported() const { return GetTransientFlag(TransientAssemblyFlags::imported); }
    void SetImported() { SetTransientFlag(TransientAssemblyFlags::imported); }
    bool SharedLibraryLoaded() const { return GetTransientFlag(TransientAssemblyFlags::sharedLibraryLoaded); }
    void SetSharedLibraryLoaded() { SetTransientFlag(TransientAssemblyFlags::sharedLibraryLoaded); }
    bool ExportedFunctionsResolved() const { return GetTransientFlag(TransientAssemblyFlags::exportedFunctionsResolved); }
    void SetExportedFunctionsResolved() { SetTransientFlag(TransientAssemblyFlags::exportedFunctionsResolved); }
    bool FunctionPtrVarMappingsResolved() const { return GetTransientFlag(TransientAssemblyFlags::functionPtrVarMappingsResolved); }
    void SetFunctionPtrVarMappingsResolved() { SetTransientFlag(TransientAssemblyFlags::functionPtrVarMappingsResolved); }
    bool ClassDataPtrVarMappingsResolved() const { return GetTransientFlag(TransientAssemblyFlags::classDataPtrVarMappingsResolved); }
    void SetClassDataPtrVarMappingsResolved() { SetTransientFlag(TransientAssemblyFlags::classDataPtrVarMappingsResolved); }
    bool TypePtrVarMappingsResolved() const { return GetTransientFlag(TransientAssemblyFlags::typePtrVarMappingsResolved); }
    void SetTypePtrVarMappingsResolved() { SetTransientFlag(TransientAssemblyFlags::typePtrVarMappingsResolved); }
    bool ConstantPoolVariableSet() const { return GetTransientFlag(TransientAssemblyFlags::constantPoolVariableSet); }
    void SetConstantPoolVariableSet() { SetTransientFlag(TransientAssemblyFlags::constantPoolVariableSet); }
    void SetSharedLibraryHandle(void* sharedLibraryHandle_) { sharedLibraryHandle = sharedLibraryHandle_; }
    void* SharedLibraryHandle() const { return sharedLibraryHandle; }
    void SetMainEntryPointAddress(void* mainEntryPointAddress_) { mainEntryPointAddress = mainEntryPointAddress_; }
    void* MainEntryPointAddress() const { return mainEntryPointAddress; }
private:
    Machine& machine;
    AssemblyFlags flags;
    uint8_t assemblyFormat;
    std::string originalFilePath;
    std::string filePathReadFrom;
    std::string nativeTargetTriple;
    std::string nativeSharedLibraryFileName;
    std::string nativeImportLibraryFileName;
    std::vector<std::string> referenceFilePaths;
    std::vector<std::unique_ptr<Assembly>> referencedAssemblies;
    AssemblyDependency assemblyDependency;
    ConstantPool constantPool;
    Constant name;
    uint32_t id;
    MachineFunctionTable machineFunctionTable;
    uint32_t finishReadPos;
    SymbolTable symbolTable;
    std::unordered_map<std::pair<std::string, uint32_t>, uint32_t, SymbolIdHash> symbolIdMapping;
    std::vector<Constant> exportedFunctions;
    std::vector<FunctionPtrVarFunctionName> functionPtrVarMappings;
    std::vector<ClassDataPtrVarClassDataName> classDatatPtrVarMappings;
    std::vector<TypePtrVarTypeName> typePtrVarMappings;
    TransientAssemblyFlags transientFlags;
    void* sharedLibraryHandle;
    void* mainEntryPointAddress;
    void Import(const std::vector<std::string>& assemblyReferences, LoadType loadType, const Assembly* rootAssembly, std::unordered_set<std::string>& importSet, const std::string& currentAssemblyDir,
        std::vector<CallInst*>& callInstructions, std::vector<Fun2DlgInst*>& fun2DlgInstructions,
        std::vector<MemFun2ClassDlgInst*>& memFun2ClassDlgInstructions, std::vector<TypeInstruction*>& typeInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions, 
        std::vector<ClassTypeSymbol*>& classTypeSymbols, std::unordered_set<utf32_string>& classTemplateSpecializationNames, std::vector<Assembly*>& assemblies, 
        std::unordered_map<std::string, AssemblyDependency*>& dependencyMap, std::unordered_map<std::string, Assembly*>& readMap);
    void WriteSymbolIdMapping(SymbolWriter& writer);
    void ReadSymbolIdMapping(SymbolReader& reader);
    void WriteExportedFunctions(Writer& writer);
    void ReadExportedFunctions(Reader& reader);
    void WriteFunctionVarMappings(Writer& writer);
    void ReadFunctionVarMappings(Reader& reader);
    void WriteClassDataVarMappings(Writer& writer);
    void ReadClasDataVarMappings(Reader& reader);
    void WriteTypePtrVarMappings(Writer& writer);
    void ReadTypePtrVarMappings(Reader& reader);
    void PrepareForNativeExecution();
    bool GetFlag(AssemblyFlags flag) const { return (flags & flag) != AssemblyFlags::none; }
    void SetFlag(AssemblyFlags flag) { flags = (flags | flag); }
    bool GetTransientFlag(TransientAssemblyFlags flag) const { return (transientFlags & flag) != TransientAssemblyFlags::none; }
    void SetTransientFlag(TransientAssemblyFlags flag) { transientFlags = transientFlags | flag; }
};

void InitAssembly();
void DoneAssembly();

class AssemblyTable
{
public:
    static void Init();
    static void Done();
    static AssemblyTable& Instance();
    void AddAssembly(Assembly* assembly);
    Assembly* GetAssembly(uint32_t assemblyId) const;
    const std::vector<Assembly*> Assemblies() const { return assemblies; }
private:
    static std::unique_ptr<AssemblyTable> instance;
    std::vector<Assembly*> assemblies;
};

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_ASSEMBLY_INCLUDED
