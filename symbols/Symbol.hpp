// =================================
// Copyright (c) 2017 Seppo Laakko
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
class InterfaceTypeSymbol;
class SymbolTable;
class MemberVariableSymbol;
class ConstructorSymbol;
class StaticConstructorSymbol;
class MemberFunctionSymbol;
class ClassTemplateSpecializationSymbol;
class PropertySymbol;
class IndexerSymbol;
class ConstantSymbol;
class EnumConstantSymbol;

enum class SymbolType : uint8_t
{
    boolTypeSymbol, charTypeSymbol, voidTypeSymbol, sbyteTypeSymbol, byteTypeSymbol, shortTypeSymbol, ushortTypeSymbol, intTypeSymbol, uintTypeSymbol, longTypeSymbol, ulongTypeSymbol,
    floatTypeSymbol, doubleTypeSymbol, nullReferenceTypeSymbol,
    classTypeSymbol, arrayTypeSymbol, interfaceTypeSymbol, functionSymbol, staticConstructorSymbol, constructorSymbol, arraySizeConstructorSymbol, memberFunctionSymbol, functionGroupSymbol, 
    parameterSymbol, localVariableSymbol, memberVariableSymbol, propertySymbol, propertyGetterSymbol, propertySetterSymbol, indexerSymbol, indexerGetterSymbol, indexerSetterSymbol, 
    indexerGroupSymbol, constantSymbol, namespaceSymbol, declarationBlock, typeParameterSymbol, boundTypeParameterSymbol, classTemplateSpecializationSymbol,
    basicTypeDefaultInit, basicTypeCopyInit, basicTypeAssignment, basicTypeReturn, basicTypeConversion, basicTypeUnaryOp, basicTypBinaryOp, objectDefaultInit, objectCopyInit, objectNullInit, 
    objectAssignment, objectNullAssignment, objectNullEqual, nullObjectEqual, nullToObjectConversion, classTypeConversion, classToInterfaceConversion, enumTypeSymbol, enumConstantSymbol,
    enumTypeDefaultInit, enumTypeConversion,
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
    bound = 1 << 4,
    instantiated = 1 << 5,
    instantiationRequested = 1 << 6
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

extern const uint32_t firstFreeSymbolId;

class SymbolEvaluator
{
public:
    virtual ~SymbolEvaluator();
    virtual void EvaluateContainerSymbol(ContainerSymbol* containerSymbol) = 0;
    virtual void EvaluateConstantSymbol(ConstantSymbol* constantSymbol) = 0;
    virtual void EvaluateEnumConstantSymbol(EnumConstantSymbol* enumConstantSymbol) = 0;
};

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
    void SetInstantiationRequested() { SetFlag(SymbolFlags::instantiationRequested); }
    bool IsInstantiationRequested() const { return GetFlag(SymbolFlags::instantiationRequested); }
    void SetInstantiated() { SetFlag(SymbolFlags::instantiated); }
    bool IsInstantiated() const { return GetFlag(SymbolFlags::instantiated); }
    Symbol* Parent() const { return parent; }
    void SetParent(Symbol* parent_) { parent = parent_; }
    Assembly* GetAssembly() const { Assert(assembly, "symbol's assembly not set");  return assembly; }
    void SetAssembly(Assembly* assembly_) { assembly = assembly_; }
    bool IsSameParentOrAncestorOf(const Symbol* that) const;
    virtual ContainerScope* GetContainerScope() { return nullptr; }
    NamespaceSymbol* Ns() const;
    ClassTypeSymbol* Class() const;
    ClassTypeSymbol* ClassNoThrow() const;
    InterfaceTypeSymbol* InterfaceNoThrow() const;
    ContainerSymbol* ClassOrNs() const;
    ContainerSymbol* ClassInterfaceOrNs() const;
    FunctionSymbol* GetFunction() const;
    FunctionSymbol* ContainingFunction() const;
    ClassTypeSymbol* ContainingClass() const;
    InterfaceTypeSymbol* ContainingInterface() const;
    ContainerScope* NsScope() const;
    ContainerScope* ClassOrNsScope() const;
    ContainerScope* ClassInterfaceOrNsScope() const;
    SymbolFlags Flags() const { return flags; }
    bool GetFlag(SymbolFlags flag) const { return (flags & flag) != SymbolFlags::none; }
    void SetFlag(SymbolFlags flag) { flags = flags | flag; }
    void ResetFlag(SymbolFlags flag) { flags = flags & ~flag; }
    virtual void EmplaceType(TypeSymbol* type, int index);
    uint32_t Id() const { return id; }
    void SetId(uint32_t id_) { id = id_; }
    bool HasId() const { return id != noSymbolId; }
    virtual void AddTo(ClassTypeSymbol* classTypeSymbol);
    virtual void MergeTo(ClassTemplateSpecializationSymbol* classTemplateSpecializationSymbol);
    void Merge(const Symbol& that);
    virtual void Evaluate(SymbolEvaluator* evaluator, const Span& span);
private:
    Span span;
    Constant name;
    SymbolFlags flags;
    Symbol* parent;
    Assembly* assembly;
    uint32_t id;
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
    void Evaluate(SymbolEvaluator* evaluator, const Span& span) override;
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
    bool IsGlobalNamespace() const { return Name() == U""; }
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
    virtual bool IsReopenedClassTemplateSpecialization() const { return false; }
    virtual bool HasBoxedType() const { return false; }
    virtual std::string GetBoxedTypeName() const { return std::string(); }
    virtual bool IsSwitchConditionType() const { return false; }
    virtual bool IsVoidType() const { return false; }
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
    bool HasBoxedType() const override { return true; }
    bool IsSwitchConditionType() const override { return true; }
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
    std::string GetBoxedTypeName() const override { return "System.BoxedBoolean"; }
};

class CharTypeSymbol : public BasicTypeSymbol
{
public:
    CharTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::charTypeSymbol; }
    std::string TypeString() const override { return "char type"; }
    ValueType GetValueType() const override { return ValueType::charType; }
    std::string GetBoxedTypeName() const override { return "System.BoxedChar"; }
};

class VoidTypeSymbol : public BasicTypeSymbol
{
public:
    VoidTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::voidTypeSymbol; }
    std::string TypeString() const override { return "void type"; }
    bool HasBoxedType() const override { return false; }
    bool IsSwitchConditionType() const override { return false; }
    bool IsVoidType() const override { return true; }
};

class SByteTypeSymbol : public BasicTypeSymbol
{
public:
    SByteTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::sbyteTypeSymbol; }
    std::string TypeString() const override { return "sbyte type"; }
    ValueType GetValueType() const override { return ValueType::sbyteType; }
    std::string GetBoxedTypeName() const override { return "System.BoxedInt8"; }
};

class ByteTypeSymbol : public BasicTypeSymbol
{
public:
    ByteTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::byteTypeSymbol; }
    bool IsUnsignedType() const override { return true; }
    std::string TypeString() const override { return "byte type"; }
    ValueType GetValueType() const override { return ValueType::byteType; }
    std::string GetBoxedTypeName() const override { return "System.BoxedUInt8"; }
};

class ShortTypeSymbol : public BasicTypeSymbol
{
public:
    ShortTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::shortTypeSymbol; }
    std::string TypeString() const override { return "short type"; }
    ValueType GetValueType() const override { return ValueType::shortType; }
    std::string GetBoxedTypeName() const override { return "System.BoxedInt16"; }
};

class UShortTypeSymbol : public BasicTypeSymbol
{
public:
    UShortTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::ushortTypeSymbol; }
    bool IsUnsignedType() const override { return true; }
    std::string TypeString() const override { return "ushort type"; }
    ValueType GetValueType() const override { return ValueType::ushortType; }
    std::string GetBoxedTypeName() const override { return "System.BoxedUInt16"; }
};

class IntTypeSymbol : public BasicTypeSymbol
{
public:
    IntTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::intTypeSymbol; }
    std::string TypeString() const override { return "int type"; }
    ValueType GetValueType() const override { return ValueType::intType; }
    std::string GetBoxedTypeName() const override { return "System.BoxedInt32"; }
};

class UIntTypeSymbol : public BasicTypeSymbol
{
public:
    UIntTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::uintTypeSymbol; }
    bool IsUnsignedType() const override { return true; }
    std::string TypeString() const override { return "uint type"; }
    ValueType GetValueType() const override { return ValueType::uintType; }
    std::string GetBoxedTypeName() const override { return "System.BoxedUInt32"; }
};

class LongTypeSymbol : public BasicTypeSymbol
{
public:
    LongTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::longTypeSymbol; }
    std::string TypeString() const override { return "long type"; }
    ValueType GetValueType() const override { return ValueType::longType; }
    std::string GetBoxedTypeName() const override { return "System.BoxedInt64"; }
};

class ULongTypeSymbol : public BasicTypeSymbol
{
public:
    ULongTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::ulongTypeSymbol; }
    bool IsUnsignedType() const override { return true; }
    std::string TypeString() const override { return "ulong type"; }
    ValueType GetValueType() const override { return ValueType::ulongType; }
    std::string GetBoxedTypeName() const override { return "System.BoxedUInt64"; }
};

class FloatTypeSymbol : public BasicTypeSymbol
{
public:
    FloatTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::floatTypeSymbol; }
    std::string TypeString() const override { return "float type"; }
    ValueType GetValueType() const override { return ValueType::floatType; }
    std::string GetBoxedTypeName() const override { return "System.BoxedFloat"; }
    bool IsSwitchConditionType() const override { return false; }
};

class DoubleTypeSymbol : public BasicTypeSymbol
{
public:
    DoubleTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::doubleTypeSymbol; }
    std::string TypeString() const override { return "double type"; }
    ValueType GetValueType() const override { return ValueType::doubleType; }
    std::string GetBoxedTypeName() const override { return "System.BoxedDouble"; }
    bool IsSwitchConditionType() const override { return false; }
};

class NullReferenceTypeSymbol : public BasicTypeSymbol
{
public:
    NullReferenceTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::nullReferenceTypeSymbol; }
    std::string TypeString() const override { return "null reference type"; }
    bool HasBoxedType() const override { return false; }
    bool IsSwitchConditionType() const override { return false; }
};

class TypeParameterSymbol : public TypeSymbol
{
public:
    TypeParameterSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::typeParameterSymbol; }
    std::string TypeString() const override { return "type parameter"; };
    utf32_string FullName() const override;
    Type* GetMachineType() const override { Assert(false, "no machine type");  return nullptr; }
    void AddTo(ClassTypeSymbol* classTypeSymbol) override;
};

class BoundTypeParameterSymbol : public Symbol
{
public:
    BoundTypeParameterSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::boundTypeParameterSymbol; }
    std::string TypeString() const override { return "bound type parameter"; };
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void EmplaceType(TypeSymbol* type, int index) override;
    TypeSymbol* GetType() const { return type; }
    void SetType(TypeSymbol* type_) { type = type_; }
private:
    TypeSymbol* type;
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
    void ReadClassNode(Assembly& assembly);
    void SetSpecifiers(Specifiers specifiers); 
    void AddSymbol(std::unique_ptr<Symbol>&& symbol) override;
    void Add(MemberVariableSymbol* memberVariableSymbol);
    void Add(MemberFunctionSymbol* memberFunctionSymbol);
    void Add(ConstructorSymbol* constructorSymbol);
    void Add(StaticConstructorSymbol* staticConstructorSymbol);
    void Add(TypeParameterSymbol* typeParameterSymbol);
    void Add(PropertySymbol* propertySymbol);
    void Add(IndexerSymbol* indexerSymbol);
    bool HasBaseClass(ClassTypeSymbol* cls) const;
    bool HasBaseClass(ClassTypeSymbol* cls, int& distance) const;
    ClassTypeSymbol* BaseClass() const { return baseClass; }
    void SetBaseClass(ClassTypeSymbol* baseClass_) { baseClass = baseClass_; }
    const std::vector<InterfaceTypeSymbol*>& ImplementedInterfaces() const { return implementedInterfaces; }
    std::vector<InterfaceTypeSymbol*>& ImplementedInterfaces() { return implementedInterfaces; }
    void AddImplementedInterface(InterfaceTypeSymbol* interfaceTypeSymbol);
    void SetObjectType(ObjectType* objectType_) { objectType.reset(objectType_); }
    ObjectType* GetObjectType() const { return objectType.get(); }
    ClassData* GetClassData() const { return classData.get(); }
    Type* GetMachineType() const override { return objectType.get(); }
    const std::vector<MemberVariableSymbol*>& MemberVariables() const { return memberVariables; }
    const std::vector<MemberVariableSymbol*>& StaticMemberVariables() const { return staticMemberVariables; }
    const std::vector<MemberFunctionSymbol*>& MemberFunctions() const { return memberFunctions; }
    const std::vector<ConstructorSymbol*>& Constructors() const { return constructors; }
    StaticConstructorSymbol* GetStaticConstructor() const { return staticConstructorSymbol; }
    bool NeedsStaticInitialization() const;
    const std::vector<PropertySymbol*>& Properties() const { return properties; }
    const std::vector<IndexerSymbol*>& Indexers() const { return indexers; }
    ConstructorSymbol* DefaultConstructorSymbol() const { return defaultConstructorSymbol; }
    bool IsClassTemplate() const { return !typeParameters.empty(); }
    const std::vector<TypeParameterSymbol*>& TypeParameters() const { return typeParameters; }
    void CloneUsingNodes(const std::vector<Node*>& usingNodes_);
    const NodeList<Node>& UsingNodes() const { return usingNodes; }
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
    void InitImts();
    void LinkImts();
private:
    ClassTypeSymbol* baseClass;
    std::vector<InterfaceTypeSymbol*> implementedInterfaces;
    std::unique_ptr<ObjectType> objectType;
    std::unique_ptr<ClassData> classData;
    ClassTypeSymbolFlags flags;
    std::vector<MemberVariableSymbol*> memberVariables;
    std::vector<MemberVariableSymbol*> staticMemberVariables;
    std::vector<MemberFunctionSymbol*> memberFunctions;
    std::vector<ConstructorSymbol*> constructors;
    ConstructorSymbol* defaultConstructorSymbol;
    StaticConstructorSymbol* staticConstructorSymbol;
    std::vector<TypeParameterSymbol*> typeParameters;
    std::vector<PropertySymbol*> properties;
    std::vector<IndexerSymbol*> indexers;
    int level;
    int priority;
    uint64_t key;
    uint64_t cid;
    uint32_t assemblyId;
    uint32_t classNodePos;
    NodeList<Node> usingNodes;
    std::unique_ptr<Node> classNode;
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

class InterfaceTypeSymbol : public TypeSymbol
{
public:
    InterfaceTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::interfaceTypeSymbol; }
    std::string TypeString() const override { return "interface"; }
    const std::vector<MemberFunctionSymbol*>& MemberFunctions() const { return memberFunctions; }
    void AddSymbol(std::unique_ptr<Symbol>&& symbol) override;
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    ObjectType* GetObjectType() const { return objectType.get(); }
    Type* GetMachineType() const override { return objectType.get(); }
    void SetSpecifiers(Specifiers specifiers);
private:
    std::unique_ptr<ObjectType> objectType; 
    std::vector<MemberFunctionSymbol*> memberFunctions;
};

bool Implements(MemberFunctionSymbol* classMemFun, MemberFunctionSymbol* interfaceMemFun);

struct ClassTemplateSpecializationKey
{
    ClassTemplateSpecializationKey() : classTypeSymbol(nullptr), typeArguments() {}
    ClassTemplateSpecializationKey(ClassTypeSymbol* classTypeSymbol_, std::vector<TypeSymbol*> typeArguments_) : classTypeSymbol(classTypeSymbol_), typeArguments(typeArguments_) {}
    ClassTypeSymbol* classTypeSymbol;
    std::vector<TypeSymbol*> typeArguments;
};

inline bool operator==(const ClassTemplateSpecializationKey& left, const ClassTemplateSpecializationKey& right)
{
    if (left.classTypeSymbol != right.classTypeSymbol) return false;
    if (left.typeArguments.size() != right.typeArguments.size()) return false;
    int n = int(left.typeArguments.size());
    for (int i = 0; i < n; ++i)
    {
        if (left.typeArguments[i] != right.typeArguments[i]) return false;
    }
    return true;
}

inline bool operator!=(const ClassTemplateSpecializationKey& left, const ClassTemplateSpecializationKey& right)
{
    return !(left == right);
}

struct ClassTemplateSpecializationKeyHash
{
    size_t operator()(const ClassTemplateSpecializationKey& key) const
    {
        if (!key.classTypeSymbol) return 0;
        size_t hashCode = std::hash<ClassTypeSymbol*>()(key.classTypeSymbol);
        int n = int(key.typeArguments.size());
        for (int i = 0; i < n; ++i)
        {
            size_t argumentHash = std::hash<TypeSymbol*>()(key.typeArguments[i]);
            hashCode = 1099511628211 * hashCode + argumentHash;
        }
        return hashCode;
    }
};

enum class ClassTemplateSpecializationSymbolFlags : uint8_t
{
    none = 0,
    reopened = 1 << 0
};

inline ClassTemplateSpecializationSymbolFlags operator|(ClassTemplateSpecializationSymbolFlags left, ClassTemplateSpecializationSymbolFlags right)
{
    return ClassTemplateSpecializationSymbolFlags(uint8_t(left) | uint8_t(right));
}

inline ClassTemplateSpecializationSymbolFlags operator&(ClassTemplateSpecializationSymbolFlags left, ClassTemplateSpecializationSymbolFlags right)
{
    return ClassTemplateSpecializationSymbolFlags(uint8_t(left) & uint8_t(right));
}

class ClassTemplateSpecializationSymbol : public ClassTypeSymbol
{
public:
    ClassTemplateSpecializationSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::classTemplateSpecializationSymbol; }
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void EmplaceType(TypeSymbol* type, int index) override;
    void SetKey(const ClassTemplateSpecializationKey& key_);
    const ClassTemplateSpecializationKey& Key() const { return key; }
    ClassTypeSymbol* PrimaryClassTemplate() const { return key.classTypeSymbol; }
    int NumTypeArguments() const { return int(key.typeArguments.size()); }
    TypeSymbol* TypeArgument(int index) const { Assert(index >= 0 && index < key.typeArguments.size(), "invalid type argument index"); return key.typeArguments[index]; }
    void SetGlobalNs(std::unique_ptr<NamespaceNode>&& globalNs_);
    NamespaceNode* GlobalNs() const { return globalNs.get(); }
    bool IsReopened() const { return GetFlag(ClassTemplateSpecializationSymbolFlags::reopened); }
    void SetReopened() { SetFlag(ClassTemplateSpecializationSymbolFlags::reopened); }
    bool IsReopenedClassTemplateSpecialization() const override { return IsReopened(); }
    void AddToBeMerged(std::unique_ptr<ClassTemplateSpecializationSymbol>&& that);
    void MergeOpenedInstances();
    void MergeConstructorSymbol(const ConstructorSymbol& constructorSymbol);
    void MergeMemberFunctionSymbol(const MemberFunctionSymbol& memberFunctionSymbol);
    void MergePropertySymbol(const PropertySymbol& propertySymbol);
    void MergeIndexerSymbol(const IndexerSymbol& indexerSymbol);
    bool HasGlobalNs() const { return globalNs != nullptr; }
    void ReadGlobalNs();
private:
    ClassTemplateSpecializationSymbolFlags flags;
    ClassTemplateSpecializationKey key;
    std::unique_ptr<NamespaceNode> globalNs;
    uint32_t assemblyId;
    uint32_t globalNsPos;
    bool GetFlag(ClassTemplateSpecializationSymbolFlags flag) const { return (flags & flag) != ClassTemplateSpecializationSymbolFlags::none; }
    void SetFlag(ClassTemplateSpecializationSymbolFlags flag) { flags = flags | flag; }
    std::vector<std::unique_ptr<ClassTemplateSpecializationSymbol>> toBeMerged;
    void WriteGlobalNs(SymbolWriter& symbolWriter);
};

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_SYMBOL_INCLUDED
