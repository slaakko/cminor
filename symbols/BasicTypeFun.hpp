// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_BASIC_TYPE_FUN_INCLUDED
#define CMINOR_SYMBOLS_BASIC_TYPE_FUN_INCLUDED
#include <cminor/symbols/FunctionSymbol.hpp>

namespace cminor { namespace symbols {

class BasicTypeFun : public FunctionSymbol
{
public:
    BasicTypeFun(const Span& span_, Constant name_);
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    TypeSymbol* GetType() const { return type; }
    void SetType(TypeSymbol* type_) { type = type_; }
    void EmplaceType(TypeSymbol* type, int index) override;
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
private:
    TypeSymbol* type;
};

class BasicTypeInit : public BasicTypeFun
{
public:
    BasicTypeInit(const Span& span_, Constant name_);
    void ComputeName() override;
};

class BasicTypeDefaultInit : public BasicTypeInit
{
public:
    BasicTypeDefaultInit(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::basicTypeDefaultInit; }
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
};

class BasicTypeCopyInit : public BasicTypeInit
{
public:
    BasicTypeCopyInit(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::basicTypeCopyInit; }
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
};

class BasicTypeAssignment : public BasicTypeFun
{
public:
    BasicTypeAssignment(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::basicTypeAssignment; }
    void ComputeName() override;
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
};

class BasicTypeReturn : public BasicTypeFun
{
public:
    BasicTypeReturn(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::basicTypeReturn; };
    void ComputeName() override;
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
};

class BasicTypeConversion : public BasicTypeFun
{
public:
    BasicTypeConversion(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::basicTypeConversion; };
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void SetConversionType(ConversionType conversionType_) { conversionType = conversionType_; }
    ConversionType GetConversionType() const override { return conversionType; }
    void SetConversionDistance(int32_t conversionDistance_) { conversionDistance = conversionDistance_; }
    int32_t ConversionDistance() const override { return conversionDistance; }
    void SetSourceType(TypeSymbol* sourceType_) { sourceType = sourceType_; }
    TypeSymbol* ConversionSourceType() const override { return sourceType; }
    void SetTargetType(TypeSymbol* targetType_) { targetType = targetType_; SetType(targetType); }
    TypeSymbol* ConversionTargetType() const override { return targetType; }
    void SetConversionInstructionName(const std::string& conversionInstructionName_) { conversionInstructionName = conversionInstructionName_; }
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
    void EmplaceType(TypeSymbol* type, int index) override;
private:
    ConversionType conversionType;
    int32_t conversionDistance;
    TypeSymbol* sourceType;
    TypeSymbol* targetType;
    std::string conversionInstructionName;
};

class BasicTypeUnaryOpFun : public BasicTypeFun
{
public:
    BasicTypeUnaryOpFun(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::basicTypeUnaryOp; };
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
    void SetInstGroupName(const std::string& instGroupName_) { instGroupName = instGroupName_; }
    void SetTypeName(const std::string& typeName_) { typeName = typeName_; }
private:
    std::string instGroupName;
    std::string typeName;
};

class BasicTypeBinaryOpFun : public BasicTypeFun
{
public:
    BasicTypeBinaryOpFun(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::basicTypBinaryOp; };
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
    void SetInstGroupName(const std::string& instGroupName_) { instGroupName = instGroupName_; }
    void SetTypeName(const std::string& typeName_) { typeName = typeName_; }
private:
    std::string instGroupName;
    std::string typeName;
};

void InitBasicTypeFun(Assembly& assembly);

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_BASIC_TYPE_FUN_INCLUDED
