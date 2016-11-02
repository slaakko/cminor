// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_SYMBOL_INCLUDED
#define CMINOR_SYMBOLS_SYMBOL_INCLUDED
#include <cminor/machine/Constant.hpp>
#include <cminor/machine/Type.hpp>
#include <cminor/ast/Function.hpp>
#include <cminor/ast/Class.hpp>
#include <cminor/ast/Statement.hpp>
#include <cminor/ast/Namespace.hpp>
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
class IndexerGroupSymbol;
class TypeSymbol;
class ClassTypeSymbol;
class SymbolTable;
class MemberVariableSymbol;
class ConstructorSymbol;
class MemberFunctionSymbol;

enum class SymbolType : uint8_t
{
    boolTypeSymbol, charTypeSymbol, voidTypeSymbol, sbyteTypeSymbol, byteTypeSymbol, shortTypeSymbol, ushortTypeSymbol, intTypeSymbol, uintTypeSymbol, longTypeSymbol, ulongTypeSymbol,
    floatTypeSymbol, doubleTypeSymbol, nullReferenceTypeSymbol,
    classTypeSymbol, arrayTypeSymbol, functionSymbol, staticConstructorSymbol, constructorSymbol, arraySizeConstructorSymbol, memberFunctionSymbol, functionGroupSymbol, parameterSymbol,
    localVariableSymbol, memberVariableSymbol, propertySymbol, propertyGetterSymbol, propertySetterSymbol, indexerSymbol, indexerGetterSymbol, indexerSetterSymbol, indexerGroupSymbol,
    constantSymbol, namespaceSymbol, declarationBlock, 
    basicTypeDefaultInit, basicTypeCopyInit, basicTypeAssignment, basicTypeReturn, basicTypeConversion, basicTypeUnaryOp, basicTypBinaryOp, objectDefaultInit, objectCopyInit, objectNullInit, 
    objectAssignment, objectNullAssignment, classTypeConversion,
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
    project = 1 << 3,
    bound = 1 << 4
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

std::string SymbolFlagStr(SymbolFlags flags);

class Symbol
{
public:
    Symbol(const Span& span_, Constant name_);
    virtual ~Symbol();
    virtual SymbolType GetSymbolType() const = 0;
    virtual std::string TypeString() const { return "symbol";  }
    virtual bool IsExportSymbol() const;
    virtual void Write(SymbolWriter& writer);
    virtual void Read(SymbolReader& reader);
    const Span& GetSpan() const { return span; }
    void SetSpan(const Span& span_) { span = span_; }
    StringPtr Name() const;
    void SetName(StringPtr newName);
    virtual utf32_string FullName() const;
    Constant NameConstant() const { return name; }
    SymbolAccess Access() const { return SymbolAccess(flags & SymbolFlags::access); }
    virtual SymbolAccess DeclaredAccess() const { return Access(); }
    void SetAccess(SymbolAccess access_) { flags = flags | SymbolFlags(access_); }
    void SetAccess(Specifiers accessSpecifiers);
    bool IsStatic() const { return GetFlag(SymbolFlags::static_); }
    void SetStatic() { SetFlag(SymbolFlags::static_); }
    bool IsBound() const { return GetFlag(SymbolFlags::bound); }
    void SetBound() { SetFlag(SymbolFlags::bound); }
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
    bool IsSameParentOrAncestorOf(const Symbol* that) const;
    virtual ContainerScope* GetContainerScope() { return nullptr; }
    NamespaceSymbol* Ns() const;
    ClassTypeSymbol* Class() const;
    ClassTypeSymbol* ClassNoThrow() const;
    ContainerSymbol* ClassOrNs() const;
    FunctionSymbol* GetFunction() const;
    FunctionSymbol* ContainingFunction() const;
    ClassTypeSymbol* ContainingClass() const;
    ContainerScope* NsScope() const;
    ContainerScope* ClassOrNsScope() const;
    SymbolFlags Flags() const { return flags; }
    bool GetFlag(SymbolFlags flag) const { return (flags & flag) != SymbolFlags::none; }
    void SetFlag(SymbolFlags flag) { flags = flags | flag; }
    void ResetFlag(SymbolFlags flag) { flags = flags & ~flag; }
    virtual void EmplaceType(TypeSymbol* type, int index);
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
    void CollectViableFunctions(int arity, StringPtr groupName, std::unordered_set<ContainerScope*>& scopesLookedUp, ScopeLookup scopeLookup, std::unordered_set<FunctionSymbol*>& viableFunctions);
    NamespaceSymbol* Ns() const;
    NamespaceSymbol* CreateNamespace(StringPtr qualifiedNsName, const Span& span);
    void Clear();
private:
    ContainerScope* base;
    ContainerScope* parent;
    ContainerSymbol* container;
    std::unordered_map<StringPtr, Symbol*, StringPtrHash> symbolMap;
};

class FileScope : public Scope
{
public:
    void InstallAlias(ContainerScope* containerScope, AliasNode* aliasNode);
    void InstallNamespaceImport(ContainerScope* containerScope, NamespaceImportNode* namespaceImportNode);
    Symbol* Lookup(StringPtr name) const override;
    Symbol* Lookup(StringPtr name, ScopeLookup lookup) const override;
    void CollectViableFunctions(int arity, StringPtr groupName, std::unordered_set<ContainerScope*>& scopesLookedUp, std::unordered_set<FunctionSymbol*>& viableFunctions);
private:
    std::vector<ContainerScope*> containerScopes;
    std::unordered_map<StringPtr, Symbol*, StringPtrHash> aliasSymbolMap;
};

class ContainerSymbol : public Symbol
{
public:
    ContainerSymbol(const Span& span_, Constant name_);
    std::string TypeString() const override { return "container symbol"; }
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    virtual void AddSymbol(std::unique_ptr<Symbol>&& symbol);
    ContainerScope* GetContainerScope() { return &containerScope; }
    const std::vector<std::unique_ptr<Symbol>>& Symbols() const { return symbols; }
    std::vector<std::unique_ptr<Symbol>>& Symbols() { return symbols; }
    void Clear();
    FunctionGroupSymbol* MakeFunctionGroupSymbol(StringPtr groupName, const Span& span);
    IndexerGroupSymbol* MakeIndexerGroupSymbol(const Span& span);
private:
    ContainerScope containerScope;
    std::vector<std::unique_ptr<Symbol>> symbols;
};

class NamespaceSymbol : public ContainerSymbol
{
public:
    NamespaceSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::namespaceSymbol; }
    std::string TypeString() const override { return "namespace"; }
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void Import(NamespaceSymbol* that, SymbolTable& symbolTable);
};

class DeclarationBlock : public ContainerSymbol
{
public:
    DeclarationBlock(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::declarationBlock; }
    std::string TypeString() const override { return "declaration block"; }
    void AddSymbol(std::unique_ptr<Symbol>&& symbol) override;
};

class TypeSymbol : public ContainerSymbol
{
public:
    TypeSymbol(const Span& span_, Constant name_);
    std::string TypeString() const override { return "type symbol"; }
    virtual bool IsUnsignedType() const { return false; }
    virtual ValueType GetValueType() const { return ValueType::none; }
    virtual bool IsAbstract() const { return false; }
    virtual Type* GetMachineType() const = 0;
    virtual bool IsInComplete() const { return false; }
};

class BasicTypeSymbol : public TypeSymbol
{
public:
    BasicTypeSymbol(const Span& span_, Constant name_);
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    std::string TypeString() const override { return "basic type"; }
    Type* GetMachineType() const override { return machineType.get(); }
    void SetMachineType(BasicType* machineType_);
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
private:
    std::unique_ptr<BasicType> machineType;
};

class BoolTypeSymbol : public BasicTypeSymbol
{
public:
    BoolTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::boolTypeSymbol; }
    std::string TypeString() const override { return "bool type"; }
    ValueType GetValueType() const override { return ValueType::boolType; }
};

class CharTypeSymbol : public BasicTypeSymbol
{
public:
    CharTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::charTypeSymbol; }
    std::string TypeString() const override { return "char type"; }
    ValueType GetValueType() const override { return ValueType::charType; }
};

class VoidTypeSymbol : public BasicTypeSymbol
{
public:
    VoidTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::voidTypeSymbol; }
    std::string TypeString() const override { return "void type"; }
};

class SByteTypeSymbol : public BasicTypeSymbol
{
public:
    SByteTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::sbyteTypeSymbol; }
    std::string TypeString() const override { return "sbyte type"; }
    ValueType GetValueType() const override { return ValueType::sbyteType; }
};

class ByteTypeSymbol : public BasicTypeSymbol
{
public:
    ByteTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::byteTypeSymbol; }
    bool IsUnsignedType() const override { return true; }
    std::string TypeString() const override { return "byte type"; }
    ValueType GetValueType() const override { return ValueType::byteType; }
};

class ShortTypeSymbol : public BasicTypeSymbol
{
public:
    ShortTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::shortTypeSymbol; }
    std::string TypeString() const override { return "short type"; }
    ValueType GetValueType() const override { return ValueType::shortType; }
};

class UShortTypeSymbol : public BasicTypeSymbol
{
public:
    UShortTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::ushortTypeSymbol; }
    bool IsUnsignedType() const override { return true; }
    std::string TypeString() const override { return "ushort type"; }
    ValueType GetValueType() const override { return ValueType::ushortType; }
};

class IntTypeSymbol : public BasicTypeSymbol
{
public:
    IntTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::intTypeSymbol; }
    std::string TypeString() const override { return "int type"; }
    ValueType GetValueType() const override { return ValueType::intType; }
};

class UIntTypeSymbol : public BasicTypeSymbol
{
public:
    UIntTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::uintTypeSymbol; }
    bool IsUnsignedType() const override { return true; }
    std::string TypeString() const override { return "uint type"; }
    ValueType GetValueType() const override { return ValueType::uintType; }
};

class LongTypeSymbol : public BasicTypeSymbol
{
public:
    LongTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::longTypeSymbol; }
    std::string TypeString() const override { return "long type"; }
    ValueType GetValueType() const override { return ValueType::longType; }
};

class ULongTypeSymbol : public BasicTypeSymbol
{
public:
    ULongTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::ulongTypeSymbol; }
    bool IsUnsignedType() const override { return true; }
    std::string TypeString() const override { return "ulong type"; }
    ValueType GetValueType() const override { return ValueType::ulongType; }
};

class FloatTypeSymbol : public BasicTypeSymbol
{
public:
    FloatTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::floatTypeSymbol; }
    std::string TypeString() const override { return "float type"; }
    ValueType GetValueType() const override { return ValueType::floatType; }
};

class DoubleTypeSymbol : public BasicTypeSymbol
{
public:
    DoubleTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::doubleTypeSymbol; }
    std::string TypeString() const override { return "double type"; }
    ValueType GetValueType() const override { return ValueType::doubleType; }
};

class NullReferenceTypeSymbol : public BasicTypeSymbol
{
public:
    NullReferenceTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::nullReferenceTypeSymbol; }
    std::string TypeString() const override { return "null reference type"; }
};

bool Overrides(MemberFunctionSymbol* f, MemberFunctionSymbol* g);

enum class ClassTypeSymbolFlags : uint8_t
{
    none = 0,
    abstract_ = 1 << 0,
    nonLeaf = 1 << 1
};

inline ClassTypeSymbolFlags operator&(ClassTypeSymbolFlags left, ClassTypeSymbolFlags right)
{
    return ClassTypeSymbolFlags(uint8_t(left) & uint8_t(right));
}

inline ClassTypeSymbolFlags operator|(ClassTypeSymbolFlags left, ClassTypeSymbolFlags right)
{
    return ClassTypeSymbolFlags(uint8_t(left) | uint8_t(right));
}

inline ClassTypeSymbolFlags operator~(ClassTypeSymbolFlags flag)
{
    return ClassTypeSymbolFlags(~uint8_t(flag));
}

class ClassTypeSymbol : public TypeSymbol
{
public:
    ClassTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::classTypeSymbol; }
    std::string TypeString() const override { return "class"; }
    ValueType GetValueType() const override { return ValueType::objectReference; }
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void SetSpecifiers(Specifiers specifiers); 
    void AddSymbol(std::unique_ptr<Symbol>&& symbol) override;
    bool HasBaseClass(ClassTypeSymbol* cls) const;
    bool HasBaseClass(ClassTypeSymbol* cls, int& distance) const;
    ClassTypeSymbol* BaseClass() const { return baseClass; }
    void SetBaseClass(ClassTypeSymbol* baseClass_) { baseClass = baseClass_; }
    void SetObjectType(ObjectType* objectType_) { objectType.reset(objectType_); }
    ObjectType* GetObjectType() const { return objectType.get(); }
    ClassData* GetClassData() const { return classData.get(); }
    Type* GetMachineType() const override { return objectType.get(); }
    const std::vector<MemberVariableSymbol*>& MemberVariables() const { return memberVariables; }
    const std::vector<MemberVariableSymbol*>& StaticMemberVariables() const { return staticMemberVariables; }
    const std::vector<MemberFunctionSymbol*>& MemberFunctions() const { return memberFunctions; }
    const std::vector<ConstructorSymbol*>& Constructors() const { return constructors; }
    ConstructorSymbol* DefaultConstructorSymbol() const { return defaultConstructorSymbol; }
    bool IsAbstract() const override { return GetFlag(ClassTypeSymbolFlags::abstract_); }
    void SetAbstract() { SetFlag(ClassTypeSymbolFlags::abstract_); }
    void EmplaceType(TypeSymbol* type, int index) override;
    int Level() const { return level; }
    void SetLevel(int level_) { level = level_; }
    int Priority() const { return priority; }
    void SetPriority(int priority_) { priority = priority_; }
    void SetNonLeaf() { SetFlag(ClassTypeSymbolFlags::nonLeaf); }
    bool IsNonLeaf() const { return GetFlag(ClassTypeSymbolFlags::nonLeaf); }
    uint64_t Key() const { return key; }
    void SetKey(uint64_t key_) { key = key_; }
    uint64_t Cid() const { return cid; }
    void SetCid(uint64_t cid_);
    void InitVmt();
    void LinkVmt();
private:
    ClassTypeSymbol* baseClass;
    std::unique_ptr<ObjectType> objectType;
    std::unique_ptr<ClassData> classData;
    ClassTypeSymbolFlags flags;
    std::vector<MemberVariableSymbol*> memberVariables;
    std::vector<MemberVariableSymbol*> staticMemberVariables;
    std::vector<MemberFunctionSymbol*> memberFunctions;
    std::vector<ConstructorSymbol*> constructors;
    ConstructorSymbol* defaultConstructorSymbol;
    int level;
    int priority;
    uint64_t key;
    uint64_t cid;
    bool GetFlag(ClassTypeSymbolFlags flag) const
    {
        return (flags & flag) != ClassTypeSymbolFlags::none;
    }
    void SetFlag(ClassTypeSymbolFlags flag)
    {
        flags = flags | flag;
    }
    void InitVmt(std::vector<MemberFunctionSymbol*>& vmt);
};

class ArrayTypeSymbol : public ClassTypeSymbol
{
public:
    ArrayTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::arrayTypeSymbol; }
    std::string TypeString() const override { return "array"; }
    void SetElementType(TypeSymbol* elementType_) { elementType = elementType_; }
    TypeSymbol* ElementType() const { return elementType; }
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void EmplaceType(TypeSymbol* type, int index);
private:
    TypeSymbol* elementType;
};

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_SYMBOL_INCLUDED
