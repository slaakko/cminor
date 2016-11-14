// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Constant.hpp>
#include <cminor/machine/Writer.hpp>
#include <cminor/machine/Reader.hpp>

namespace cminor { namespace machine {

void ConstantId::Write(Writer& writer)
{
    writer.PutEncodedUInt(value);
}

void ConstantId::Read(Reader& reader)
{
    value = reader.GetEncodedUInt();
}

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
    // string literals not read yet
}

ConstantPool::ConstantPool() : emptyStringConstantId()
{
    utf32_string emptyString;
    strings.push_back(std::move(emptyString));
    const utf32_string& u = strings.back();
    Constant emptyStringConstant(IntegralValue(u.c_str()));
    emptyStringConstantId = NextFreeConstantId();
    constants.push_back(emptyStringConstant);
    stringIdMap[StringPtr(u.c_str())] = emptyStringConstantId;
}

ConstantId ConstantPool::GetIdFor(Constant constant)
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
            return noConstantId;
        }
    }
    else
    {
        auto it = stringIdMap.find(StringPtr(constant.Value().AsStringLiteral()));
        if (it != stringIdMap.cend())
        {
            return it->second;
        }
        else
        {
            return noConstantId;
        }
    }
}

ConstantId ConstantPool::GetIdFor(const utf32_string& s)
{
    auto it = stringIdMap.find(StringPtr(s.c_str()));
    if (it != stringIdMap.cend())
    {
        return it->second;
    }
    else
    {
        return noConstantId;
    }
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
        auto it = stringIdMap.find(StringPtr(constant.Value().AsStringLiteral()));
        if (it != stringIdMap.cend())
        {
            return it->second;
        }
        else
        {
            utf32_string s(constant.Value().AsStringLiteral());
            strings.push_back(std::move(s));
            const utf32_string& u = strings.back();
            Constant myConstant(IntegralValue(u.c_str()));
            ConstantId id = NextFreeConstantId();
            constants.push_back(myConstant);
            constantIdMap[constant] = id;
            stringIdMap[StringPtr(u.c_str())] = id;
            return id;
        }
    }
}

ConstantId ConstantPool::Install(StringPtr s)
{
    Constant constant(s.Value());
    return Install(constant);
}

void ConstantPool::Write(Writer& writer)
{
    uint32_t n = static_cast<uint32_t>(constants.size());
    Assert(n > 0, "positive value expected");
    writer.PutEncodedUInt(n - 1);
    for (ConstantId id = ConstantId(emptyStringConstantId.Value() + 1); id.Value() != n; id = ConstantId(id.Value() + 1))
    {
        Constant constant = GetConstant(id);
        constant.Write(writer);
    }
}

void ConstantPool::Read(Reader& reader)
{
    uint32_t n = reader.GetEncodedUInt();
    for (uint32_t i = 0; i < n; ++i)
    {
        ConstantId id = ConstantId((emptyStringConstantId.Value() + 1) + i);
        Constant constant;
        constant.Read(reader);
        if (constant.Value().GetType() == ValueType::stringLiteral) // note: string literal value not read yet, so read it now...
        {
            utf32_string s = reader.GetUtf32String();
            strings.push_back(std::move(s));
            const utf32_string& u = strings.back();
            stringIdMap[StringPtr(u.c_str())] = id;
            constant.SetValue(IntegralValue(u.c_str()));
            constantIdMap[constant] = id;
        }
        else
        {
            constantIdMap[constant] = id;
        }
        constants.push_back(constant);
    }
}

} } // namespace cminor::machine
