// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_OPERATOR_INCLUDED
#define CMINOR_OPERATOR_INCLUDED
#include <stdint.h>

enum class Operator : uint8_t
{
    or_, and_, eq, neq, less, greater, lessOrEq, greaterOrEq, is, as, plus, minus, mul, div, rem, not_, bitOr, bitXor, bitAnd, shiftLeft, shiftRight, complement
};

#endif // CMINOR_OPERATOR_INCLUDED
