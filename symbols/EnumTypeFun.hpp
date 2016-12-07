// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_ENUM_TYPE_FUN_INCLUDED
#define CMINOR_SYMBOLS_ENUM_TYPE_FUN_INCLUDED
#include <cminor/symbols/EnumSymbol.hpp>
#include <cminor/symbols/BasicTypeFun.hpp>

namespace cminor { namespace symbols {

class EnumTypeDefaultInit : public BasicTypeInit
{
public:
    EnumTypeDefaultInit(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::enumTypeDefaultInit; }
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
};

class EnumTypeConversionFun : public BasicTypeFun
{
public:
    EnumTypeConversionFun(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::enumTypeConversion; };
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
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
    void EmplaceType(TypeSymbol* type, int index) override;
private:
    ConversionType conversionType;
    int32_t conversionDistance;
    TypeSymbol* sourceType;
    TypeSymbol* targetType;
};

void CreateEnumFun(Assembly& assembly, EnumTypeSymbol* enumType);

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_ENUM_TYPE_FUN_INCLUDED
