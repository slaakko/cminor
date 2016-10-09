// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/SymbolReader.hpp>
#include <cminor/symbols/Assembly.hpp>

namespace cminor { namespace symbols {

SymbolReader::SymbolReader(Machine& machine_, const std::string& fileName_) : Reader(machine_, fileName_), assembly(nullptr)
{
}

Symbol* SymbolReader::GetSymbol()
{
    SymbolType symbolType = static_cast<SymbolType>(GetByte());
    if (symbolType == SymbolType::basicTypeConversion)
    {
        int x = 0;
    }
    Span span = GetSpan();
    ConstantId id;
    id.Read(*this);
    Constant name = assembly->GetConstantPool().GetConstant(id);
    Symbol* symbol = SymbolFactory::Instance().CreateSymbol(symbolType, span, name);
    symbol->SetAssembly(assembly);
    symbol->Read(*this);
    return symbol;
}

} } // namespace cminor::symbols
