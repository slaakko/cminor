// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_OBJECT_FUN_INCLUDED
#define CMINOR_SYMBOLS_OBJECT_FUN_INCLUDED
#include <cminor/symbols/BasicTypeFun.hpp>

namespace cminor { namespace symbols {

class Assembly;

class ObjectCopyInit : public BasicTypeInit
{
public:
    ObjectCopyInit(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::objectCopyInit; }
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects) override;
};

void InitObjectFun(Assembly& assembly);

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_OBJECT_FUN_INCLUDED
