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

void Link(const std::vector<CallInst*>& callInstructions);

class Assembly
{
public:
    Assembly(Machine& machine_);
    Assembly(Machine& machine_, const utf32_string& name_, const std::string& filePath_);
    Machine& GetMachine() { return machine; }
    void Write(SymbolWriter& writer);
    void Read(SymbolReader& reader, std::vector<CallInst*>& callInstructions);
    const std::string& FilePath() const { return filePath; }
    StringPtr Name() const { return StringPtr(name.Value().AsStringLiteral()); }
    ConstantPool& GetConstantPool() { return constantPool; }
    MachineFunctionTable& GetMachineFunctionTable() { return machineFunctionTable; }
    SymbolTable& GetSymbolTable() { return symbolTable; }
    bool IsSystemAssembly() const;
    void ImportAssemblies(std::vector<CallInst*>& callInstructions);
    void ImportAssemblies(const std::vector<std::string>& assemblyReferences, std::vector<CallInst*>& callInstructions);
    void ImportSymbolTables();
    void Dump(CodeFormatter& formatter);
private:
    Machine& machine;
    std::string filePath;
    std::vector<std::string> referenceFilePaths;
    std::vector<std::unique_ptr<Assembly>> referencedAssemblies;
    ConstantPool constantPool;
    Constant name;
    MachineFunctionTable machineFunctionTable;
    SymbolTable symbolTable;
    void Import(const std::vector<std::string>& assemblyReferences, std::unordered_set<std::string>& importSet, std::vector<CallInst*>& callInstructions);
};

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_ASSEMBLY_INCLUDED
