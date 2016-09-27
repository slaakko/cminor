// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Object.hpp>
#include <string>

namespace cminor { namespace machine {

ObjectPtr ObjectPool::GetObjectPtr(ObjectHandle handle) const
{
    auto it = objects.find(handle);
    if (it != objects.cend())
    {
        const Object& object = it->second;
        return object.Ptr();
    }
    throw std::runtime_error("object with handle " + std::to_string(handle.Value()) + " not found");
}

} } // namespace cminor::machine