// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/ConstantSymbol.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolReader.hpp>

namespace cminor { namespace symbols {

ConstantSymbol::ConstantSymbol(const Span& span_, Constant name_) : Symbol(span_, name_), type(nullptr), value()
{
}

void ConstantSymbol::SetValue(Constant value_)
{
    ConstantPool& constantPool = GetAssembly()->GetConstantPool();
    ConstantId id = constantPool.Install(value_);
    value = constantPool.GetConstant(id);
}

void ConstantSymbol::Write(SymbolWriter& writer)
{
    ConstantPool& constantPool = GetAssembly()->GetConstantPool();
    ConstantId id = constantPool.GetIdFor(value);
    Assert(id != noConstantId, "got no id for constant");
    id.Write(writer);
}

void ConstantSymbol::Read(SymbolReader& reader)
{
    ConstantPool& constantPool = GetAssembly()->GetConstantPool();
    ConstantId id;
    id.Read(reader);
    value = constantPool.GetConstant(id);
}

ConstantId ConstantSymbol::GetId() const
{
    ConstantPool& constantPool = GetAssembly()->GetConstantPool();
    ConstantId id = constantPool.GetIdFor(value);
    Assert(id != noConstantId, "got no id for constant");
    return id;
}

} } // namespace cminor::symbols
