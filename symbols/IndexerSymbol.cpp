// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/IndexerSymbol.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolReader.hpp>

namespace cminor { namespace symbols {

IndexerSymbol::IndexerSymbol(const Span& span_, Constant name_) : ContainerSymbol(span_, name_), valueType(nullptr), indexType(nullptr), getter(nullptr), setter(nullptr)
{
}

void IndexerSymbol::Write(SymbolWriter& writer)
{
    ContainerSymbol::Write(writer);
    utf32_string valueTypeFullName = valueType->FullName();
    ConstantId valueTypeId = GetAssembly()->GetConstantPool().GetIdFor(valueTypeFullName);
    Assert(valueTypeId != noConstantId, "got no id");
    valueTypeId.Write(writer);
    utf32_string indexTypeFullName = indexType->FullName();
    ConstantId indexTypeId = GetAssembly()->GetConstantPool().GetIdFor(indexTypeFullName);
    Assert(indexTypeId != noConstantId, "got no id");
    indexTypeId.Write(writer);
}

void IndexerSymbol::Read(SymbolReader& reader)
{
    ContainerSymbol::Read(reader);
    ConstantId valueTypeId;
    valueTypeId.Read(reader);
    reader.EmplaceTypeRequest(this, valueTypeId, 0);
    ConstantId indexTypeId;
    indexTypeId.Read(reader);
    reader.EmplaceTypeRequest(this, indexTypeId, 1);
}

void IndexerSymbol::EmplaceType(TypeSymbol* type, int index)
{
    if (index == 0)
    {
        valueType = type;
    }
    else if (index == 1)
    {
        indexType = type;
    }
    else
    {
        throw std::runtime_error("indexer symbol emplace type got invalid type index " + std::to_string(index));
    }
}

void IndexerSymbol::AddSymbol(std::unique_ptr<Symbol>&& symbol)
{
    Symbol* s = symbol.get();
    ContainerSymbol::AddSymbol(std::move(symbol));
    if (IndexerGetterFunctionSymbol* getter_ = dynamic_cast<IndexerGetterFunctionSymbol*>(s))
    {
        getter = getter_;
    }
    else if (IndexerSetterFunctionSymbol* setter_ = dynamic_cast<IndexerSetterFunctionSymbol*>(s))
    {
        setter = setter_;
    }
}

void IndexerSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    if ((specifiers & Specifiers::static_) != Specifiers::none)
    {
        SetStatic();
    }
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        throw Exception("indexers cannot be external", GetSpan());
    }
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        throw Exception("indexers cannot be virtual", GetSpan());
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        throw Exception("indexers cannot be override", GetSpan());
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        throw Exception("indexers cannot be abstract", GetSpan());
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        throw Exception("indexers cannot be inline", GetSpan());
    }
    if ((specifiers & Specifiers::new_) != Specifiers::none)
    {
        throw Exception("indexers cannot be new", GetSpan());
    }
}

void IndexerSymbol::AddTo(ClassTypeSymbol* classTypeSymbol)
{
    classTypeSymbol->Add(this);
}

void IndexerSymbol::MergeTo(ClassTemplateSpecializationSymbol* classTemplateSpecializationSymbol)
{
    classTemplateSpecializationSymbol->MergeIndexerSymbol(*this);
}

void IndexerSymbol::Merge(const IndexerSymbol& that)
{
    if (that.Getter())
    {
        Assert(Getter(), "getter expected");
        Getter()->Merge(*that.Getter());
    }
    if (that.Setter())
    {
        Assert(Setter(), "setter expected");
        Setter()->Merge(*that.Setter());
    }
}

IndexerGroupSymbol::IndexerGroupSymbol(const Span& span_, Constant name_) : Symbol(span_, name_)
{
}

void IndexerGroupSymbol::AddIndexer(IndexerSymbol* indexer)
{
    indexers.push_back(indexer);
}

IndexerGetterFunctionSymbol::IndexerGetterFunctionSymbol(const Span& span_, Constant name_) : FunctionSymbol(span_, name_)
{
}

IndexerSetterFunctionSymbol::IndexerSetterFunctionSymbol(const Span& span_, Constant name_) : FunctionSymbol(span_, name_)
{
}

} } // namespace cminor::symbols
