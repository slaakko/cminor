// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_SPECIFIER_INCLUDED
#define CMINOR_AST_SPECIFIER_INCLUDED
#include <stdint.h>
#include <string>

namespace cminor { namespace ast {

enum class Specifiers : uint16_t
{
    none = 0,
    public_ = 1 << 0,
    protected_ = 1 << 1,
    private_ = 1 << 2,
    internal_ = 1 << 3,
    static_ = 1 << 4,
    virtual_ = 1 << 5,
    override_ = 1 << 6,
    abstract_ = 1 << 7,
    inline_ = 1 << 8,
    access_ = public_ | protected_ | private_ | internal_
};

inline Specifiers operator|(Specifiers left, Specifiers right)
{
    return Specifiers(uint16_t(left) | uint16_t(right));
}

inline Specifiers operator&(Specifiers left, Specifiers right)
{
    return Specifiers(uint16_t(left) & uint16_t(right));
}

inline bool HasStaticSpecifier(Specifiers specifiers)
{
    return (specifiers & Specifiers::static_) != Specifiers::none;
}

std::string SpecifierStr(Specifiers specifiers);

} } // namespace cminor::ast

#endif // CMINOR_AST_SPECIFIER_INCLUDED
