// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_OBJECT_FUN_INCLUDED
#define CMINOR_SYMBOLS_OBJECT_FUN_INCLUDED
#include <cminor/symbols/BasicTypeFun.hpp>

namespace cminor { namespace symbols {

class Assembly;

class ObjectDefaultInit : public BasicTypeInit
{
public:
    ObjectDefaultInit(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::objectDefaultInit; }
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects) override;
};

class ObjectCopyInit : public BasicTypeInit
{
public:
    ObjectCopyInit(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::objectCopyInit; }
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects) override;
};

class ObjectNullInit : public BasicTypeInit
{
public:
    ObjectNullInit(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::objectNullInit; }
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects) override;
};

void CreateDefaultConstructor(Assembly& assembly, ClassTypeSymbol* classTypeSymbol);
void CreateBasicTypeObjectFun(Assembly& assembly, ClassTypeSymbol* classType);
void InitObjectFun(Assembly& assembly);

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_OBJECT_FUN_INCLUDED
