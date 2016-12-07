// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Machine.hpp>
#include <cminor/machine/String.hpp>
#include <cminor/machine/Function.hpp>
#include <functional>

namespace cminor { namespace machine {

Machine::Machine() : rootInst(*this, "<root_instruction>", true), managedMemoryPool(*this),
    garbageCollector(*this, defaultGarbageCollectionIntervalMs), gen1Arena(ArenaId::gen1Arena, defaultArenaSize), gen2Arena(ArenaId::gen2Arena, defaultArenaSize), exiting(), exited(), nextFrameId(0)
{
    // no operation:
    rootInst.SetInst(0x00, new NopInst());

    // loading default value of basic types:

    rootInst.SetInst(0x01, new LoadDefaultValueInst<ValueType::sbyteType>("defsb", "System.Int8"));
    rootInst.SetInst(0x02, new LoadDefaultValueInst<ValueType::byteType>("defby", "System.UInt8"));
    rootInst.SetInst(0x03, new LoadDefaultValueInst<ValueType::shortType>("defsh", "System.Int16"));
    rootInst.SetInst(0x04, new LoadDefaultValueInst<ValueType::ushortType>("defus", "System.UInt16"));
    rootInst.SetInst(0x05, new LoadDefaultValueInst<ValueType::intType>("defin", "System.Int32"));
    rootInst.SetInst(0x06, new LoadDefaultValueInst<ValueType::uintType>("defui", "System.UInt32"));
    rootInst.SetInst(0x07, new LoadDefaultValueInst<ValueType::longType>("deflo", "System.Int64"));
    rootInst.SetInst(0x08, new LoadDefaultValueInst<ValueType::ulongType>("deful", "System.UInt64"));
    rootInst.SetInst(0x09, new LoadDefaultValueInst<ValueType::floatType>("deffl", "System.Float"));
    rootInst.SetInst(0x0A, new LoadDefaultValueInst<ValueType::doubleType>("defdo", "System.Double"));
    rootInst.SetInst(0x0B, new LoadDefaultValueInst<ValueType::charType>("defch", "System.Char"));
    rootInst.SetInst(0x0C, new LoadDefaultValueInst<ValueType::boolType>("defbo", "System.Boolean"));

    //  arithmetic unary operators:
    //  ---------------------------

    //  unary plus:
    rootInst.SetInst(0x0D, new UnaryOpInst<int8_t, std::identity<int8_t>, ValueType::sbyteType>("uplussb", "uplus", "System.Int8"));
    rootInst.SetInst(0x0E, new UnaryOpInst<uint8_t, std::identity<uint8_t>, ValueType::byteType>("uplusby", "uplus", "System.UInt8"));
    rootInst.SetInst(0x0F, new UnaryOpInst<int16_t, std::identity<int16_t>, ValueType::shortType>("uplussh", "uplus", "System.Int16"));
    rootInst.SetInst(0x10, new UnaryOpInst<uint16_t, std::identity<uint16_t>, ValueType::ushortType>("uplusus", "uplus", "System.UInt16"));
    rootInst.SetInst(0x11, new UnaryOpInst<int32_t, std::identity<int32_t>, ValueType::intType>("uplusin", "uplus", "System.Int32"));
    rootInst.SetInst(0x12, new UnaryOpInst<uint32_t, std::identity<uint32_t>, ValueType::uintType>("uplusui", "uplus", "System.UInt32"));
    rootInst.SetInst(0x13, new UnaryOpInst<int64_t, std::identity<int64_t>, ValueType::longType>("upluslo", "uplus", "System.Int64"));
    rootInst.SetInst(0x14, new UnaryOpInst<uint64_t, std::identity<uint64_t>, ValueType::ulongType>("uplusul", "uplus", "System.UInt64"));
    rootInst.SetInst(0x15, new UnaryOpInst<float, std::identity<float>, ValueType::floatType>("uplusfl", "uplus", "System.Float"));
    rootInst.SetInst(0x16, new UnaryOpInst<double, std::identity<double>, ValueType::doubleType>("uplusdo", "uplus", "System.Double"));

    //  unary minus:
    rootInst.SetInst(0x17, new UnaryOpInst<int8_t, std::negate<int8_t>, ValueType::sbyteType>("negsb", "neg", "System.Int8"));
    rootInst.SetInst(0x18, new UnaryOpInst<uint8_t, std::negate<uint8_t>, ValueType::byteType>("negby", "neg", "System.UInt8"));
    rootInst.SetInst(0x19, new UnaryOpInst<int16_t, std::negate<int16_t>, ValueType::shortType>("negsh", "neg", "System.Int16"));
    rootInst.SetInst(0x1A, new UnaryOpInst<uint16_t, std::negate<uint16_t>, ValueType::ushortType>("negus", "neg", "System.UInt16"));
    rootInst.SetInst(0x1B, new UnaryOpInst<int32_t, std::negate<int32_t>, ValueType::intType>("negin", "neg", "System.Int32"));
    rootInst.SetInst(0x1C, new UnaryOpInst<uint32_t, std::negate<uint32_t>, ValueType::uintType>("negui", "neg", "System.UInt32"));
    rootInst.SetInst(0x1D, new UnaryOpInst<int64_t, std::negate<int64_t>, ValueType::longType>("neglo", "neg", "System.Int64"));
    rootInst.SetInst(0x1E, new UnaryOpInst<uint64_t, std::negate<uint64_t>, ValueType::ulongType>("negul", "neg", "System.UInt64"));
    rootInst.SetInst(0x1F, new UnaryOpInst<float, std::negate<float>, ValueType::floatType>("negfl", "neg", "System.Float"));
    rootInst.SetInst(0x20, new UnaryOpInst<double, std::negate<double>, ValueType::doubleType>("negdo", "neg", "System.Double"));

    //  bitwise complement:
    rootInst.SetInst(0x21, new UnaryOpInst<int8_t, std::bit_not<int8_t>, ValueType::sbyteType>("cplsb", "cpl", "System.Int8"));
    rootInst.SetInst(0x22, new UnaryOpInst<uint8_t, std::bit_not<uint8_t>, ValueType::byteType>("cplby", "cpl", "System.UInt8"));
    rootInst.SetInst(0x23, new UnaryOpInst<int16_t, std::bit_not<int16_t>, ValueType::shortType>("cplsh", "cpl", "System.Int16"));
    rootInst.SetInst(0x24, new UnaryOpInst<uint16_t, std::bit_not<uint16_t>, ValueType::ushortType>("cplus", "cpl", "System.UInt16"));
    rootInst.SetInst(0x25, new UnaryOpInst<int32_t, std::bit_not<int32_t>, ValueType::intType>("cplin", "cpl", "System.Int32"));
    rootInst.SetInst(0x26, new UnaryOpInst<uint32_t, std::bit_not<uint32_t>, ValueType::uintType>("cplui", "cpl", "System.UInt32"));
    rootInst.SetInst(0x27, new UnaryOpInst<int64_t, std::bit_not<int64_t>, ValueType::longType>("cpllo", "cpl", "System.Int64"));
    rootInst.SetInst(0x28, new UnaryOpInst<uint64_t, std::bit_not<uint64_t>, ValueType::ulongType>("cplul", "cpl", "System.UInt64"));

    // arithmetic binary operators:
    // ----------------------------

    //  add:
    rootInst.SetInst(0x29, new BinaryOpInst<int8_t, std::plus<int8_t>, ValueType::sbyteType>("addsb", "add", "System.Int8"));
    rootInst.SetInst(0x2A, new BinaryOpInst<uint8_t, std::plus<uint8_t>, ValueType::byteType>("addby", "add", "System.UInt8"));
    rootInst.SetInst(0x2B, new BinaryOpInst<int16_t, std::plus<int16_t>, ValueType::shortType>("addsh", "add", "System.Int16"));
    rootInst.SetInst(0x2C, new BinaryOpInst<uint16_t, std::plus<uint16_t>, ValueType::ushortType>("addus", "add", "System.UInt16"));
    rootInst.SetInst(0x2D, new BinaryOpInst<int32_t, std::plus<int32_t>, ValueType::intType>("addin", "add", "System.Int32"));
    rootInst.SetInst(0x2E, new BinaryOpInst<uint32_t, std::plus<uint32_t>, ValueType::uintType>("addui", "add", "System.UInt32"));
    rootInst.SetInst(0x2F, new BinaryOpInst<int64_t, std::plus<int64_t>, ValueType::longType>("addlo", "add", "System.Int64"));
    rootInst.SetInst(0x30, new BinaryOpInst<uint64_t, std::plus<uint64_t>, ValueType::ulongType>("addul", "add", "System.UInt64"));
    rootInst.SetInst(0x31, new BinaryOpInst<float, std::plus<float>, ValueType::floatType>("addfl", "add", "System.Float"));
    rootInst.SetInst(0x32, new BinaryOpInst<double, std::plus<double>, ValueType::doubleType>("adddo", "add", "System.Double"));

    //  subtract:
    rootInst.SetInst(0x33, new BinaryOpInst<int8_t, std::minus<int8_t>, ValueType::sbyteType>("subsb", "sub", "System.Int8"));
    rootInst.SetInst(0x34, new BinaryOpInst<uint8_t, std::minus<uint8_t>, ValueType::byteType>("subby", "sub", "System.UInt8"));
    rootInst.SetInst(0x35, new BinaryOpInst<int16_t, std::minus<int16_t>, ValueType::shortType>("subsh", "sub", "System.Int16"));
    rootInst.SetInst(0x36, new BinaryOpInst<uint16_t, std::minus<uint16_t>, ValueType::ushortType>("subus", "sub", "System.UInt16"));
    rootInst.SetInst(0x37, new BinaryOpInst<int32_t, std::minus<int32_t>, ValueType::intType>("subin", "sub", "System.Int32"));
    rootInst.SetInst(0x38, new BinaryOpInst<uint32_t, std::minus<uint32_t>, ValueType::uintType>("subui", "sub", "System.UInt32"));
    rootInst.SetInst(0x39, new BinaryOpInst<int64_t, std::minus<int64_t>, ValueType::longType>("sublo", "sub", "System.Int64"));
    rootInst.SetInst(0x3A, new BinaryOpInst<uint64_t, std::minus<uint64_t>, ValueType::ulongType>("subul", "sub", "System.UInt64"));
    rootInst.SetInst(0x3B, new BinaryOpInst<float, std::minus<float>, ValueType::floatType>("subfl", "sub", "System.Float"));
    rootInst.SetInst(0x3C, new BinaryOpInst<double, std::minus<double>, ValueType::doubleType>("subdo", "sub", "System.Double"));

    //  multiply:
    rootInst.SetInst(0x3D, new BinaryOpInst<int8_t, std::multiplies<int8_t>, ValueType::sbyteType>("mulsb", "mul", "System.Int8"));
    rootInst.SetInst(0x3E, new BinaryOpInst<uint8_t, std::multiplies<uint8_t>, ValueType::byteType>("mulby", "mul", "System.UInt8"));
    rootInst.SetInst(0x3F, new BinaryOpInst<int16_t, std::multiplies<int16_t>, ValueType::shortType>("mulsh", "mul", "System.Int16"));
    rootInst.SetInst(0x40, new BinaryOpInst<uint16_t, std::multiplies<uint16_t>, ValueType::ushortType>("mulus", "mul", "System.UInt16"));
    rootInst.SetInst(0x41, new BinaryOpInst<int32_t, std::multiplies<int32_t>, ValueType::intType>("mulin", "mul", "System.Int32"));
    rootInst.SetInst(0x42, new BinaryOpInst<uint32_t, std::multiplies<uint32_t>, ValueType::uintType>("mului", "mul", "System.UInt32"));
    rootInst.SetInst(0x43, new BinaryOpInst<int64_t, std::multiplies<int64_t>, ValueType::longType>("mullo", "mul", "System.Int64"));
    rootInst.SetInst(0x44, new BinaryOpInst<uint64_t, std::multiplies<uint64_t>, ValueType::ulongType>("mulul", "mul", "System.UInt64"));
    rootInst.SetInst(0x45, new BinaryOpInst<float, std::multiplies<float>, ValueType::floatType>("mulfl", "mul", "System.Float"));
    rootInst.SetInst(0x46, new BinaryOpInst<double, std::multiplies<double>, ValueType::doubleType>("muldo", "mul", "System.Double"));

    //  divide:
    rootInst.SetInst(0x47, new BinaryOpInst<int8_t, std::divides<int8_t>, ValueType::sbyteType>("divsb", "div", "System.Int8"));
    rootInst.SetInst(0x48, new BinaryOpInst<uint8_t, std::divides<uint8_t>, ValueType::byteType>("divby", "div", "System.UInt8"));
    rootInst.SetInst(0x49, new BinaryOpInst<int16_t, std::divides<int16_t>, ValueType::shortType>("divsh", "div", "System.Int16"));
    rootInst.SetInst(0x4A, new BinaryOpInst<uint16_t, std::divides<uint16_t>, ValueType::ushortType>("divus", "div", "System.UInt16"));
    rootInst.SetInst(0x4B, new BinaryOpInst<int32_t, std::divides<int32_t>, ValueType::intType>("divin", "div", "System.Int32"));
    rootInst.SetInst(0x4C, new BinaryOpInst<uint32_t, std::divides<uint32_t>, ValueType::uintType>("divui", "div", "System.UInt32"));
    rootInst.SetInst(0x4D, new BinaryOpInst<int64_t, std::divides<int64_t>, ValueType::longType>("divlo", "div", "System.Int64"));
    rootInst.SetInst(0x4E, new BinaryOpInst<uint64_t, std::divides<uint64_t>, ValueType::ulongType>("divul", "div", "System.UInt64"));
    rootInst.SetInst(0x4F, new BinaryOpInst<float, std::divides<float>, ValueType::floatType>("divfl", "div", "System.Float"));
    rootInst.SetInst(0x50, new BinaryOpInst<double, std::divides<double>, ValueType::doubleType>("divdo", "div", "System.Double"));

    //  remainder:
    rootInst.SetInst(0x51, new BinaryOpInst<int8_t, std::modulus<int8_t>, ValueType::sbyteType>("remsb", "rem", "System.Int8"));
    rootInst.SetInst(0x52, new BinaryOpInst<uint8_t, std::modulus<uint8_t>, ValueType::byteType>("remby", "rem", "System.UInt8"));
    rootInst.SetInst(0x53, new BinaryOpInst<int16_t, std::modulus<int16_t>, ValueType::shortType>("remsh", "rem", "System.Int16"));
    rootInst.SetInst(0x54, new BinaryOpInst<uint16_t, std::modulus<uint16_t>, ValueType::ushortType>("remus", "rem", "System.UInt16"));
    rootInst.SetInst(0x55, new BinaryOpInst<int32_t, std::modulus<int32_t>, ValueType::intType>("remin", "rem", "System.Int32"));
    rootInst.SetInst(0x56, new BinaryOpInst<uint32_t, std::modulus<uint32_t>, ValueType::uintType>("remui", "rem", "System.UInt32"));
    rootInst.SetInst(0x57, new BinaryOpInst<int64_t, std::modulus<int64_t>, ValueType::longType>("remlo", "rem", "System.Int64"));
    rootInst.SetInst(0x58, new BinaryOpInst<uint64_t, std::modulus<uint64_t>, ValueType::ulongType>("remul", "rem", "System.UInt64"));

    //  bitwise and:
    rootInst.SetInst(0x59, new BinaryOpInst<int8_t, std::bit_and<int8_t>, ValueType::sbyteType>("andsb", "and", "System.Int8"));
    rootInst.SetInst(0x5A, new BinaryOpInst<uint8_t, std::bit_and<uint8_t>, ValueType::byteType>("andby", "and", "System.UInt8"));
    rootInst.SetInst(0x5B, new BinaryOpInst<int16_t, std::bit_and<int16_t>, ValueType::shortType>("andsh", "and", "System.Int16"));
    rootInst.SetInst(0x5C, new BinaryOpInst<uint16_t, std::bit_and<uint16_t>, ValueType::ushortType>("andus", "and", "System.UInt16"));
    rootInst.SetInst(0x5D, new BinaryOpInst<int32_t, std::bit_and<int32_t>, ValueType::intType>("andin", "and", "System.Int32"));
    rootInst.SetInst(0x5E, new BinaryOpInst<uint32_t, std::bit_and<uint32_t>, ValueType::uintType>("andui", "and", "System.UInt32"));
    rootInst.SetInst(0x5F, new BinaryOpInst<int64_t, std::bit_and<int64_t>, ValueType::longType>("andlo", "and", "System.Int64"));
    rootInst.SetInst(0x60, new BinaryOpInst<uint64_t, std::bit_and<uint64_t>, ValueType::ulongType>("andul", "and", "System.UInt64"));

    //  bitwise or:
    rootInst.SetInst(0x61, new BinaryOpInst<int8_t, std::bit_or<int8_t>, ValueType::sbyteType>("orsb", "or", "System.Int8"));
    rootInst.SetInst(0x62, new BinaryOpInst<uint8_t, std::bit_or<uint8_t>, ValueType::byteType>("orby", "or", "System.UInt8"));
    rootInst.SetInst(0x63, new BinaryOpInst<int16_t, std::bit_or<int16_t>, ValueType::shortType>("orsh", "or", "System.Int16"));
    rootInst.SetInst(0x64, new BinaryOpInst<uint16_t, std::bit_or<uint16_t>, ValueType::ushortType>("orus", "or", "System.UInt16"));
    rootInst.SetInst(0x65, new BinaryOpInst<int32_t, std::bit_or<int32_t>, ValueType::intType>("orin", "or", "System.Int32"));
    rootInst.SetInst(0x66, new BinaryOpInst<uint32_t, std::bit_or<uint32_t>, ValueType::uintType>("orui", "or", "System.UInt32"));
    rootInst.SetInst(0x67, new BinaryOpInst<int64_t, std::bit_or<int64_t>, ValueType::longType>("orlo", "or", "System.Int64"));
    rootInst.SetInst(0x68, new BinaryOpInst<uint64_t, std::bit_or<uint64_t>, ValueType::ulongType>("orul", "or", "System.UInt64"));

    //  bitwise xor:
    rootInst.SetInst(0x69, new BinaryOpInst<int8_t, std::bit_xor<int8_t>, ValueType::sbyteType>("xorsb", "xor", "System.Int8"));
    rootInst.SetInst(0x6A, new BinaryOpInst<uint8_t, std::bit_xor<uint8_t>, ValueType::byteType>("xorby", "xor", "System.UInt8"));
    rootInst.SetInst(0x6B, new BinaryOpInst<int16_t, std::bit_xor<int16_t>, ValueType::shortType>("xorsh", "xor", "System.Int16"));
    rootInst.SetInst(0x6C, new BinaryOpInst<uint16_t, std::bit_xor<uint16_t>, ValueType::ushortType>("xorus", "xor", "System.UInt16"));
    rootInst.SetInst(0x6D, new BinaryOpInst<int32_t, std::bit_xor<int32_t>, ValueType::intType>("xorin", "xor", "System.Int32"));
    rootInst.SetInst(0x6E, new BinaryOpInst<uint32_t, std::bit_xor<uint32_t>, ValueType::uintType>("xorui", "xor", "System.UInt32"));
    rootInst.SetInst(0x6F, new BinaryOpInst<int64_t, std::bit_xor<int64_t>, ValueType::longType>("xorlo", "xor", "System.Int64"));
    rootInst.SetInst(0x70, new BinaryOpInst<uint64_t, std::bit_xor<uint64_t>, ValueType::ulongType>("xorul", "xor", "System.UInt64"));

    //  shift left:
    rootInst.SetInst(0x71, new BinaryOpInst<int8_t, ShiftLeft<int8_t>, ValueType::sbyteType>("shlsb", "shl", "System.Int8"));
    rootInst.SetInst(0x72, new BinaryOpInst<uint8_t, ShiftLeft<uint8_t>, ValueType::byteType>("shlby", "shl", "System.UInt8"));
    rootInst.SetInst(0x73, new BinaryOpInst<int16_t, ShiftLeft<int16_t>, ValueType::shortType>("shlsh", "shl", "System.Int16"));
    rootInst.SetInst(0x74, new BinaryOpInst<uint16_t, ShiftLeft<uint16_t>, ValueType::ushortType>("shlus", "shl", "System.UInt16"));
    rootInst.SetInst(0x75, new BinaryOpInst<int32_t, ShiftLeft<int32_t>, ValueType::intType>("shlin", "shl", "System.Int32"));
    rootInst.SetInst(0x76, new BinaryOpInst<uint32_t, ShiftLeft<uint32_t>, ValueType::uintType>("shlui", "shl", "System.UInt32"));
    rootInst.SetInst(0x77, new BinaryOpInst<int64_t, ShiftLeft<int64_t>, ValueType::longType>("shllo", "shl", "System.Int64"));
    rootInst.SetInst(0x78, new BinaryOpInst<uint64_t, ShiftLeft<uint64_t>, ValueType::ulongType>("shlul", "shl", "System.UInt64"));

    //  shift right:
    rootInst.SetInst(0x79, new BinaryOpInst<int8_t, ShiftRight<int8_t>, ValueType::sbyteType>("shrsb", "shr", "System.Int8"));
    rootInst.SetInst(0x7A, new BinaryOpInst<uint8_t, ShiftRight<uint8_t>, ValueType::byteType>("shrby", "shr", "System.UInt8"));
    rootInst.SetInst(0x7B, new BinaryOpInst<int16_t, ShiftRight<int16_t>, ValueType::shortType>("shrsh", "shr", "System.Int16"));
    rootInst.SetInst(0x7C, new BinaryOpInst<uint16_t, ShiftRight<uint16_t>, ValueType::ushortType>("shrus", "shr", "System.UInt16"));
    rootInst.SetInst(0x7D, new BinaryOpInst<int32_t, ShiftRight<int32_t>, ValueType::intType>("shrin", "shr", "System.Int32"));
    rootInst.SetInst(0x7E, new BinaryOpInst<uint32_t, ShiftRight<uint32_t>, ValueType::uintType>("shrui", "shr", "System.UInt32"));
    rootInst.SetInst(0x7F, new BinaryOpInst<int64_t, ShiftRight<int64_t>, ValueType::longType>("shrlo", "shr", "System.Int64"));
    rootInst.SetInst(0x80, new BinaryOpInst<uint64_t, ShiftRight<uint64_t>, ValueType::ulongType>("shrul", "shr", "System.UInt64"));

    // comparisons:
    // ------------

    //  equal:
    rootInst.SetInst(0x81, new BinaryPredInst<int8_t, std::equal_to<int8_t>, ValueType::sbyteType>("equalsb", "equal", "System.Int8"));
    rootInst.SetInst(0x82, new BinaryPredInst<uint8_t, std::equal_to<uint8_t>, ValueType::byteType>("equalby", "equal", "System.UInt8"));
    rootInst.SetInst(0x83, new BinaryPredInst<int16_t, std::equal_to<int16_t>, ValueType::shortType>("equalsh", "equal", "System.Int16"));
    rootInst.SetInst(0x84, new BinaryPredInst<uint16_t, std::equal_to<uint16_t>, ValueType::ushortType>("equalus", "equal", "System.UInt16"));
    rootInst.SetInst(0x85, new BinaryPredInst<int32_t, std::equal_to<int32_t>, ValueType::intType>("equalin", "equal", "System.Int32"));
    rootInst.SetInst(0x86, new BinaryPredInst<uint32_t, std::equal_to<uint32_t>, ValueType::uintType>("equalui", "equal", "System.UInt32"));
    rootInst.SetInst(0x87, new BinaryPredInst<int64_t, std::equal_to<int64_t>, ValueType::longType>("equallo", "equal", "System.Int64"));
    rootInst.SetInst(0x88, new BinaryPredInst<uint64_t, std::equal_to<uint64_t>, ValueType::ulongType>("equalul", "equal", "System.UInt64"));
    rootInst.SetInst(0x89, new BinaryPredInst<float, std::equal_to<float>, ValueType::floatType>("equalfl", "equal", "System.Float"));
    rootInst.SetInst(0x8A, new BinaryPredInst<double, std::equal_to<double>, ValueType::doubleType>("equaldo", "equal", "System.Double"));
    rootInst.SetInst(0x8B, new BinaryPredInst<char32_t, std::equal_to<char32_t>, ValueType::charType>("equalch", "equal", "System.Char"));
    rootInst.SetInst(0x8C, new BinaryPredInst<bool, std::equal_to<bool>, ValueType::boolType>("equalbo", "equal", "System.Boolean"));

    //  less:
    rootInst.SetInst(0x8D, new BinaryPredInst<int8_t, std::less<int8_t>, ValueType::sbyteType>("lesssb", "less", "System.Int8"));
    rootInst.SetInst(0x8E, new BinaryPredInst<uint8_t, std::less<uint8_t>, ValueType::byteType>("lessby", "less", "System.UInt8"));
    rootInst.SetInst(0x8F, new BinaryPredInst<int16_t, std::less<int16_t>, ValueType::shortType>("lesssh", "less", "System.Int16"));
    rootInst.SetInst(0x90, new BinaryPredInst<uint16_t, std::less<uint16_t>, ValueType::ushortType>("lessus", "less", "System.UInt16"));
    rootInst.SetInst(0x91, new BinaryPredInst<int32_t, std::less<int32_t>, ValueType::intType>("lessin", "less", "System.Int32"));
    rootInst.SetInst(0x92, new BinaryPredInst<uint32_t, std::less<uint32_t>, ValueType::uintType>("lessui", "less", "System.UInt32"));
    rootInst.SetInst(0x93, new BinaryPredInst<int64_t, std::less<int64_t>, ValueType::longType>("lesslo", "less", "System.Int64"));
    rootInst.SetInst(0x94, new BinaryPredInst<uint64_t, std::less<uint64_t>, ValueType::ulongType>("lessul", "less", "System.UInt64"));
    rootInst.SetInst(0x95, new BinaryPredInst<float, std::less<float>, ValueType::floatType>("lessfl", "less", "System.Float"));
    rootInst.SetInst(0x96, new BinaryPredInst<double, std::less<double>, ValueType::doubleType>("lessdo", "less", "System.Double"));
    rootInst.SetInst(0x97, new BinaryPredInst<char32_t, std::less<char32_t>, ValueType::charType>("lessch", "less", "System.Char"));
    rootInst.SetInst(0x98, new BinaryPredInst<bool, std::less<bool>, ValueType::boolType>("lessbo", "less", "System.Boolean"));

    //  logical operations:
    //  -------------------
    rootInst.SetInst(0x99, new LogicalNotInst());


    //  load & store:
    //  -------------

    rootInst.SetInst(0x9A, new LoadLocalInst());
    rootInst.SetInst(0x9B, new LoadLocal0Inst());
    rootInst.SetInst(0x9C, new LoadLocal1Inst());
    rootInst.SetInst(0x9D, new LoadLocal2Inst());
    rootInst.SetInst(0x9E, new LoadLocal3Inst());
    rootInst.SetInst(0x9F, new LoadLocalBInst());
    rootInst.SetInst(0xA0, new LoadLocalSInst());
    rootInst.SetInst(0xA1, new StoreLocalInst());
    rootInst.SetInst(0xA2, new StoreLocal0Inst());
    rootInst.SetInst(0xA3, new StoreLocal1Inst());
    rootInst.SetInst(0xA4, new StoreLocal2Inst());
    rootInst.SetInst(0xA5, new StoreLocal3Inst());
    rootInst.SetInst(0xA6, new StoreLocalBInst());
    rootInst.SetInst(0xA7, new StoreLocalSInst());
    rootInst.SetInst(0xA8, new LoadFieldInst());
    rootInst.SetInst(0xA9, new LoadField0Inst());
    rootInst.SetInst(0xAA, new LoadField1Inst());
    rootInst.SetInst(0xAB, new LoadField2Inst());
    rootInst.SetInst(0xAC, new LoadField3Inst());
    rootInst.SetInst(0xAD, new LoadFieldBInst());
    rootInst.SetInst(0xAE, new LoadFieldSInst());
    rootInst.SetInst(0xAF, new StoreFieldInst());
    rootInst.SetInst(0xB0, new StoreField0Inst());
    rootInst.SetInst(0xB1, new StoreField1Inst());
    rootInst.SetInst(0xB2, new StoreField2Inst());
    rootInst.SetInst(0xB3, new StoreField3Inst());
    rootInst.SetInst(0xB4, new StoreFieldBInst());
    rootInst.SetInst(0xB5, new StoreFieldSInst());
    rootInst.SetInst(0xB6, new LoadElemInst());
    rootInst.SetInst(0xB7, new StoreElemInst());
    rootInst.SetInst(0xB8, new LoadConstantInst());
    rootInst.SetInst(0xB9, new LoadConstantBInst());
    rootInst.SetInst(0xBA, new LoadConstantSInst());
    rootInst.SetInst(0xBC, new ReceiveInst());

    // jump & call:
    // ------------

    rootInst.SetInst(0xBD, new JumpInst());
    rootInst.SetInst(0xBE, new JumpTrueInst());
    rootInst.SetInst(0xBF, new JumpFalseInst());
    rootInst.SetInst(0xC0, new CallInst());
    rootInst.SetInst(0xC1, new VirtualCallInst());
    rootInst.SetInst(0xC2, new InterfaceCallInst());
    rootInst.SetInst(0xC3, new EnterBlockInst());
    rootInst.SetInst(0xC4, new ExitBlockInst());
    rootInst.SetInst(0xC5, new ContinuousSwitchInst());
    rootInst.SetInst(0xC6, new BinarySearchSwitchInst());

    // stack manipulation:
    // -------------------

    rootInst.SetInst(0xC7, new DupInst());
    rootInst.SetInst(0xC8, new SwapInst());
    rootInst.SetInst(0xC9, new RotateInst());
    rootInst.SetInst(0xCA, new PopInst());


    // objects:
    // --------

    rootInst.SetInst(0xCB, new LoadDefaultValueInst<ValueType::objectReference>("defo", "object"));
    rootInst.SetInst(0xCC, new EqualObjectNullInst());
    rootInst.SetInst(0xCD, new EqualNullObjectInst());
    rootInst.SetInst(0xCE, new CreateObjectInst());
    rootInst.SetInst(0xCF, new CopyObjectInst());
    rootInst.SetInst(0xD0, new SetClassDataInst());
    rootInst.SetInst(0xD1, new UpCastInst());
    rootInst.SetInst(0xD2, new DownCastInst());
    rootInst.SetInst(0xD3, new ThrowInst());
    rootInst.SetInst(0xD4, new RethrowInst());
    rootInst.SetInst(0xD5, new EndCatchInst());
    rootInst.SetInst(0xD6, new EndFinallyInst());
    rootInst.SetInst(0xD7, new NextInst());
    rootInst.SetInst(0xD8, new StaticInitInst());
    rootInst.SetInst(0xD9, new DoneStaticInitInst());
    rootInst.SetInst(0xDA, new LoadStaticFieldInst());
    rootInst.SetInst(0xDB, new StoreStaticFieldInst());
    rootInst.SetInst(0xDC, new IsInst());
    rootInst.SetInst(0xDD, new AsInst());

    // strings:
    // --------

    rootInst.SetInst(0xDE, new StrLitToStringInst());
    rootInst.SetInst(0xDF, new LoadStringCharInst());

    // arrays:
    // -------

    rootInst.SetInst(0xE0, new AllocateArrayElementsInst());

    // vmcall:
    // -------

    rootInst.SetInst(0xF0, new VmCallInst());

    //  conversion group instruction:
    //  -----------------------------

    ContainerInst* convContainerInst = new ContainerInst(*this, "<conversion_container_instruction>", false);
    rootInst.SetInst(0xFE, convContainerInst);

    //  conversions (prefixed by FE):
    //  -----------------------------

    // from sbyte:
    convContainerInst->SetInst(0x00, new ConversionInst<int8_t, uint8_t, ValueType::byteType>("sb2by"));
    convContainerInst->SetInst(0x01, new ConversionInst<int8_t, int16_t, ValueType::shortType>("sb2sh"));
    convContainerInst->SetInst(0x02, new ConversionInst<int8_t, uint16_t, ValueType::ushortType>("sb2us"));
    convContainerInst->SetInst(0x03, new ConversionInst<int8_t, int32_t, ValueType::intType>("sb2in"));
    convContainerInst->SetInst(0x04, new ConversionInst<int8_t, uint32_t, ValueType::uintType>("sb2ui"));
    convContainerInst->SetInst(0x05, new ConversionInst<int8_t, int64_t, ValueType::longType>("sb2lo"));
    convContainerInst->SetInst(0x06, new ConversionInst<int8_t, uint64_t, ValueType::ulongType>("sb2ul"));
    convContainerInst->SetInst(0x07, new ConversionInst<int8_t, float, ValueType::floatType>("sb2fl"));
    convContainerInst->SetInst(0x08, new ConversionInst<int8_t, double, ValueType::doubleType>("sb2do"));
    convContainerInst->SetInst(0x0B, new ConversionInst<int8_t, char32_t, ValueType::charType>("sb2ch"));
    convContainerInst->SetInst(0x0C, new ConversionInst<int8_t, bool, ValueType::boolType>("sb2bo"));

    // from byte:
    convContainerInst->SetInst(0x0D, new ConversionInst<uint8_t, int8_t, ValueType::sbyteType>("by2sb"));
    convContainerInst->SetInst(0x0E, new ConversionInst<uint8_t, int16_t, ValueType::shortType>("by2sh"));
    convContainerInst->SetInst(0x0F, new ConversionInst<uint8_t, uint16_t, ValueType::ushortType>("by2us"));
    convContainerInst->SetInst(0x10, new ConversionInst<uint8_t, int32_t, ValueType::intType>("by2in"));
    convContainerInst->SetInst(0x11, new ConversionInst<uint8_t, uint32_t, ValueType::uintType>("by2ui"));
    convContainerInst->SetInst(0x12, new ConversionInst<uint8_t, int64_t, ValueType::longType>("by2lo"));
    convContainerInst->SetInst(0x13, new ConversionInst<uint8_t, uint64_t, ValueType::ulongType>("by2ul"));
    convContainerInst->SetInst(0x14, new ConversionInst<uint8_t, float, ValueType::floatType>("by2fl"));
    convContainerInst->SetInst(0x15, new ConversionInst<uint8_t, double, ValueType::doubleType>("by2do"));
    convContainerInst->SetInst(0x18, new ConversionInst<uint8_t, char32_t, ValueType::charType>("by2ch"));
    convContainerInst->SetInst(0x19, new ConversionInst<uint8_t, bool, ValueType::boolType>("by2bo"));

    // from short:
    convContainerInst->SetInst(0x1A, new ConversionInst<int16_t, int8_t, ValueType::sbyteType>("sh2sb"));
    convContainerInst->SetInst(0x1B, new ConversionInst<int16_t, uint8_t, ValueType::byteType>("sh2by"));
    convContainerInst->SetInst(0x1C, new ConversionInst<int16_t, uint16_t, ValueType::ushortType>("sh2us"));
    convContainerInst->SetInst(0x1D, new ConversionInst<int16_t, int32_t, ValueType::intType>("sh2in"));
    convContainerInst->SetInst(0x1E, new ConversionInst<int16_t, uint32_t, ValueType::uintType>("sh2ui"));
    convContainerInst->SetInst(0x1F, new ConversionInst<int16_t, int64_t, ValueType::longType>("sh2lo"));
    convContainerInst->SetInst(0x20, new ConversionInst<int16_t, uint64_t, ValueType::ulongType>("sh2ul"));
    convContainerInst->SetInst(0x21, new ConversionInst<int16_t, float, ValueType::floatType>("sh2fl"));
    convContainerInst->SetInst(0x22, new ConversionInst<int16_t, double, ValueType::doubleType>("sh2do"));
    convContainerInst->SetInst(0x25, new ConversionInst<int16_t, char32_t, ValueType::charType>("sh2ch"));
    convContainerInst->SetInst(0x26, new ConversionInst<int16_t, bool, ValueType::boolType>("sh2bo"));

    // from ushort:
    convContainerInst->SetInst(0x27, new ConversionInst<uint16_t, int8_t, ValueType::sbyteType>("us2sb"));
    convContainerInst->SetInst(0x28, new ConversionInst<uint16_t, uint8_t, ValueType::byteType>("us2by"));
    convContainerInst->SetInst(0x29, new ConversionInst<uint16_t, int16_t, ValueType::shortType>("us2sh"));
    convContainerInst->SetInst(0x2A, new ConversionInst<uint16_t, int32_t, ValueType::intType>("us2in"));
    convContainerInst->SetInst(0x2B, new ConversionInst<uint16_t, uint32_t, ValueType::uintType>("us2ui"));
    convContainerInst->SetInst(0x2C, new ConversionInst<uint16_t, int64_t, ValueType::longType>("us2lo"));
    convContainerInst->SetInst(0x2D, new ConversionInst<uint16_t, uint64_t, ValueType::ulongType>("us2ul"));
    convContainerInst->SetInst(0x2E, new ConversionInst<uint16_t, float, ValueType::floatType>("us2fl"));
    convContainerInst->SetInst(0x2F, new ConversionInst<uint16_t, double, ValueType::doubleType>("us2do"));
    convContainerInst->SetInst(0x32, new ConversionInst<uint16_t, char32_t, ValueType::charType>("us2ch"));
    convContainerInst->SetInst(0x33, new ConversionInst<uint16_t, bool, ValueType::boolType>("us2bo"));

    // from int:
    convContainerInst->SetInst(0x34, new ConversionInst<int32_t, int8_t, ValueType::sbyteType>("in2sb"));
    convContainerInst->SetInst(0x35, new ConversionInst<int32_t, uint8_t, ValueType::byteType>("in2by"));
    convContainerInst->SetInst(0x36, new ConversionInst<int32_t, int16_t, ValueType::shortType>("in2sh"));
    convContainerInst->SetInst(0x37, new ConversionInst<int32_t, uint16_t, ValueType::ushortType>("in2us"));
    convContainerInst->SetInst(0x38, new ConversionInst<int32_t, uint32_t, ValueType::uintType>("in2ui"));
    convContainerInst->SetInst(0x39, new ConversionInst<int32_t, int64_t, ValueType::longType>("in2lo"));
    convContainerInst->SetInst(0x3A, new ConversionInst<int32_t, uint64_t, ValueType::ulongType>("in2ul"));
    convContainerInst->SetInst(0x3B, new ConversionInst<int32_t, float, ValueType::floatType>("in2fl"));
    convContainerInst->SetInst(0x3C, new ConversionInst<int32_t, double, ValueType::doubleType>("in2do"));
    convContainerInst->SetInst(0x3F, new ConversionInst<int32_t, char32_t, ValueType::charType>("in2ch"));
    convContainerInst->SetInst(0x40, new ConversionInst<int32_t, bool, ValueType::boolType>("in2bo"));

    // from uint:
    convContainerInst->SetInst(0x41, new ConversionInst<uint32_t, int8_t, ValueType::sbyteType>("ui2sb"));
    convContainerInst->SetInst(0x42, new ConversionInst<uint32_t, uint8_t, ValueType::byteType>("ui2by"));
    convContainerInst->SetInst(0x43, new ConversionInst<uint32_t, int16_t, ValueType::shortType>("ui2sh"));
    convContainerInst->SetInst(0x44, new ConversionInst<uint32_t, uint16_t, ValueType::ushortType>("ui2us"));
    convContainerInst->SetInst(0x45, new ConversionInst<uint32_t, int32_t, ValueType::intType>("ui2in"));
    convContainerInst->SetInst(0x46, new ConversionInst<uint32_t, int64_t, ValueType::longType>("ui2lo"));
    convContainerInst->SetInst(0x47, new ConversionInst<uint32_t, uint64_t, ValueType::ulongType>("ui2ul"));
    convContainerInst->SetInst(0x48, new ConversionInst<uint32_t, float, ValueType::floatType>("ui2fl"));
    convContainerInst->SetInst(0x49, new ConversionInst<uint32_t, double, ValueType::doubleType>("ui2do"));
    convContainerInst->SetInst(0x4C, new ConversionInst<uint32_t, char32_t, ValueType::charType>("ui2ch"));
    convContainerInst->SetInst(0x4D, new ConversionInst<uint32_t, bool, ValueType::boolType>("ui2bo"));

    // from long:
    convContainerInst->SetInst(0x4E, new ConversionInst<int64_t, int8_t, ValueType::sbyteType>("lo2sb"));
    convContainerInst->SetInst(0x4F, new ConversionInst<int64_t, uint8_t, ValueType::byteType>("lo2by"));
    convContainerInst->SetInst(0x50, new ConversionInst<int64_t, int16_t, ValueType::shortType>("lo2sh"));
    convContainerInst->SetInst(0x51, new ConversionInst<int64_t, uint16_t, ValueType::ushortType>("lo2us"));
    convContainerInst->SetInst(0x52, new ConversionInst<int64_t, int32_t, ValueType::intType>("lo2in"));
    convContainerInst->SetInst(0x53, new ConversionInst<int64_t, uint32_t, ValueType::uintType>("lo2ui"));
    convContainerInst->SetInst(0x54, new ConversionInst<int64_t, uint64_t, ValueType::ulongType>("lo2ul"));
    convContainerInst->SetInst(0x55, new ConversionInst<int64_t, float, ValueType::floatType>("lo2fl"));
    convContainerInst->SetInst(0x56, new ConversionInst<int64_t, double, ValueType::doubleType>("lo2do"));
    convContainerInst->SetInst(0x59, new ConversionInst<int64_t, char32_t, ValueType::charType>("lo2ch"));
    convContainerInst->SetInst(0x5A, new ConversionInst<int64_t, bool, ValueType::boolType>("lo2bo"));

    // from ulong:
    convContainerInst->SetInst(0x5B, new ConversionInst<uint64_t, int8_t, ValueType::sbyteType>("ul2sb"));
    convContainerInst->SetInst(0x5C, new ConversionInst<uint64_t, uint8_t, ValueType::byteType>("ul2by"));
    convContainerInst->SetInst(0x5D, new ConversionInst<uint64_t, int16_t, ValueType::shortType>("ul2sh"));
    convContainerInst->SetInst(0x5E, new ConversionInst<uint64_t, uint16_t, ValueType::ushortType>("ul2us"));
    convContainerInst->SetInst(0x5F, new ConversionInst<uint64_t, int32_t, ValueType::intType>("ul2in"));
    convContainerInst->SetInst(0x60, new ConversionInst<uint64_t, uint32_t, ValueType::uintType>("ul2ui"));
    convContainerInst->SetInst(0x61, new ConversionInst<uint64_t, int64_t, ValueType::longType>("ul2lo"));
    convContainerInst->SetInst(0x62, new ConversionInst<uint64_t, float, ValueType::floatType>("ul2fl"));
    convContainerInst->SetInst(0x63, new ConversionInst<uint64_t, double, ValueType::doubleType>("ul2do"));
    convContainerInst->SetInst(0x66, new ConversionInst<uint64_t, char32_t, ValueType::charType>("ul2ch"));
    convContainerInst->SetInst(0x67, new ConversionInst<uint64_t, bool, ValueType::boolType>("ul2bo"));

    // from float:
    convContainerInst->SetInst(0x68, new ConversionInst<float, int8_t, ValueType::sbyteType>("fl2sb"));
    convContainerInst->SetInst(0x69, new ConversionInst<float, uint8_t, ValueType::byteType>("fl2by"));
    convContainerInst->SetInst(0x6A, new ConversionInst<float, int16_t, ValueType::shortType>("fl2sh"));
    convContainerInst->SetInst(0x6B, new ConversionInst<float, uint16_t, ValueType::ushortType>("fl2us"));
    convContainerInst->SetInst(0x6C, new ConversionInst<float, int32_t, ValueType::intType>("fl2in"));
    convContainerInst->SetInst(0x6D, new ConversionInst<float, uint32_t, ValueType::uintType>("fl2ui"));
    convContainerInst->SetInst(0x6E, new ConversionInst<float, int64_t, ValueType::longType>("fl2lo"));
    convContainerInst->SetInst(0x6F, new ConversionInst<float, uint64_t, ValueType::ulongType>("fl2ul"));
    convContainerInst->SetInst(0x70, new ConversionInst<float, double, ValueType::doubleType>("fl2do"));
    convContainerInst->SetInst(0x73, new ConversionInst<float, char32_t, ValueType::charType>("fl2ch"));
    convContainerInst->SetInst(0x74, new ConversionInst<float, bool, ValueType::boolType>("fl2bo"));

    // from double:
    convContainerInst->SetInst(0x75, new ConversionInst<double, int8_t, ValueType::sbyteType>("do2sb"));
    convContainerInst->SetInst(0x76, new ConversionInst<double, uint8_t, ValueType::byteType>("do2by"));
    convContainerInst->SetInst(0x77, new ConversionInst<double, int16_t, ValueType::shortType>("do2sh"));
    convContainerInst->SetInst(0x78, new ConversionInst<double, uint16_t, ValueType::ushortType>("do2us"));
    convContainerInst->SetInst(0x79, new ConversionInst<double, int32_t, ValueType::intType>("do2in"));
    convContainerInst->SetInst(0x7A, new ConversionInst<double, uint32_t, ValueType::uintType>("do2ui"));
    convContainerInst->SetInst(0x7B, new ConversionInst<double, int64_t, ValueType::longType>("do2lo"));
    convContainerInst->SetInst(0x7C, new ConversionInst<double, uint64_t, ValueType::ulongType>("do2ul"));
    convContainerInst->SetInst(0x7D, new ConversionInst<double, float, ValueType::floatType>("do2fl"));
    convContainerInst->SetInst(0x80, new ConversionInst<double, char32_t, ValueType::charType>("do2ch"));
    convContainerInst->SetInst(0x81, new ConversionInst<double, bool, ValueType::boolType>("do2bo"));

    // from char:
    convContainerInst->SetInst(0xA2, new ConversionInst<char32_t, int8_t, ValueType::sbyteType>("ch2sb"));
    convContainerInst->SetInst(0xA3, new ConversionInst<char32_t, uint8_t, ValueType::byteType>("ch2by"));
    convContainerInst->SetInst(0xA4, new ConversionInst<char32_t, int16_t, ValueType::shortType>("ch2sh"));
    convContainerInst->SetInst(0xA5, new ConversionInst<char32_t, uint16_t, ValueType::ushortType>("ch2us"));
    convContainerInst->SetInst(0xA6, new ConversionInst<char32_t, int32_t, ValueType::intType>("ch2in"));
    convContainerInst->SetInst(0xA7, new ConversionInst<char32_t, uint32_t, ValueType::uintType>("ch2ui"));
    convContainerInst->SetInst(0xA8, new ConversionInst<char32_t, int64_t, ValueType::longType>("ch2lo"));
    convContainerInst->SetInst(0xA9, new ConversionInst<char32_t, uint64_t, ValueType::ulongType>("ch2ul"));
    convContainerInst->SetInst(0xAA, new ConversionInst<char32_t, float, ValueType::floatType>("ch2fl"));
    convContainerInst->SetInst(0xAB, new ConversionInst<char32_t, double, ValueType::doubleType>("ch2do"));
    convContainerInst->SetInst(0xAE, new ConversionInst<char32_t, bool, ValueType::boolType>("ch2bo"));

    // from bool:
    convContainerInst->SetInst(0xAF, new ConversionInst<bool, int8_t, ValueType::sbyteType>("bo2sb"));
    convContainerInst->SetInst(0xB0, new ConversionInst<bool, uint8_t, ValueType::byteType>("bo2by"));
    convContainerInst->SetInst(0xB1, new ConversionInst<bool, int16_t, ValueType::shortType>("bo2sh"));
    convContainerInst->SetInst(0xB2, new ConversionInst<bool, uint16_t, ValueType::ushortType>("bo2us"));
    convContainerInst->SetInst(0xB3, new ConversionInst<bool, int32_t, ValueType::intType>("bo2in"));
    convContainerInst->SetInst(0xB4, new ConversionInst<bool, uint32_t, ValueType::uintType>("bo2ui"));
    convContainerInst->SetInst(0xB5, new ConversionInst<bool, int64_t, ValueType::longType>("bo2lo"));
    convContainerInst->SetInst(0xB6, new ConversionInst<bool, uint64_t, ValueType::ulongType>("bo2ul"));
    convContainerInst->SetInst(0xB7, new ConversionInst<bool, float, ValueType::floatType>("bo2fl"));
    convContainerInst->SetInst(0xB8, new ConversionInst<bool, double, ValueType::doubleType>("bo2do"));
    convContainerInst->SetInst(0xB9, new ConversionInst<bool, char32_t, ValueType::charType>("bo2ch"));

//  boxing:
    convContainerInst->SetInst(0xBA, new BoxInst<ValueType::sbyteType>("sb2o"));
    convContainerInst->SetInst(0xBB, new BoxInst<ValueType::byteType>("by2o"));
    convContainerInst->SetInst(0xBC, new BoxInst<ValueType::shortType>("sh2o"));
    convContainerInst->SetInst(0xBD, new BoxInst<ValueType::ushortType>("us2o"));
    convContainerInst->SetInst(0xBE, new BoxInst<ValueType::intType>("in2o"));
    convContainerInst->SetInst(0xBF, new BoxInst<ValueType::uintType>("ui2o"));
    convContainerInst->SetInst(0xC0, new BoxInst<ValueType::longType>("lo2o"));
    convContainerInst->SetInst(0xC1, new BoxInst<ValueType::ulongType>("ul2o"));
    convContainerInst->SetInst(0xC2, new BoxInst<ValueType::floatType>("fl2o"));
    convContainerInst->SetInst(0xC3, new BoxInst<ValueType::doubleType>("do2o"));
    convContainerInst->SetInst(0xC4, new BoxInst<ValueType::charType>("ch2o"));
    convContainerInst->SetInst(0xC5, new BoxInst<ValueType::boolType>("bo2o"));

//  unboxing:
    convContainerInst->SetInst(0xC6, new UnboxInst<ValueType::sbyteType>("o2sb"));
    convContainerInst->SetInst(0xC7, new UnboxInst<ValueType::byteType>("o2by"));
    convContainerInst->SetInst(0xC8, new UnboxInst<ValueType::shortType>("o2sh"));
    convContainerInst->SetInst(0xC9, new UnboxInst<ValueType::ushortType>("o2us"));
    convContainerInst->SetInst(0xCA, new UnboxInst<ValueType::intType>("o2in"));
    convContainerInst->SetInst(0xCB, new UnboxInst<ValueType::uintType>("o2ui"));
    convContainerInst->SetInst(0xCC, new UnboxInst<ValueType::longType>("o2lo"));
    convContainerInst->SetInst(0xCD, new UnboxInst<ValueType::ulongType>("o2ul"));
    convContainerInst->SetInst(0xCE, new UnboxInst<ValueType::floatType>("o2fl"));
    convContainerInst->SetInst(0xCF, new UnboxInst<ValueType::doubleType>("o2do"));
    convContainerInst->SetInst(0xD0, new UnboxInst<ValueType::charType>("o2ch"));
    convContainerInst->SetInst(0xD1, new UnboxInst<ValueType::boolType>("o2bo"));
}

Machine::~Machine()
{
    try
    {
        Exit();
    }
    catch (...)
    {
    }
}

void Machine::Start()
{
    threads.clear();
    Function* mainFun = FunctionTable::Instance().GetMain();
    if (!mainFun)
    {
        throw std::runtime_error("no main function set");
    }
    //RunGarbageCollector();
    threads.push_back(std::unique_ptr<Thread>(new Thread(*this, *mainFun)));
}

void Machine::Run()
{
    Function* mainFun = FunctionTable::Instance().GetMain();
    if (!mainFun)
    {
        throw std::runtime_error("no main function set");
    }
    //RunGarbageCollector();
    threads.push_back(std::unique_ptr<Thread>(new Thread(*this, *mainFun)));
    MainThread().Run();
}

void Machine::AddInst(Instruction* inst)
{
    instructionMap[inst->Name()] = inst;
    if (!inst->GroupName().empty() && !inst->TypeName().empty())
    {
        InstructionTypeGroup& instructionTypeGroup = instructionTypeGroupMap[inst->GroupName()];
        instructionTypeGroup.AddInst(inst);
    }
}

std::unique_ptr<Instruction> Machine::CreateInst(const std::string& instName) const
{
    auto it = instructionMap.find(instName);
    if (it != instructionMap.cend())
    {
        Instruction* inst = it->second;
        std::unique_ptr<Instruction> copy(inst->Clone());
        copy->Clear();
        return copy;
    }
    throw std::runtime_error("instruction '" + instName + "' not found");
}


std::unique_ptr<Instruction> Machine::CreateInst(const std::string& instGroupName, const std::string& typeName) const
{
    auto it = instructionTypeGroupMap.find(instGroupName);
    if (it != instructionTypeGroupMap.cend())
    {
        const InstructionTypeGroup& instructionTypeGroup = it->second;
        return instructionTypeGroup.CreateInst(typeName);
    }
    throw std::runtime_error("instruction group '" + instGroupName + "' not found");
}

std::unique_ptr<Instruction> Machine::DecodeInst(Reader& reader)
{
    Instruction* inst = rootInst.Decode(reader);
    Instruction* clonedInst = inst->Clone();
    if (CallInst* callInst = dynamic_cast<CallInst*>(clonedInst))
    {
        reader.AddCallInst(callInst);
    }
    else if (TypeInstruction* typeInst = dynamic_cast<TypeInstruction*>(clonedInst))
    {
        reader.AddTypeInstruction(typeInst);
    }
    else if (SetClassDataInst* setClassDataInst = dynamic_cast<SetClassDataInst*>(clonedInst))
    {
        reader.AddSetClassDataInst(setClassDataInst);
    }
    return std::unique_ptr<Instruction>(clonedInst);
}

std::pair<ArenaId, MemPtr> Machine::AllocateMemory(Thread& thread, uint64_t blockSize)
{
    if (blockSize < defaultLargeObjectThresholdSize)
    {
        return std::make_pair(gen1Arena.Id(), gen1Arena.Allocate(thread, blockSize));
    }
    else
    {
        return std::make_pair(gen2Arena.Id(), gen2Arena.Allocate(thread, blockSize));
    }
}

void DoRunGarbageCollector(GarbageCollector* garbageCollector)
{
    try
    {
        garbageCollector->Run();
    }
    catch (...)
    {
        int y = 0;
        int x = 0;
    }
}

void Machine::RunGarbageCollector()
{
    garbageCollectorThread = std::thread(DoRunGarbageCollector, &garbageCollector);
}

int32_t Machine::GetNextFrameId()
{
    return nextFrameId++;
}

bool Machine::Exiting()
{
    return exiting.load();
}

void Machine::Exit()
{
    if (exited.load())
    {
        return;
    }
    exiting.store(true);
    if (garbageCollector.Started())
    {
        garbageCollector.RequestGarbageCollection();
        garbageCollectorThread.join();
    }
    exited.store(true);
}

} } // namespace cminor::machine
