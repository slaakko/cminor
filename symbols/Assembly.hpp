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
    Assembly(const utf32_string& name_, const std::string& fileName_);
    void Write(SymbolWriter& writer);
    void Read(SymbolReader& reader);
    const std::string& FileName() const { return fileName; }
    StringPtr Name() const { return StringPtr(name.Value().AsStringLiteral()); }
    ConstantPool& GetConstantPool() { return constantPool; }
    SymbolTable& GetSymbolTable() { return symbolTable; }
private:
    std::string fileName;
    ConstantPool constantPool;
    Constant name;
    SymbolTable symbolTable;
};

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_ASSEMBLY_INCLUDED
