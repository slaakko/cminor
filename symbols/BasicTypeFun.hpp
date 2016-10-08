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
    virtual void ComputeName();
private:
    TypeSymbol* type;
};

class BasicTypeConstructor : public BasicTypeFun
{
public:
    BasicTypeConstructor(const Span& span_, Constant name_);
    void ComputeName() override;
};

class BasicTypeDefaultConstructor : public BasicTypeConstructor
{
public:
    BasicTypeDefaultConstructor(const Span& span_, Constant name_);
};

class BasicTypeInitConstructor : public BasicTypeConstructor
{
public:
    BasicTypeInitConstructor(const Span& span_, Constant name_);
    void GenerateCode(Machine& machine, Function& function, std::vector<GenObject*>& objects) override;
};

class BasicTypeAssignment : public BasicTypeFun
{
public:
    BasicTypeAssignment(const Span& span_, Constant name_);
    void ComputeName() override;
    void GenerateCode(Machine& machine, Function& function, std::vector<GenObject*>& objects) override;
};

void InitBasicTypeFun(Assembly& assembly);

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_BASIC_TYPE_FUN_INCLUDED
