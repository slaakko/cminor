// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_INDEXER_SYMBOL_INCLUDED
#define CMINOR_SYMBOLS_INDEXER_SYMBOL_INCLUDED
#include <cminor/symbols/FunctionSymbol.hpp>

namespace cminor { namespace symbols {

class IndexerGetterFunctionSymbol;
class IndexerSetterFunctionSymbol;

class IndexerSymbol : public ContainerSymbol
{
public:
    IndexerSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::indexerSymbol; }
    std::string TypeString() const override { return "indexer"; }
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void AddSymbol(std::unique_ptr<Symbol>&& symbol) override;
    TypeSymbol* GetValueType()  const { return valueType; }
    void SetValueType(TypeSymbol* valueType_) { valueType = valueType_; }
    TypeSymbol* GetIndexType()  const { return indexType; }
    void SetIndexType(TypeSymbol* indexType_) { indexType = indexType_; }
    void EmplaceType(TypeSymbol* type, int index) override;
    void SetSpecifiers(Specifiers specifiers);
    IndexerGetterFunctionSymbol* Getter() const { return getter; }
    IndexerSetterFunctionSymbol* Setter() const { return setter; }
    void AddTo(ClassTypeSymbol* classTypeSymbol) override;
    void MergeTo(ClassTemplateSpecializationSymbol* classTemplateSpecializationSymbol) override;
    void Merge(const IndexerSymbol& that);
private:
    TypeSymbol* valueType;
    TypeSymbol* indexType;
    IndexerGetterFunctionSymbol* getter;
    IndexerSetterFunctionSymbol* setter;
};

class IndexerGroupSymbol : public Symbol
{
public:
    IndexerGroupSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::indexerGroupSymbol; }
    std::string TypeString() const override { return "indexer group"; }
    bool IsExportSymbol() const override { return false; }
    void AddIndexer(IndexerSymbol* indexer);
    const std::vector<IndexerSymbol*>& Indexers() const { return indexers; }
private:
    std::vector<IndexerSymbol*> indexers;
};

class IndexerGetterFunctionSymbol : public FunctionSymbol
{
public:
    IndexerGetterFunctionSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::indexerGetterSymbol; }
    std::string TypeString() const override { return "getter"; }
    ParameterSymbol* GetThisParam() const override { return Parameters()[0]; }
    void SetOriginalSourceAssembly(Assembly* originalSourceAssembly_) { originalSourceAssembly = originalSourceAssembly_; }
    Assembly* GetOriginalSourceAssembly() const { return originalSourceAssembly; }
private:
    Assembly* originalSourceAssembly;
};

class IndexerSetterFunctionSymbol : public FunctionSymbol
{
public:
    IndexerSetterFunctionSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::indexerSetterSymbol; }
    std::string TypeString() const override { return "setter"; }
    ParameterSymbol* GetThisParam() const override { return Parameters()[0]; }
    void SetOriginalSourceAssembly(Assembly* originalSourceAssembly_) { originalSourceAssembly = originalSourceAssembly_; }
    Assembly* GetOriginalSourceAssembly() const { return originalSourceAssembly; }
private:
    Assembly* originalSourceAssembly;
};


} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_INDEXER_SYMBOL_INCLUDED
