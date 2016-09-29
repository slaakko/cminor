// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Constant.hpp>
#include <cminor/machine/Writer.hpp>
#include <cminor/machine/Reader.hpp>

namespace cminor { namespace machine {

Constant::Constant(IntegralValue value_) : value(value_)
{
}

void Constant::Write(Writer& writer)
{
    value.Write(writer);
}

void Constant::Read(Reader& reader)
{
    value.Read(reader);
    // note: string literal value not read yet
}

ConstantPool::ConstantPool() : emptyStringConstantId()
{
    utf32_string emptyString;
    strings.push_back(std::move(emptyString));
    const utf32_string& u = strings.back();
    Constant emptyStringConstant(IntegralValue(u.c_str()));
    emptyStringConstantId = NextFreeConstantId();
    constants.push_back(emptyStringConstant);
    stringIdMap[u] = emptyStringConstantId;
}

ConstantId ConstantPool::Install(Constant constant)
{
    if (constant.Value().GetType() != ValueType::stringLiteral)
    {
        auto it = constantIdMap.find(constant);
        if (it != constantIdMap.cend())
        {
            return it->second;
        }
        else
        {
            ConstantId id = NextFreeConstantId();
            constants.push_back(constant);
            constantIdMap[constant] = id;
            return id;
        }
    }
    else
    {
        utf32_string s = constant.Value().AsStringLiteral();
        auto it = stringIdMap.find(s);
        if (it != stringIdMap.cend())
        {
            return it->second;
        }
        else
        {
            strings.push_back(std::move(s));
            const utf32_string& u = strings.back();
            Constant myConstant(IntegralValue(u.c_str()));
            ConstantId id = NextFreeConstantId();
            constants.push_back(myConstant);
            stringIdMap[u] = id;
            return id;
        }
    }
}

void ConstantPool::Write(Writer& writer)
{
    writer.Put(static_cast<uint64_t>(constants.size()));
    for (Constant& constant : constants)
    {
        constant.Write(writer);
    }
}

void ConstantPool::Read(Reader& reader)
{
    uint64_t n = reader.GetULong();
    for (uint64_t i = 0; i < n; ++i)
    {
        Constant constant;
        constant.Read(reader);
        if (constant.Value().GetType() == ValueType::stringLiteral) // note: string literal value not read yet, so read it now...
        {
            utf32_string s = reader.GetUtf32String();
            strings.push_back(std::move(s));
            constant.SetValue(IntegralValue(strings.back().c_str()));
        }
        constants.push_back(constant);
    }
}

} } // namespace cminor::machine
