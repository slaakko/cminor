// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_SYMBOL_INCLUDED
#define CMINOR_SYMBOLS_SYMBOL_INCLUDED
#include <cminor/machine/Constant.hpp>
#include <cminor/ast/Function.hpp>
#include <cminor/ast/Statement.hpp>
#include <unordered_map>

namespace cminor { namespace symbols {

using namespace cminor::machine;
using namespace cminor::ast;

class Assembly;
class SymbolWriter;
class SymbolReader;
class ContainerScope;
class ContainerSymbol;
class NamespaceSymbol;
class FunctionSymbol;
class FunctionGroupSymbol;
class SymbolTable;

enum class SymbolType : uint8_t
{
    boolTypeSymbol, charTypeSymbol, voidTypeSymbol, sbyteTypeSymbol, byteTypeSymbol, shortTypeSymbol, ushortTypeSymbol, intTypeSymbol, uintTypeSymbol, longTypeSymbol, ulongTypeSymbol,
    floatTypeSymbol, doubleTypeSymbol,
    classTypeSymbol, functionSymbol, functionGroupSymbol, parameterSymbol, localVariableSymbol, memberVariableSymbol, namespaceSymbol, declarationBlock,
    maxSymbol
};

std::string SymbolTypeStr(SymbolType symbolType);

enum class SymbolSource
{
    project, library
};

enum class SymbolAccess : uint8_t
{
    private_ = 0, protected_ = 1, internal_ = 2, public_ = 3
};

enum class SymbolFlags : uint8_t
{
    none = 0,
    access = 1 << 0 | 1 << 1,
    static_ = 1 << 2,
    project = 1 << 3
};

inline SymbolFlags operator~(SymbolFlags flag)
{
    return SymbolFlags(~uint8_t(flag));
}

inline SymbolFlags operator|(SymbolFlags left, SymbolFlags right)
{
    return SymbolFlags(uint8_t(left) | uint8_t(right));
}

inline SymbolFlags operator&(SymbolFlags left, SymbolFlags right)
{
    return SymbolFlags(uint8_t(left) & uint8_t(right));
}

class Symbol
{
public:
    Symbol(const Span& span_, Constant name_);
    virtual ~Symbol();
    virtual SymbolType GetSymbolType() const = 0;
    virtual Symbol* ToSymbol() const { return const_cast<Symbol*>(this); }
    virtual bool IsExportSymbol() const;
    virtual void Write(SymbolWriter& writer);
    virtual void Read(SymbolReader& reader);
    const Span& GetSpan() const { return span; }
    void SetSpan(const Span& span_) { span = span_; }
    StringPtr Name() const;
    void SetName(StringPtr newName);
    utf32_string FullName() const;
    Constant NameConstant() const { return name; }
    SymbolAccess Access() const { return SymbolAccess(flags & SymbolFlags::access); }
    virtual SymbolAccess DeclaredAccess() const { return Access(); }
    void SetAccess(SymbolAccess access_) { flags = flags | SymbolFlags(access_); }
    bool IsStatic() const { return GetFlag(SymbolFlags::static_); }
    void SetStatic() { SetFlag(SymbolFlags::static_); }
    bool IsPublic() const { return Access() == SymbolAccess::public_; }
    void SetPublic() { SetAccess(SymbolAccess::public_); }
    SymbolSource Source() const { return GetFlag(SymbolFlags::project) ? SymbolSource::project : SymbolSource::library; }
    void SetSource(SymbolSource source) { if (source == SymbolSource::project) SetFlag(SymbolFlags::project); else ResetFlag(SymbolFlags::project); }
    bool IsProject() const { return GetFlag(SymbolFlags::project); }
    void SetProject() { SetFlag(SymbolFlags::project); }
    Symbol* Parent() const { return parent; }
    void SetParent(Symbol* parent_) { parent = parent_; }
    Assembly* GetAssembly() const { Assert(assembly, "symbol's assembly not set");  return assembly; }
    void SetAssembly(Assembly* assembly_) { assembly = assembly_; }
    virtual ContainerScope* GetContainerScope() { return nullptr; }
    NamespaceSymbol* Ns() const;
    SymbolFlags Flags() const { return flags; }
    bool GetFlag(SymbolFlags flag) const { return (flags & flag) != SymbolFlags::none; }
    void SetFlag(SymbolFlags flag) { flags = flags | flag; }
    void ResetFlag(SymbolFlags flag) { flags = flags & ~flag; }
private:
    Span span;
    Constant name;
    SymbolFlags flags;
    Symbol* parent;
    Assembly* assembly;
};

enum class ScopeLookup : uint8_t
{
    none = 0,
    this_ = 1 << 0,
    base = 1 << 1,
    parent = 1 << 2,
    this_and_base = this_ | base,
    this_and_parent = this_ | parent,
    this_and_base_and_parent = this_ | base | parent,
    fileScopes = 1 << 3
};

inline ScopeLookup operator&(ScopeLookup left, ScopeLookup right)
{
    return ScopeLookup(uint8_t(left) & uint8_t(right));
}

inline ScopeLookup operator~(ScopeLookup subject)
{
    return ScopeLookup(~uint8_t(subject));
}

class Scope
{
public:
    virtual ~Scope();
    virtual Symbol* Lookup(StringPtr name) const = 0;
    virtual Symbol* Lookup(StringPtr name, ScopeLookup lookup) const = 0;
};

class ContainerScope : public Scope
{
public:
    ContainerScope();
    ContainerScope* Base() const { return base; }
    void SetBase(ContainerScope* base_) { base = base_; }
    ContainerScope* Parent() const { return parent; }
    void SetParent(ContainerScope* parent_) { parent = parent_; }
    ContainerSymbol* Container() { return container; }
    void SetContainer(ContainerSymbol* container_) { container = container_; }
    void Install(Symbol* symbol);
    Symbol* Lookup(StringPtr name) const override;
    Symbol* Lookup(StringPtr name, ScopeLookup lookup) const override;
    Symbol* LookupQualified(const std::vector<utf32_string>& components, ScopeLookup lookup) const;
    NamespaceSymbol* Ns() const;
    NamespaceSymbol* CreateNamespace(StringPtr qualifiedNsName, const Span& span);
    void Clear();
private:
    ContainerScope* base;
    ContainerScope* parent;
    ContainerSymbol* container;
    std::unordered_map<StringPtr, Symbol*, StringPtrHash> symbolMap;
};

class ContainerSymbol : public Symbol
{
public:
    ContainerSymbol(const Span& span_, Constant name_);
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    virtual void AddSymbol(std::unique_ptr<Symbol>&& symbol);
    ContainerScope* GetContainerScope() { return &containerScope; }
    const std::vector<std::unique_ptr<Symbol>>& Symbols() const { return symbols; }
    std::vector<std::unique_ptr<Symbol>>& Symbols() { return symbols; }
    void Clear();
    FunctionGroupSymbol* MakeFunctionGroupSymbol(StringPtr groupName, const Span& span);
private:
    ContainerScope containerScope;
    std::vector<std::unique_ptr<Symbol>> symbols;
};

class NamespaceSymbol : public ContainerSymbol
{
public:
    NamespaceSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::namespaceSymbol; }
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void Import(NamespaceSymbol* that, SymbolTable& symbolTable);
};

class DeclarationBlock : public ContainerSymbol
{
public:
    DeclarationBlock(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::declarationBlock; }
    bool IsExportSymbol() const override { return false; }
};

class TypeSymbol : public Symbol
{
public:
    TypeSymbol(const Span& span_, Constant name_);
};

class BasicTypeSymbol : public TypeSymbol
{
public:
    BasicTypeSymbol(const Span& span_, Constant name_);
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
};

class BoolTypeSymbol : public BasicTypeSymbol
{
public:
    BoolTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::boolTypeSymbol; }
};

class CharTypeSymbol : public BasicTypeSymbol
{
public:
    CharTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::charTypeSymbol; }
};

class VoidTypeSymbol : public BasicTypeSymbol
{
public:
    VoidTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::voidTypeSymbol; }
};

class SByteTypeSymbol : public BasicTypeSymbol
{
public:
    SByteTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::sbyteTypeSymbol; }
};

class ByteTypeSymbol : public BasicTypeSymbol
{
public:
    ByteTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::byteTypeSymbol; }
};

class ShortTypeSymbol : public BasicTypeSymbol
{
public:
    ShortTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::shortTypeSymbol; }
};

class UShortTypeSymbol : public BasicTypeSymbol
{
public:
    UShortTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::ushortTypeSymbol; }
};

class IntTypeSymbol : public BasicTypeSymbol
{
public:
    IntTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::intTypeSymbol; }
};

class UIntTypeSymbol : public BasicTypeSymbol
{
public:
    UIntTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::uintTypeSymbol; }
};

class LongTypeSymbol : public BasicTypeSymbol
{
public:
    LongTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::longTypeSymbol; }
};

class ULongTypeSymbol : public BasicTypeSymbol
{
public:
    ULongTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::ulongTypeSymbol; }
};

class FloatTypeSymbol : public BasicTypeSymbol
{
public:
    FloatTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::floatTypeSymbol; }
};

class DoubleTypeSymbol : public BasicTypeSymbol
{
public:
    DoubleTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::doubleTypeSymbol; }
};

class ClassTypeSymbol : public TypeSymbol, public ContainerSymbol
{
public:
    ClassTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::classTypeSymbol; }
    Symbol* ToSymbol() const { return TypeSymbol::ToSymbol(); }
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
};

class SymbolTable
{
public:
    SymbolTable(Assembly* assembly_);
    NamespaceSymbol& GlobalNs() { return globalNs; }
    const NamespaceSymbol& GlobalNs() const { return globalNs; }
    ContainerSymbol* Container() const { return container; }
    void BeginContainer(ContainerSymbol* container_);
    void EndContainer();
    void BeginNamespace(StringPtr namespaceName, const Span& span);
    void EndNamespace();
    void BeginFunction(FunctionNode& functionNode);
    void EndFunction();
    void AddParameter(ParameterNode& parameterNode);
    void BeginDeclarationBlock(StatementNode& statementNode);
    void EndDeclarationBlock();
    void AddLocalVariable(ConstructionStatementNode& constructionStatementNode);
    void Write(SymbolWriter& writer);
    void Read(SymbolReader& reader);
    void Import(SymbolTable& symbolTable);
    void Clear();
    TypeSymbol* GetTypeNoThrow(const utf32_string& typeFullName) const;
    TypeSymbol* GetType(const utf32_string& typeFullName) const;
    void AddType(TypeSymbol* type);
private:
    Assembly* assembly;
    NamespaceSymbol globalNs;
    ContainerSymbol* container;
    FunctionSymbol* function;
    FunctionSymbol* mainFunction;
    std::stack<ContainerSymbol*> containerStack;
    std::unordered_map<Constant, TypeSymbol*, ConstantHash> typeSymbolMap;
};

class SymbolCreator
{
public:
    virtual ~SymbolCreator();
    virtual Symbol* CreateSymbol(const Span& span, Constant name) = 0;
};

class SymbolFactory
{
public:
    SymbolFactory();
    static void Init();
    static void Done();
    static SymbolFactory& Instance();
    void Register(SymbolType symbolType, SymbolCreator* creator);
    Symbol* CreateSymbol(SymbolType symbolType, const Span& span, Constant name);
private:
    static std::unique_ptr<SymbolFactory> instance;
    std::vector<std::unique_ptr<SymbolCreator>> creators;
};

void InitSymbol();
void DoneSymbol();

std::unique_ptr<Assembly> CreateSystemAssembly(const std::string& config);

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_SYMBOL_INCLUDED
