// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/MachineFunctionTable.hpp>
#include <cminor/symbols/FunctionSymbol.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolReader.hpp>
#include <cminor/machine/FileRegistry.hpp>
#include <cminor/util/Unicode.hpp>

namespace cminor { namespace symbols {

using namespace cminor::unicode;

Function* MachineFunctionTable::CreateFunction(FunctionSymbol* functionSymbol)
{
    std::u32string functionCallName = functionSymbol->FullName();
    ConstantPool& constantPool = functionSymbol->GetAssembly()->GetConstantPool();
    Constant functionGroupNameConstant = constantPool.GetConstant(constantPool.Install(functionSymbol->GroupName()));
    Constant functionCallNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(functionCallName.c_str())));
    std::u32string functionFullNameWithSpecifiers = functionSymbol->FullNameWithSpecifiers();
    Constant functionFullNameWithSpecifiersConstant = constantPool.GetConstant(constantPool.Install(StringPtr(functionFullNameWithSpecifiers.c_str())));
    Function* function = new Function(functionGroupNameConstant, functionCallNameConstant, functionFullNameWithSpecifiersConstant, uint32_t(machineFunctions.size()), &constantPool);
    if (functionSymbol->GetSpan().Valid())
    {
        std::u32string sfp = ToUtf32(FileRegistry::GetParsedFileName(functionSymbol->GetSpan().FileIndex()));
        Constant sourceFilePath = constantPool.GetConstant(constantPool.Install(StringPtr(sfp.c_str())));
        function->SetSourceFilePath(sourceFilePath);
    }
    machineFunctions.push_back(std::unique_ptr<Function>(function));
    return function;
}

Function* MachineFunctionTable::GetFunction(uint32_t functionId) const
{
    Assert(functionId < uint32_t(machineFunctions.size()), "invalid function id");
    Function* function = machineFunctions[functionId].get();
    Assert(function, "function not set");
    return function;
}

void MachineFunctionTable::Write(SymbolWriter& writer)
{
    uint32_t n = uint32_t(machineFunctions.size());
    writer.AsMachineWriter().PutEncodedUInt(n);
    for (uint32_t i = 0; i < n; ++i)
    {
        Function* function = machineFunctions[i].get();
        function->Write(writer);
    }
}

void MachineFunctionTable::Read(SymbolReader& reader)
{
    uint32_t n = reader.GetEncodedUInt();
    for (uint32_t i = 0; i < n; ++i)
    {
        std::unique_ptr<Function> function(new Function());
        function->Read(reader);
        machineFunctions.push_back(std::move(function));
    }
}

void MachineFunctionTable::Dump(CodeFormatter& formatter)
{
    formatter.WriteLine("FUNCTION TABLE");
    formatter.WriteLine();
    int32_t n = int32_t(machineFunctions.size());
    for (int32_t i = 0; i < n; ++i)
    {
        Function* function = machineFunctions[i].get();
        function->Dump(formatter);
    }
    formatter.WriteLine();
}

} } // namespace cminor::symbols
