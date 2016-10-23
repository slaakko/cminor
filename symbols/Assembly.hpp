// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_ASSEMBLY_INCLUDED
#define CMINOR_SYMBOLS_ASSEMBLY_INCLUDED
#include <cminor/symbols/Symbol.hpp>
#include <cminor/symbols/MachineFunctionTable.hpp>

namespace cminor { namespace symbols {

using namespace cminor::machine;

class SymbolReader;
class SymbolWriter;

enum class LoadType
{
    build, execute
};

void Link(const std::vector<CallInst*>& callInstructions, const std::vector<CreateObjectInst*>& createObjectInstructions, const std::vector<SetClassDataInst*>& setClassDataInstructions, 
    const std::vector<ClassTypeSymbol*>& classTypes);

class Assembly
{
public:
    Assembly(Machine& machine_);
    Assembly(Machine& machine_, const utf32_string& name_, const std::string& filePath_);
    Machine& GetMachine() { return machine; }
    void Write(SymbolWriter& writer);
    void Read(SymbolReader& reader, LoadType loadType, const Assembly* rootAssembly, const std::string& currentAssemblyDir, std::unordered_set<std::string>& importSet, 
        std::vector<CallInst*>& callInstructions, std::vector<CreateObjectInst*>& createObjectInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions, 
        std::vector<ClassTypeSymbol*>& classTypeSymbols);
    const std::string& FilePath() const { return filePath; }
    StringPtr Name() const { return StringPtr(name.Value().AsStringLiteral()); }
    ConstantPool& GetConstantPool() { return constantPool; }
    MachineFunctionTable& GetMachineFunctionTable() { return machineFunctionTable; }
    SymbolTable& GetSymbolTable() { return symbolTable; }
    bool IsSystemAssembly() const;
    void ImportAssemblies(LoadType loadType, const Assembly* rootAssembly, const std::string& currentAssemblyDir, std::unordered_set<std::string>& importSet, std::vector<CallInst*>& callInstructions,
        std::vector<CreateObjectInst*>& createObjectInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions, std::vector<ClassTypeSymbol*>& classTypeSymbols);
    void ImportAssemblies(const std::vector<std::string>& assemblyReferences, LoadType loadType, const Assembly* rootAssembly, const std::string& currentAssemblyDir, 
        std::unordered_set<std::string>& importSet,
        std::vector<CallInst*>& callInstructions, std::vector<CreateObjectInst*>& createObjectInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions, 
        std::vector<ClassTypeSymbol*>& classTypeSymbols);
    void ImportSymbolTables();
    void Dump(CodeFormatter& formatter);
    const std::vector<std::unique_ptr<Assembly>>& ReferencedAssemblies() const { return referencedAssemblies; }
private:
    Machine& machine;
    std::string filePath;
    std::vector<std::string> referenceFilePaths;
    std::vector<std::unique_ptr<Assembly>> referencedAssemblies;
    ConstantPool constantPool;
    Constant name;
    MachineFunctionTable machineFunctionTable;
    SymbolTable symbolTable;
    void Import(const std::vector<std::string>& assemblyReferences, LoadType loadType, const Assembly* rootAssembly, std::unordered_set<std::string>& importSet, const std::string& currentAssemblyDir,
        std::vector<CallInst*>& callInstructions, std::vector<CreateObjectInst*>& createObjectInstructions, std::vector<SetClassDataInst*>& setClassDataInstructions, 
        std::vector<ClassTypeSymbol*>& classTypeSymbols);
};

void InitAssembly();
void DoneAssembly();

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_ASSEMBLY_INCLUDED
