// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/Symbol.hpp>
#include <cminor/symbols/Assembly.hpp>

namespace cminor { namespace symbols {

SymbolWriter::SymbolWriter(const std::string& fileName_) : AstWriter(fileName_), assembly(nullptr)
{
}

void SymbolWriter::Put(Symbol* symbol)
{
    Writer::Put(uint8_t(symbol->GetSymbolType()));
    Writer::Put(symbol->GetSpan());
    ConstantId id = assembly->GetConstantPool().GetIdFor(symbol->NameConstant());
    if (id != noConstantId)
    {
        id.Write(*this);
    }
    else
    {
        throw std::runtime_error("name constant for symbol '" + ToUtf8(symbol->FullName()) + "' not installed to constant pool of assembly '" + ToUtf8(assembly->Name().Value()) + "'");
    }
    symbol->Write(*this);
}

void SymbolWriter::Put(Value* value)
{
    Writer::Put(uint8_t(value->GetValueKind()));
    value->Write(*this);
}

} } // namespace cminor::symbols
