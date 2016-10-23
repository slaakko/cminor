// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/Specifier.hpp>

namespace cminor { namespace ast {

std::string SpecifierStr(Specifiers specifiers)
{
    std::string s;
    if ((specifiers & Specifiers::public_) != Specifiers::none)
    {
        s.append("public");
    }
    if ((specifiers & Specifiers::protected_) != Specifiers::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("protected");
    }
    if ((specifiers & Specifiers::private_) != Specifiers::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("private");
    }
    if ((specifiers & Specifiers::internal_) != Specifiers::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("internal");
    }
    if ((specifiers & Specifiers::static_) != Specifiers::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("static");
    }
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("virtual");
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("override");
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("abstract");
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("inline");
    }
    return s;
}

} } // namespace cminor::ast
