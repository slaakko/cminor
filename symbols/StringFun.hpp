// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_STRING_FUN_INCLUDED
#define CMINOR_SYMBOLS_STRING_FUN_INCLUDED
#include <cminor/symbols/BasicTypeFun.hpp>

namespace cminor { namespace symbols {

BasicTypeConversion* CreateStringLiteralToStringConversion(Assembly& assembly, ClassTypeSymbol* stringType);

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_STRING_FUN_INCLUDED
