// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_OBJECT_FUN_INCLUDED
#define CMINOR_SYMBOLS_OBJECT_FUN_INCLUDED
#include <cminor/symbols/BasicTypeFun.hpp>

namespace cminor { namespace symbols {

class Assembly;
class ArrayTypeSymbol;

class ObjectDefaultInit : public BasicTypeInit
{
public:
    ObjectDefaultInit(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::objectDefaultInit; }
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
};

class ObjectCopyInit : public BasicTypeInit
{
public:
    ObjectCopyInit(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::objectCopyInit; }
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
};

class ObjectNullInit : public BasicTypeInit
{
public:
    ObjectNullInit(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::objectNullInit; }
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
};

class ObjectAssignment : public BasicTypeFun
{
public:
    ObjectAssignment(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::objectAssignment; };
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
};

class ObjectNullAssignment : public BasicTypeFun
{
public:
    ObjectNullAssignment(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::objectNullAssignment; }
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
};

class ObjectNullEqual : public BasicTypeFun
{
public:
    ObjectNullEqual(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::objectNullEqual; }
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
};

class NullObjectEqual : public BasicTypeFun
{
public:
    NullObjectEqual(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::nullObjectEqual; }
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
};

class NullToObjectConversion : public BasicTypeFun
{
public:
    NullToObjectConversion(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::nullToObjectConversion; }
    ConversionType GetConversionType() const override { return ConversionType::implicit_; }
    int32_t ConversionDistance() const override { return 1; }
    void SetSourceType(TypeSymbol* sourceType_) { sourceType = sourceType_; }
    TypeSymbol* ConversionSourceType() const override { return sourceType; }
    void SetTargetType(TypeSymbol* targetType_) { targetType = targetType_; SetType(targetType); }
    TypeSymbol* ConversionTargetType() const override { return targetType; }
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
private:
    TypeSymbol* sourceType;
    TypeSymbol* targetType;
};

void CreateDefaultConstructor(Assembly& assembly, ClassTypeSymbol* classTypeSymbol);
void CreateArraySizeConstructor(Assembly& assembly, ArrayTypeSymbol* arrayTypeSymbol);
void CreateBasicTypeObjectFun(Assembly& assembly, TypeSymbol* classOrInterfaceType);

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_OBJECT_FUN_INCLUDED
