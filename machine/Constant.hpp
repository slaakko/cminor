// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_CONSTANT_INCLUDED
#define CMINOR_MACHINE_CONSTANT_INCLUDED
#include <cminor/machine/Object.hpp>

namespace cminor { namespace machine {

class ConstantId
{
public:
    ConstantId() : value(-1) {}
    constexpr ConstantId(int32_t value_) : value(value_) {}
    constexpr int32_t Value() const { return value; }
private:
    int32_t value;
};

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
    Constant GetConstant(ConstantId id) const { assert(id.Value() < constants.size(), "invalid constant id " + std::to_string(id.Value())); return constants[ConstantIndex(id)]; }
    Constant GetEmptyStringConstant() const { return GetConstant(emptyStringConstantId); }
    ConstantId GetEmptyStringConstantId() const { return emptyStringConstantId; }
    void Write(Writer& writer);
    void Read(Reader& reader);
private:
    ConstantId emptyStringConstantId;
    std::vector<Constant> constants;
    std::unordered_map<Constant, ConstantId, ConstantHash> constantIdMap;
    std::list<utf32_string> strings;
    std::unordered_map<utf32_string, ConstantId> stringIdMap;
    ConstantId NextFreeConstantId() { return int32_t(constants.size()); }
    int32_t ConstantIndex(ConstantId id) const { return id.Value(); }
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_CONSTANT_INCLUDED
