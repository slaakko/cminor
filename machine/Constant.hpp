// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_CONSTANT_INCLUDED
#define CMINOR_MACHINE_CONSTANT_INCLUDED
#include <cminor/machine/Object.hpp>
#include <cminor/machine/String.hpp>

namespace cminor { namespace machine {

class ConstantId
{
public:
    constexpr ConstantId() : value(-1) {}
    constexpr ConstantId(uint32_t value_) : value(value_) {}
    constexpr uint32_t Value() const { return value; }
    void Write(Writer& writer);
    void Read(Reader& reader);
private:
    uint32_t value;
};

constexpr ConstantId noConstantId = ConstantId();

inline bool operator==(ConstantId left, ConstantId right)
{
    return left.Value() == right.Value();
}

inline bool operator!=(ConstantId left, ConstantId right)
{
    return !(left == right);
}

class Constant
{
public:
    Constant() : value() {}
    Constant(IntegralValue value_);
    IntegralValue Value() const { return value; }
    void SetValue(IntegralValue value_) { value = value_; }
    void Write(Writer& writer);
    void Read(Reader& reader);
private:
    IntegralValue value;
};

inline bool operator==(Constant left, Constant right)
{
    return left.Value() == right.Value();
}

struct ConstantHash
{
    size_t operator()(Constant constant) const
    {
        return IntegralValueHash()(constant.Value());
    }
};

typedef std::basic_string<char32_t> utf32_string;

class ConstantPool
{
public:
    ConstantPool();
    ConstantId Install(Constant constant);
    ConstantId Install(StringPtr s);
    ConstantId GetIdFor(Constant constant);
    ConstantId GetIdFor(const utf32_string& s);
    uint64_t GetStringLength(ConstantId stringConstantId) const 
    { 
        Assert(stringConstantId.Value() < stringLengths.size(), "invalid string constant id " + std::to_string(stringConstantId.Value())); return stringLengths[ConstantIndex(stringConstantId)];
    }
    Constant GetConstant(ConstantId id) const { Assert(id.Value() < constants.size(), "invalid constant id " + std::to_string(id.Value())); return constants[ConstantIndex(id)]; }
    Constant GetEmptyStringConstant() const { return GetConstant(emptyStringConstantId); }
    ConstantId GetEmptyStringConstantId() const { return emptyStringConstantId; }
    void Write(Writer& writer);
    void Read(Reader& reader);
private:
    ConstantId emptyStringConstantId;
    std::vector<Constant> constants;
    std::unordered_map<Constant, ConstantId, ConstantHash> constantIdMap;
    std::list<utf32_string> strings;
    std::unordered_map<StringPtr, ConstantId, StringPtrHash> stringIdMap;
    std::vector<uint64_t> stringLengths;
    ConstantId NextFreeConstantId() { return int32_t(constants.size()); }
    int32_t ConstantIndex(ConstantId id) const { return id.Value(); }
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_CONSTANT_INCLUDED
