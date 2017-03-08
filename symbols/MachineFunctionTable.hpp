// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_MACHINE_FUNCTION_TABLE_INCLUDED
#define CMINOR_SYMBOLS_MACHINE_FUNCTION_TABLE_INCLUDED
#include <cminor/machine/Function.hpp>

namespace cminor { namespace symbols {

using namespace cminor::machine;

class FunctionSymbol;
class SymbolWriter;
class SymbolReader;

class MachineFunctionTable
{
public:
    Function* CreateFunction(FunctionSymbol* functionSymbol);
    Function* GetFunction(uint32_t functionId) const;
    void Write(SymbolWriter& writer);
    void Read(SymbolReader& reader);
    void Dump(CodeFormatter& formatter);
    const std::vector<std::unique_ptr<Function>>& MachineFunctions() const { return machineFunctions; }
    std::vector<std::unique_ptr<Function>>& MachineFunctions() { return machineFunctions; }
private:
    std::vector<std::unique_ptr<Function>> machineFunctions;
    void InstallNameConstants(FunctionSymbol* functionSymbol);
};

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_MACHINE_FUNCTION_TABLE_INCLUDED
