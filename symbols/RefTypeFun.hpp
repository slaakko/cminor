// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_REF_TYPE_FUN_INCLUDED
#define CMINOR_SYMBOLS_REF_TYPE_FUN_INCLUDED
#include <cminor/symbols/BasicTypeFun.hpp>

namespace cminor { namespace symbols {

class RefTypeInit : public BasicTypeInit
{
public:
    RefTypeInit(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::refTypeInit; }
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
};

class RefTypeAssignment : public BasicTypeFun
{
public:
    RefTypeAssignment(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::refTypeAssignment; }
    void ComputeName() override;
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
};

void CreateRefTypeBasicFun(Assembly& assembly, RefTypeSymbol* refTypeSymbol);

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_REF_TYPE_FUN_INCLUDED
