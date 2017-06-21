// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_CODEDOM_LITERAL_INCLUDED
#define CMINOR_CODEDOM_LITERAL_INCLUDED
#include <cminor/codedom/Object.hpp>

namespace cminor { namespace codedom {

class Literal: public CppObject
{
public:
    Literal(const std::u32string& name_);
    virtual int Rank() const { return 24; }
    virtual void Accept(Visitor& visitor);
};

} } // namespace cminor::codedom

#endif // CMINOR_CODEDOM_LITERAL_INCLUDED

