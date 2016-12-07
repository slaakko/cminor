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

enum class LoadType
{
    build, execute
};

void Link(const std::vector<CallInst*>& callInstructions, const std::vector<TypeInstruction*>& typeInstructions, const std::vector<SetClassDataInst*>& setClassDataInstructions,
    const std::vector<ClassTypeSymbol*>& classTypes);

struct SymbolIdHash
{
    size_t operator()(const std::pair<std::string, uint32_t>& p) const
    {
        return std::hash<std::string>()(p.first) ^ size_t(p.second);
    }
};

class Assembly
{
public:
    Assembly(Machine& machine_);
    Assembly(Machine& machine_, const utf32_string& name_, const std::string& filePath_);
    void SetId(uint32_t id_) { id = id_; }
    uint32_t Id() const { return id; }
    Machine& GetMachine() { return machine; }
    void Write(SymbolWriter& writer);
    void Read(SymbolReader& reader, LoadType loadType, const Assembly* rootAssembly, const std::string& currentAssemblyDir, std::unordered_set<std::string>& importSet, 
        std::vector<CallInst*>& callInstructions, std::vector<TypeInstruction*>& typeInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions,
        std::vector<ClassTypeSymbol*>& classTypeSymbols, std::unordered_set<utf32_string>& classTemplateSpecializationNames);
    const std::string& FilePath() const { return filePath; }
    StringPtr Name() const { return StringPtr(name.Value().AsStringLiteral()); }
    ConstantPool& GetConstantPool() { return constantPool; }
    MachineFunctionTable& GetMachineFunctionTable() { return machineFunctionTable; }
    SymbolTable& GetSymbolTable() { return symbolTable; }
    bool IsSystemAssembly() const;
    void ImportAssemblies(LoadType loadType, const Assembly* rootAssembly, const std::string& currentAssemblyDir, std::unordered_set<std::string>& importSet, std::vector<CallInst*>& callInstructions,
        std::vector<TypeInstruction*>& typeInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions, std::vector<ClassTypeSymbol*>& classTypeSymbols, 
        std::unordered_set<utf32_string>& classTemplateSpecializationNames);
    void ImportAssemblies(const std::vector<std::string>& assemblyReferences, LoadType loadType, const Assembly* rootAssembly, const std::string& currentAssemblyDir, 
        std::unordered_set<std::string>& importSet,
        std::vector<CallInst*>& callInstructions, std::vector<TypeInstruction*>& typeInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions,
        std::vector<ClassTypeSymbol*>& classTypeSymbols, std::unordered_set<utf32_string>& classTemplateSpecializationNames);
    void ImportSymbolTables();
    void Dump(CodeFormatter& formatter);
    const std::vector<std::unique_ptr<Assembly>>& ReferencedAssemblies() const { return referencedAssemblies; }
    void AddSymbolIdMapping(const std::string& assemblyName, uint32_t assemblySymbolId, uint32_t mySymbolId);
    uint32_t GetSymbolIdMapping(const std::string& assemblyName, uint32_t assemblySymbolId) const;
private:
    Machine& machine;
    std::string filePath;
    std::vector<std::string> referenceFilePaths;
    std::vector<std::unique_ptr<Assembly>> referencedAssemblies;
    ConstantPool constantPool;
    Constant name;
    uint32_t id;
    MachineFunctionTable machineFunctionTable;
    SymbolTable symbolTable;
    std::unordered_map<std::pair<std::string, uint32_t>, uint32_t, SymbolIdHash> symbolIdMapping;
    void Import(const std::vector<std::string>& assemblyReferences, LoadType loadType, const Assembly* rootAssembly, std::unordered_set<std::string>& importSet, const std::string& currentAssemblyDir,
        std::vector<CallInst*>& callInstructions, std::vector<TypeInstruction*>& typeInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions,
        std::vector<ClassTypeSymbol*>& classTypeSymbols, std::unordered_set<utf32_string>& classTemplateSpecializationNames);
    void WriteSymbolIdMapping(SymbolWriter& writer);
    void ReadSymbolIdMapping(SymbolReader& reader);
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
private:
    static std::unique_ptr<AssemblyTable> instance;
    std::vector<Assembly*> assemblies;
};

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_ASSEMBLY_INCLUDED
