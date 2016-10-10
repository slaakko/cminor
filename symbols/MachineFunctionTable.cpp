// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/MachineFunctionTable.hpp>
#include <cminor/symbols/FunctionSymbol.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolReader.hpp>

namespace cminor { namespace symbols {

Function* MachineFunctionTable::CreateFunction(FunctionSymbol* functionSymbol)
{
    utf32_string functionFullName = functionSymbol->FullName();
    ConstantPool& constantPool = functionSymbol->GetAssembly()->GetConstantPool();
    Constant functionFullNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(functionFullName.c_str())));
    Function* function = new Function(functionFullNameConstant, int32_t(machineFunctions.size()), &constantPool);
    machineFunctions.push_back(std::unique_ptr<Function>(function));
    return function;
}

Function* MachineFunctionTable::GetFunction(int32_t functionId) const
{
    Assert(functionId < int32_t(machineFunctions.size()), "invalid function id");
    return machineFunctions[functionId].get();
}

void MachineFunctionTable::Write(SymbolWriter& writer)
{
    int32_t n = int32_t(machineFunctions.size());
    static_cast<Writer&>(writer).Put(n);
    for (int32_t i = 0; i < n; ++i)
    {
        Function* function = machineFunctions[i].get();
        function->Write(writer);
    }
}

void MachineFunctionTable::Read(SymbolReader& reader)
{
    int32_t n = reader.GetInt();
    for (int32_t i = 0; i < n; ++i)
    {
        std::unique_ptr<Function> function(new Function());
        function->Read(reader);
        machineFunctions.push_back(std::move(function));
    }
}

} } // namespace cminor::symbols
