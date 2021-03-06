// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_FUNCTION_SYMBOL_INCLUDED
#define CMINOR_SYMBOLS_FUNCTION_SYMBOL_INCLUDED
#include <cminor/symbols/Symbol.hpp>
#include <cminor/machine/GenObject.hpp>

namespace cminor { namespace symbols {

using namespace cminor::machine;

class Assembly;
class ParameterSymbol;
class LocalVariableSymbol;

enum class ConversionType : uint8_t
{
    implicit_, explicit_
};

enum class FunctionSymbolFlags : uint8_t
{
    none = 0,
    inline_ = 1 << 0,
    external_ = 1 << 1,
    conversionFun = 1 << 2,
    exported = 1 << 3,
    canThrow = 1 << 4,
    intrinsic = 1 << 5
};

inline FunctionSymbolFlags operator&(FunctionSymbolFlags left, FunctionSymbolFlags right)
{
    return FunctionSymbolFlags(uint8_t(left) & uint8_t(right));
}

inline FunctionSymbolFlags operator|(FunctionSymbolFlags left, FunctionSymbolFlags right)
{
    return FunctionSymbolFlags(uint8_t(left) | uint8_t(right));
}

std::string FunctionSymbolFlagStr(FunctionSymbolFlags flags);

class FunctionSymbol : public ContainerSymbol
{
public:
    FunctionSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::functionSymbol; }
    std::string TypeString() const override { return "function"; }
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void SetSpecifiers(Specifiers specifiers);
    void SetInline() { SetFlag(FunctionSymbolFlags::inline_); }
    bool IsInline() const { return GetFlag(FunctionSymbolFlags::inline_); }
    void SetExternal() { SetFlag(FunctionSymbolFlags::external_); }
    bool IsExternal() const { return GetFlag(FunctionSymbolFlags::external_); }
    void SetIntrinsic() { SetFlag(FunctionSymbolFlags::intrinsic); }
    bool IsIntrinsic() const { return GetFlag(FunctionSymbolFlags::intrinsic); }
    bool IsExported() const { return GetFlag(FunctionSymbolFlags::exported); }
    void SetExported() { SetFlag(FunctionSymbolFlags::exported); }
    bool CanThrow() const { return GetFlag(FunctionSymbolFlags::canThrow); }
    void SetCanThrow() { SetFlag(FunctionSymbolFlags::canThrow); }
    virtual bool IsAbstract() const { return false; }
    StringPtr GroupName() const { return StringPtr(groupName.Value().AsStringLiteral()); }
    void SetGroupNameConstant(Constant groupName_) { groupName = groupName_; }
    virtual void ComputeName();
    std::u32string FullName() const override;
    virtual std::u32string FullNameWithSpecifiers() const;
    virtual std::u32string FullParsingName() const;
    int Arity() const { return int(parameters.size()); }
    void AddSymbol(std::unique_ptr<Symbol>&& symbol) override;
    const std::vector<ParameterSymbol*>& Parameters() const { return parameters; }
    TypeSymbol* ReturnType() const { return returnType; }
    void SetReturnType(TypeSymbol* returnType_) { returnType = returnType_;  }
    const std::vector<LocalVariableSymbol*>& LocalVariables() const { return localVariables; }
    void AddLocalVariable(LocalVariableSymbol* localVariable);
    bool IsConversionFun() const { return GetFlag(FunctionSymbolFlags::conversionFun);  }
    void SetConversionFun() { SetFlag(FunctionSymbolFlags::conversionFun); }
    virtual ParameterSymbol* GetThisParam() const { return nullptr; }
    virtual ConversionType GetConversionType() const { return ConversionType::implicit_; }
    virtual int ConversionDistance() const { return 0; }
    virtual TypeSymbol* ConversionSourceType() const { return nullptr; }
    virtual TypeSymbol* ConversionTargetType() const { return nullptr; }
    virtual void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start);
    bool GetFlag(FunctionSymbolFlags flag) const { return (flags & flag) != FunctionSymbolFlags::none; }
    void SetFlag(FunctionSymbolFlags flag) { flags = flags | flag; }
    Function* MachineFunction() const { return machineFunction; }
    virtual void CreateMachineFunction();
    void EmplaceType(TypeSymbol* type, int index) override;
    void SetVmf(StringPtr vmf);
    Constant Vmf() const { return vmf; }
    FunctionSymbolFlags GetFlags() const { return flags; }
    virtual bool IsDerived() const { return false; }
    virtual bool IsMemFunToClassDelegateConversion() const { return false; }
    void DumpHeader(CodeFormatter& formatter) override;
    int DeclarationBlockId() const { return declarationBlockId; }
    void SetDeclarationBlockId(int declarationBlockId_) { declarationBlockId = declarationBlockId_; }
    virtual bool IsPropertyGetterOrSetter() const { return false; }
    virtual bool IsIndexerGetterOrSetter() const { return false; }
    uint32_t GetNextContainerScopeId() { return nextContainerScopeId++; }
    void MapContainerScope(uint32_t containerScopeId, ContainerScope* containerScope);
    void MapPCToContainerScopeId(uint32_t pc, uint32_t containerScopeId);
    ContainerScope* GetMappedContainerScopeForPC(uint32_t pc) const;
private:
    Constant groupName;
    Constant vmf;
    std::vector<ParameterSymbol*> parameters;
    std::vector<LocalVariableSymbol*> localVariables;
    TypeSymbol* returnType;
    FunctionSymbolFlags flags;
    Function* machineFunction;
    int declarationBlockId;
    uint32_t nextContainerScopeId;
    std::unordered_map<uint32_t, ContainerScope*> containerScopeMap;
    std::unordered_map<uint32_t, uint32_t> pcContainerScopeIdMap;
};

class StaticConstructorSymbol : public FunctionSymbol
{
public:
    StaticConstructorSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::staticConstructorSymbol; }
    std::string TypeString() const override { return "static constructor"; }
    void SetSpecifiers(Specifiers specifiers);
    std::u32string FullParsingName() const override;
    std::u32string FullNameWithSpecifiers() const override;
    bool IsDerived() const override { return true; }
    void AddTo(ClassTypeSymbol* classTypeSymbol) override;
    void SetFullNameConstant(Constant fullNameConstant_) { fullNameConstant = fullNameConstant_; }
    Constant GetFullNameConstant() const { return fullNameConstant; }
private:
    Constant fullNameConstant;
};

enum class ConstructorSymbolFlags : uint8_t
{
    none = 0,
    baseConstructorCallGenerated = 1 << 0
};

inline ConstructorSymbolFlags operator&(ConstructorSymbolFlags left, ConstructorSymbolFlags right)
{
    return ConstructorSymbolFlags(uint8_t(left) & uint8_t(right));
}

inline ConstructorSymbolFlags operator|(ConstructorSymbolFlags left, ConstructorSymbolFlags right)
{
    return ConstructorSymbolFlags(uint8_t(left) | uint8_t(right));
}

class ConstructorSymbol : public FunctionSymbol
{
public:
    ConstructorSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::constructorSymbol; }
    std::string TypeString() const override { return "constructor"; }
    void SetSpecifiers(Specifiers specifiers);
    std::u32string FullParsingName() const override;
    std::u32string FullNameWithSpecifiers() const override;
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
    void CreateMachineFunction() override;
    bool IsDefaultConstructorSymbol() const;
    bool IsIntConstructorSymbol() const;
    void SetBaseConstructorCallGenerated() { SetFlag(ConstructorSymbolFlags::baseConstructorCallGenerated); }
    bool BaseConstructorCallGenerated() const { return GetFlag(ConstructorSymbolFlags::baseConstructorCallGenerated); }
    ParameterSymbol* GetThisParam() const override { return Parameters()[0]; }
    void AddTo(ClassTypeSymbol* classTypeSymbol) override;
    void MergeTo(ClassTemplateSpecializationSymbol* classTemplateSpecializationSymbol, Assembly* assembly) override;
    void Merge(ConstructorSymbol& that, Assembly* assembly);
    bool IsDerived() const override { return true; }
private:
    ConstructorSymbolFlags flags;
    bool GetFlag(ConstructorSymbolFlags flag) const { return (flags & flag) != ConstructorSymbolFlags::none; }
    void SetFlag(ConstructorSymbolFlags flag) { flags = flags | flag; }
};

class ArraySizeConstructorSymbol : public ConstructorSymbol
{
public:
    ArraySizeConstructorSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::arraySizeConstructorSymbol; }
    std::string TypeString() const override { return "array size constructor"; }
    void CreateMachineFunction() override;
};

enum class MemberFunctionSymbolFlags : uint8_t
{
    none = 0,
    virtual_ = 1 << 0,
    abstract_ = 1 << 1,
    override_ = 1 << 2,
    new_ = 1 << 3
};

inline MemberFunctionSymbolFlags operator&(MemberFunctionSymbolFlags left, MemberFunctionSymbolFlags right)
{
    return MemberFunctionSymbolFlags(uint8_t(left) & uint8_t(right));
}

inline MemberFunctionSymbolFlags operator|(MemberFunctionSymbolFlags left, MemberFunctionSymbolFlags right)
{
    return MemberFunctionSymbolFlags(uint8_t(left) | uint8_t(right));
}

std::string MemberFunctionSymbolFlagStr(MemberFunctionSymbolFlags flags);

class MemberFunctionSymbol : public FunctionSymbol
{
public:
    MemberFunctionSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::memberFunctionSymbol; }
    std::string TypeString() const override { return "member function"; }
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void SetSpecifiers(Specifiers specifiers);
    std::u32string FullParsingName() const override;
    std::u32string FullNameWithSpecifiers() const override;
    void CreateMachineFunction() override;
    bool IsVirtual() const { return GetFlag(MemberFunctionSymbolFlags::virtual_); }
    void SetVirtual() { SetFlag(MemberFunctionSymbolFlags::virtual_); }
    bool IsAbstract() const override { return GetFlag(MemberFunctionSymbolFlags::abstract_); }
    void SetAbstract() { SetFlag(MemberFunctionSymbolFlags::abstract_); }
    bool IsOverride() const { return GetFlag(MemberFunctionSymbolFlags::override_); }
    void SetOverride() { SetFlag(MemberFunctionSymbolFlags::override_); }
    bool IsVirtualAbstractOrOverride() const { return GetFlag(MemberFunctionSymbolFlags::virtual_ | MemberFunctionSymbolFlags::abstract_ | MemberFunctionSymbolFlags::override_); }
    bool IsNew() const { return GetFlag(MemberFunctionSymbolFlags::new_); }
    void SetNew() { SetFlag(MemberFunctionSymbolFlags::new_); }
    uint32_t VmtIndex() const { return vmtIndex; }
    void SetVmtIndex(uint32_t vmtIndex_) { vmtIndex = vmtIndex_; }
    uint32_t ImtIndex() const { return imtIndex; }
    void SetImtIndex(uint32_t imtIndex_) { imtIndex = imtIndex_; }
    void GenerateVirtualCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects);
    void GenerateInterfaceCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects);
    ParameterSymbol* GetThisParam() const override { if (IsStatic()) return nullptr; else return Parameters()[0]; }
    void AddTo(ClassTypeSymbol* classTypeSymbol) override;
    void MergeTo(ClassTemplateSpecializationSymbol* classTemplateSpecializationSymbol, Assembly* assembly) override;
    void Merge(MemberFunctionSymbol& that, Assembly* assembly);
    bool ImplementsInterfaceMemFun(InterfaceTypeSymbol* intf);
private:
    uint32_t vmtIndex;
    uint32_t imtIndex;
    MemberFunctionSymbolFlags flags;
    bool GetFlag(MemberFunctionSymbolFlags flag) const { return (flags & flag) != MemberFunctionSymbolFlags::none; }
    void SetFlag(MemberFunctionSymbolFlags flag) { flags = flags | flag; }
};

class ArrayGetEnumeratorMemberFunctionSymbol : public MemberFunctionSymbol
{
public:
    ArrayGetEnumeratorMemberFunctionSymbol(const Span& span_, Constant name_);
};

class ClassTypeConversion : public FunctionSymbol
{
public:
    ClassTypeConversion(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::classTypeConversion; };
    void SetConversionType(ConversionType conversionType_) { conversionType = conversionType_; }
    ConversionType GetConversionType() const override { return conversionType; }
    void SetConversionDistance(int32_t conversionDistance_) { conversionDistance = conversionDistance_; }
    int32_t ConversionDistance() const override { return conversionDistance; }
    void SetSourceType(TypeSymbol* sourceType_) { sourceType = sourceType_; }
    TypeSymbol* ConversionSourceType() const override { return sourceType; }
    void SetTargetType(TypeSymbol* targetType_) { targetType = targetType_; SetReturnType(targetType);  }
    TypeSymbol* ConversionTargetType() const override { return targetType; }
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
private:
    ConversionType conversionType;
    int32_t conversionDistance;
    TypeSymbol* sourceType;
    TypeSymbol* targetType;
};

class ClassToInterfaceConversion : public FunctionSymbol
{
public:
    ClassToInterfaceConversion(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::classToInterfaceConversion; }
    ConversionType GetConversionType() const override { return ConversionType::implicit_; }
    int32_t ConversionDistance() const override { return 1; }
    void SetSourceType(TypeSymbol* sourceType_) { sourceType = sourceType_; }
    TypeSymbol* ConversionSourceType() const override { return sourceType; }
    void SetTargetType(TypeSymbol* targetType_) { targetType = targetType_; SetReturnType(targetType); }
    TypeSymbol* ConversionTargetType() const override { return targetType; }
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
    void SetITabIndex(int32_t itabIndex_) { itabIndex = itabIndex_; }
private:
    TypeSymbol* sourceType;
    TypeSymbol* targetType;
    int32_t itabIndex;
};

class FunctionGroupSymbol : public Symbol
{
public:
    FunctionGroupSymbol(const Span& span_, Constant name_, ContainerScope* containerScope_);
    SymbolType GetSymbolType() const override { return SymbolType::functionGroupSymbol; }
    std::string TypeString() const override { return "function group"; }
    bool IsExportSymbol() const override { return false; }
    void AddFunction(FunctionSymbol* function);
    void CollectViableFunctions(int arity, std::unordered_set<FunctionSymbol*>& viableFunctions);
    FunctionSymbol* GetOverload() const;
private:
    std::unordered_map<int, std::vector<FunctionSymbol*>> arityFunctionListMap;
    ContainerScope* containerScope;
};

class FunctionGroupTypeSymbol : public TypeSymbol
{
public:
    FunctionGroupTypeSymbol(FunctionGroupSymbol* functionGroup_);
    SymbolType GetSymbolType() const override { return SymbolType::functionGroupTypeSymbol; }
    std::string TypeString() const override { return "function group type"; }
    bool IsExportSymbol() const override { return false; }
    FunctionGroupSymbol* FunctionGroup() const { return functionGroup; }
    Type* GetMachineType() const override { Assert(false, "function group type has no machine type"); return nullptr; }
    bool IsFunctionGroupTypeSymbol() const override { return true; }
private:
    FunctionGroupSymbol* functionGroup;
};

class MemberExpressionTypeSymbol : public TypeSymbol
{
public:
    MemberExpressionTypeSymbol(Constant name_, void* boundMemberExpression_);
    SymbolType GetSymbolType() const override { return SymbolType::memberExpressionTypeSymbol; }
    std::string TypeString() const override { return "member expression type"; }
    bool IsExportSymbol() const override { return false; }
    void* BoundMemberExpression() const { return boundMemberExpression; }
    Type* GetMachineType() const override { Assert(false, "member expression type has no machine type"); return nullptr; }
    bool IsMemberExpressionTypeSymbol() const override { return true; }
private:
    void* boundMemberExpression;
};

class FunctionToDelegateConversion : public FunctionSymbol
{
public:
    FunctionToDelegateConversion(const Span& span_, Constant name_);
    ConversionType GetConversionType() const override { return ConversionType::implicit_; }
    int32_t ConversionDistance() const override { return 1; }
    void SetSourceType(TypeSymbol* sourceType_) { sourceType = sourceType_; }
    TypeSymbol* ConversionSourceType() const override { return sourceType; }
    void SetTargetType(TypeSymbol* targetType_) { targetType = targetType_; SetReturnType(targetType); }
    TypeSymbol* ConversionTargetType() const override { return targetType; }
    void SetFunctionName(Constant functionName_);
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
private:
    TypeSymbol* sourceType;
    TypeSymbol* targetType;
    Constant functionName;
};

class MemFunToClassDelegateConversion : public FunctionSymbol
{
public:
    MemFunToClassDelegateConversion(const Span& span_, Constant name_);
    ConversionType GetConversionType() const override { return ConversionType::implicit_; }
    int32_t ConversionDistance() const override { return 1; }
    void SetSourceType(TypeSymbol* sourceType_) { sourceType = sourceType_; }
    TypeSymbol* ConversionSourceType() const override { return sourceType; }
    void SetTargetType(TypeSymbol* targetType_) { targetType = targetType_; SetReturnType(targetType); }
    TypeSymbol* ConversionTargetType() const override { return targetType; }
    void SetFunctionName(Constant functionName_);
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
    bool IsMemFunToClassDelegateConversion() const override { return true; }
private:
    TypeSymbol* sourceType;
    TypeSymbol* targetType;
    Constant functionName;
};

class RequestGcFunctionSymbol : public FunctionSymbol
{
public:
    RequestGcFunctionSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::requestGcFunctionSymbol; }
    std::string TypeString() const override { return "request GC function"; }
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
};

struct ConversionTypeHash
{
    size_t operator()(const std::pair<TypeSymbol*, TypeSymbol*>& typeSymbolPair) const
    {
        size_t source = std::hash<TypeSymbol*>()(typeSymbolPair.first);
        size_t target = std::hash<TypeSymbol*>()(typeSymbolPair.second);
        return source ^ target;
    }
};

class ConversionTable
{
public:
    ConversionTable(Assembly& assembly_);
    void AddConversion(FunctionSymbol* conversionFun);
    FunctionSymbol* GetConversion(TypeSymbol* sourceType, TypeSymbol* targetType);
    const std::unordered_map<std::pair<TypeSymbol*, TypeSymbol*>, FunctionSymbol*, ConversionTypeHash>& ConversionMap() const { return conversionMap; }
    void SetConversionMap(const std::unordered_map<std::pair<TypeSymbol*, TypeSymbol*>, FunctionSymbol*, ConversionTypeHash>& conversionMap_);
    void ImportConversionMap(const std::unordered_map<std::pair<TypeSymbol*, TypeSymbol*>, FunctionSymbol*, ConversionTypeHash>& conversionMap_);
private:
    Assembly& assembly;
    std::unordered_map<std::pair<TypeSymbol*, TypeSymbol*>, FunctionSymbol*, ConversionTypeHash> conversionMap;
    std::vector<std::unique_ptr<FunctionSymbol>> nullConversions;
    std::vector<std::unique_ptr<ClassTypeConversion>> classTypeConversions;
    std::vector<std::unique_ptr<ClassToInterfaceConversion>> interfaceTypeConversions;
    std::vector<std::unique_ptr<FunctionSymbol>> enumTypeConversions;
    std::vector<std::unique_ptr<FunctionSymbol>> stringConversions;
    std::vector<std::unique_ptr<FunctionSymbol>> delegateConversions;
};

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_FUNCTION_SYMBOL_INCLUDED
