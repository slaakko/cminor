// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_CONSTANT_INCLUDED
#define CMINOR_MACHINE_CONSTANT_INCLUDED
#include <cminor/machine/MachineApi.hpp>
#include <cminor/machine/Object.hpp>
#include <cminor/util/String.hpp>
#include <cminor/util/Unicode.hpp>
#include <cminor/util/CodeFormatter.hpp>

namespace cminor { namespace machine {

class MACHINE_API ConstantId
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

class MACHINE_API Constant
{
public:
    Constant() : value() {}
    Constant(IntegralValue value_);
    IntegralValue Value() const { return value; }
    void SetValue(IntegralValue value_) { value = value_; }
    void Write(Writer& writer);
    void Read(Reader& reader);
    void Dump(CodeFormatter& formatter);
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

class ConstantPoolImpl;

class MACHINE_API ConstantPool
{
public:
    ConstantPool();
    ~ConstantPool();
    ConstantId Install(Constant constant);
    ConstantId Install(StringPtr s);
    ConstantId GetIdFor(Constant constant);
    ConstantId GetIdFor(const std::u32string& s);
    Constant GetConstant(ConstantId id) const;
    Constant GetEmptyStringConstant() const;
    ConstantId GetEmptyStringConstantId() const;
    void Write(Writer& writer);
    void Read(Reader& reader);
    void Dump(CodeFormatter& formatter);
private:
    ConstantPoolImpl* impl;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_CONSTANT_INCLUDED
