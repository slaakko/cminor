// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/StringFun.hpp>
#include <cminor/symbols/ObjectFun.hpp>
#include <cminor/symbols/FunctionSymbol.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/VariableSymbol.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Function.hpp>

namespace cminor { namespace symbols {

BasicTypeConversion* CreateStringLiteralToStringConversion(Assembly& assembly, ClassTypeSymbol* stringType)
{
    ConstantPool& constantPool = assembly.GetConstantPool();
    BasicTypeConversion* strLit2String = new BasicTypeConversion(Span(), constantPool.GetEmptyStringConstant());
    strLit2String->SetAssembly(&assembly);
    Constant groupName = constantPool.GetConstant(constantPool.Install(StringPtr(U"@conversion")));
    strLit2String->SetGroupNameConstant(groupName);
    strLit2String->SetConversionType(ConversionType::implicit_);
    strLit2String->SetConversionDistance(1);
    strLit2String->SetSourceType(stringType);
    strLit2String->SetTargetType(stringType);
    strLit2String->SetConversionInstructionName("slit2s");
    return strLit2String;
}

} } // namespace cminor::symbols