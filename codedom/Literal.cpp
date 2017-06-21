// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/codedom/Literal.hpp>
#include <cminor/codedom/Visitor.hpp>

namespace cminor { namespace codedom {

Literal::Literal(const std::u32string& name_): CppObject(name_)
{
}

void Literal::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::codedom
