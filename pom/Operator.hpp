// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_POM_OPERATOR_INCLUDED
#define CMINOR_POM_OPERATOR_INCLUDED
#include <string>

namespace cminor { namespace pom {

enum class Operator 
{  
    deref, addrOf, plus, minus, and_, or_, not_, neg, dotStar, arrowStar, bitand_, bitxor, bitor_, mul, div, rem, shiftLeft, shiftRight, 
    less, greater, lessOrEq, greaterOrEq, eq, notEq, comma,
    assign, mulAssing, divAssign, remAssign, plusAssign, minusAssing, shiftLeftAssign, shiftRightAssign, andAssing, xorAssing, orAssign
};

Operator GetOperator(const std::string& operatorName);
std::string GetOperatorStr(Operator op);
void OperatorInit();
void OperatorDone();

} } // namespace cminor::pom

#endif // CMINOR_POM_OPERATOR_INCLUDED
