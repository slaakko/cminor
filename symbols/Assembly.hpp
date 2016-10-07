// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_ASSEMBLY_INCLUDED
#define CMINOR_SYMBOLS_ASSEMBLY_INCLUDED
#include <cminor/symbols/Symbol.hpp>

namespace cminor { namespace symbols {

using namespace cminor::machine;

class SymbolReader;
class SymbolWriter;

class Assembly
{
public:
    Assembly();
    Assembly(const utf32_string& name_, const std::string& filePath_);
    void Write(SymbolWriter& writer);
    void Read(SymbolReader& reader);
    const std::string& FilePath() const { return filePath; }
    StringPtr Name() const { return StringPtr(name.Value().AsStringLiteral()); }
    ConstantPool& GetConstantPool() { return constantPool; }
    SymbolTable& GetSymbolTable() { return symbolTable; }
    bool IsSystemAssembly() const;
    void ImportAssemblies(Machine& machine);
    void ImportAssemblies(Machine& machine, const std::vector<std::string>& assemblyReferences);
private:
    std::string filePath;
    std::vector<std::string> referenceFilePaths;
    std::vector<std::unique_ptr<Assembly>> importedAssemblies;
    ConstantPool constantPool;
    Constant name;
    SymbolTable symbolTable;
    void Import(Machine& machine, const std::vector<std::string>& assemblyReferences, std::unordered_set<std::string>& importSet);
};

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_ASSEMBLY_INCLUDED
