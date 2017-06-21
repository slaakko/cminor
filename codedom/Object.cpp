// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/codedom/Object.hpp>
#include <cminor/util/Unicode.hpp>

namespace cminor { namespace codedom {

using namespace cminor::util;
using namespace cminor::unicode;

CppObject::CppObject(const std::u32string& name_): name(name_), isOwned(false)
{
}

CppObject::~CppObject()
{
}

void CppObject::Own(CppObject* object)
{
    if (object)
    {
        if (!object->IsOwned())
        {
            object->SetOwned();
            ownedObjects.push_back(std::unique_ptr<CppObject>(object));
        }
    }
}

std::u32string CppObject::ToString() const
{
    return name;
}

void CppObject::Print(CodeFormatter& formatter)
{
    formatter.Write(ToUtf8(name));
}

} } // namespace cminor::codedom
