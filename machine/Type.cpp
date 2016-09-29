// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Type.hpp>

namespace cminor { namespace machine {

Type::Type(const std::string& fullName_) : fullName(fullName_)
{
}

Type* TypeRepository::CreateType(const std::string& fullName)
{
    auto it = typeMap.find(fullName);
    if (it != typeMap.cend())
    {
        throw std::runtime_error("type '" + fullName + "' already exists");
    }
    Type* type(new Type(fullName));
    typeMap[fullName] = std::unique_ptr<Type>(type);
    return type;
}

Type* TypeRepository::GetType(const std::string& fullName) const
{
    auto it = typeMap.find(fullName);
    if (it != typeMap.cend())
    {
        return it->second.get();
    }
    return nullptr;
}

} } // namespace cminor::machine
