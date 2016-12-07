// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/Access.hpp>

namespace cminor { namespace binder {

void CheckAccess(FunctionSymbol* fromFunction, Symbol* toSymbol)
{
    FunctionSymbol* toContainingFunction = toSymbol->ContainingFunction();
    if (toContainingFunction)
    {
        if (fromFunction == toContainingFunction)
        {
            return;
        }
    }
    ClassTypeSymbol* toContainingClass = toSymbol->ContainingClass();
    if (toContainingClass)
    {
        CheckAccess(fromFunction, toContainingClass);
    }
    switch (toSymbol->DeclaredAccess())
    {
        case SymbolAccess::public_: return;
        case SymbolAccess::protected_:
        {
            ClassTypeSymbol* fromContainingClass = fromFunction->ContainingClass();
            if (fromContainingClass)
            {
                if (toContainingClass->IsSameParentOrAncestorOf(fromContainingClass))
                {
                    return;
                }
                if (fromContainingClass->HasBaseClass(toContainingClass))
                {
                    return;
                }
            }
            break;
        }
        case SymbolAccess::internal_:
        {
            if (fromFunction->GetAssembly() == toSymbol->GetAssembly())
            {
                return;
            }
            break;
        }
        case SymbolAccess::private_:
        {
            if (toContainingClass)
            {
                ClassTypeSymbol* fromContainingClass = fromFunction->ContainingClass();
                if (fromContainingClass)
                {
                    if (toContainingClass->IsSameParentOrAncestorOf(fromContainingClass))
                    {
                        return;
                    }
                }
            }
            break;
        }
    }
    throw Exception(toSymbol->TypeString() + " '" + ToUtf8(toSymbol->FullName()) + "' is inaccessible due to its protection level", fromFunction->GetSpan(), toSymbol->GetSpan());
}

} } // namespace cminor::binder
