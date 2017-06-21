// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Constant.hpp>
#include <cminor/machine/Writer.hpp>
#include <cminor/machine/Reader.hpp>
#include <list>

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

void Constant::Dump(CodeFormatter& formatter)
{
    value.Dump(formatter);
}

class ConstantPoolImpl
{
public:
    ConstantPoolImpl();
    ConstantId Install(Constant constant);
    ConstantId Install(StringPtr s);
    ConstantId GetIdFor(Constant constant);
    ConstantId GetIdFor(const std::u32string& s);
    Constant GetConstant(ConstantId id) const { Assert(id.Value() < constants.size(), "invalid constant id " + std::to_string(id.Value())); return constants[ConstantIndex(id)]; }
    Constant GetEmptyStringConstant() const { return GetConstant(emptyStringConstantId); }
    ConstantId GetEmptyStringConstantId() const { return emptyStringConstantId; }
    void Write(Writer& writer);
    void Read(Reader& reader);
    void Dump(CodeFormatter& formatter);
private:
    ConstantId emptyStringConstantId;
    std::vector<Constant> constants;
    std::unordered_map<Constant, ConstantId, ConstantHash> constantIdMap;
    std::list<std::u32string> strings;
    std::unordered_map<StringPtr, ConstantId, StringPtrHash> stringIdMap;
    ConstantId NextFreeConstantId() { return int32_t(constants.size()); }
    int32_t ConstantIndex(ConstantId id) const { return id.Value(); }
};

ConstantPoolImpl::ConstantPoolImpl() : emptyStringConstantId()
{
    std::u32string emptyString;
    strings.push_back(std::move(emptyString));
    const std::u32string& u = strings.back();
    Constant emptyStringConstant(IntegralValue(u.c_str()));
    emptyStringConstantId = NextFreeConstantId();
    constants.push_back(emptyStringConstant);
    stringIdMap[StringPtr(u.c_str())] = emptyStringConstantId;
}

ConstantId ConstantPoolImpl::GetIdFor(Constant constant)
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

ConstantId ConstantPoolImpl::GetIdFor(const std::u32string& s)
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

ConstantId ConstantPoolImpl::Install(Constant constant)
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
            std::u32string s(constant.Value().AsStringLiteral());
            strings.push_back(std::move(s));
            const std::u32string& u = strings.back();
            Constant myConstant(IntegralValue(u.c_str()));
            ConstantId id = NextFreeConstantId();
            constants.push_back(myConstant);
            constantIdMap[constant] = id;
            stringIdMap[StringPtr(u.c_str())] = id;
            return id;
        }
    }
}

ConstantId ConstantPoolImpl::Install(StringPtr s)
{
    Constant constant(s.Value());
    return Install(constant);
}

void ConstantPoolImpl::Write(Writer& writer)
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

void ConstantPoolImpl::Read(Reader& reader)
{
    uint32_t n = reader.GetEncodedUInt();
    for (uint32_t i = 0; i < n; ++i)
    {
        ConstantId id = ConstantId((emptyStringConstantId.Value() + 1) + i);
        Constant constant;
        constant.Read(reader);
        if (constant.Value().GetType() == ValueType::stringLiteral) // note: string literal value not read yet, so read it now...
        {
            std::u32string s = reader.GetUtf32String();
            strings.push_back(std::move(s));
            const std::u32string& u = strings.back();
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

void ConstantPoolImpl::Dump(CodeFormatter& formatter)
{
    formatter.WriteLine("CONSTANT POOL");
    formatter.WriteLine();
    uint32_t n = uint32_t(constants.size());
    for (uint32_t i = 0; i < n; ++i)
    {
        formatter.Write("id: " + std::to_string(i) + ", ");
        Constant constant = constants[i];
        constant.Dump(formatter);
    }
    formatter.WriteLine();
}

ConstantPool::ConstantPool() : impl(new ConstantPoolImpl())
{
}

ConstantPool::~ConstantPool()
{
    delete impl;
}

ConstantId ConstantPool::Install(Constant constant)
{
    return impl->Install(constant);
}

ConstantId ConstantPool::Install(StringPtr s)
{
    return impl->Install(s);
}

ConstantId ConstantPool::GetIdFor(Constant constant)
{
    return impl->GetIdFor(constant);
}

ConstantId ConstantPool::GetIdFor(const std::u32string& s)
{
    return impl->GetIdFor(s);
}

Constant ConstantPool::GetConstant(ConstantId id) const 
{ 
    return impl->GetConstant(id);
}

Constant ConstantPool::GetEmptyStringConstant() const
{
    return impl->GetEmptyStringConstant();
}

ConstantId ConstantPool::GetEmptyStringConstantId() const 
{ 
    return impl->GetEmptyStringConstantId();
}

void ConstantPool::Write(Writer& writer)
{
    impl->Write(writer);
}

void ConstantPool::Read(Reader& reader)
{
    impl->Read(reader);
}

void ConstantPool::Dump(CodeFormatter& formatter)
{
    impl->Dump(formatter);
}

} } // namespace cminor::machine
