// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Machine.hpp>
#include <cminor/machine/String.hpp>
#include <functional>

namespace cminor { namespace machine {

Machine::Machine() : rootInst(*this, "<root_instruction>", true)
{
    // no operation:
    rootInst.SetInst(0x00, new NopInst());

    //  arithmetic unary operators:
    //  ---------------------------

    //  unary plus:
    rootInst.SetInst(0x01, new UnaryOpInst<int8_t, std::identity<int8_t>>("uplussb", "uplus", "sbyte"));
    rootInst.SetInst(0x02, new UnaryOpInst<uint8_t, std::identity<uint8_t>>("uplusby", "uplus", "byte"));
    rootInst.SetInst(0x03, new UnaryOpInst<int16_t, std::identity<int16_t>>("uplussh", "uplus", "short"));
    rootInst.SetInst(0x04, new UnaryOpInst<uint16_t, std::identity<uint16_t>>("uplusus", "uplus", "ushort"));
    rootInst.SetInst(0x05, new UnaryOpInst<int32_t, std::identity<int32_t>>("uplusin", "uplus", "int"));
    rootInst.SetInst(0x06, new UnaryOpInst<uint32_t, std::identity<uint32_t>>("uplusui", "uplus", "uint"));
    rootInst.SetInst(0x07, new UnaryOpInst<int64_t, std::identity<int64_t>>("upluslo", "uplus", "long"));
    rootInst.SetInst(0x08, new UnaryOpInst<uint64_t, std::identity<uint64_t>>("uplusul", "uplus", "ulong"));
    rootInst.SetInst(0x09, new UnaryOpInst<float, std::identity<float>>("uplusfl", "uplus", "float"));
    rootInst.SetInst(0x0A, new UnaryOpInst<double, std::identity<double>>("uplusdo", "uplus", "double"));

    //  unary minus:
    rootInst.SetInst(0x0B, new UnaryOpInst<int8_t, std::negate<int8_t>>("negsb", "neg", "sbyte"));
    rootInst.SetInst(0x0C, new UnaryOpInst<uint8_t, std::negate<uint8_t>>("negby", "neg", "byte"));
    rootInst.SetInst(0x0D, new UnaryOpInst<int16_t, std::negate<int16_t>>("negsh", "neg", "short"));
    rootInst.SetInst(0x0E, new UnaryOpInst<uint16_t, std::negate<uint16_t>>("negus", "neg", "ushort"));
    rootInst.SetInst(0x0F, new UnaryOpInst<int32_t, std::negate<int32_t>>("negin", "neg", "int"));
    rootInst.SetInst(0x10, new UnaryOpInst<uint32_t, std::negate<uint32_t>>("negui", "neg", "uint"));
    rootInst.SetInst(0x11, new UnaryOpInst<int64_t, std::negate<int64_t>>("neglo", "neg", "long"));
    rootInst.SetInst(0x12, new UnaryOpInst<uint64_t, std::negate<uint64_t>>("negul", "neg", "ulong"));
    rootInst.SetInst(0x13, new UnaryOpInst<float, std::negate<float>>("negfl", "neg", "float"));
    rootInst.SetInst(0x14, new UnaryOpInst<double, std::negate<double>>("negdo", "neg", "double"));

    //  bitwise complement:
    rootInst.SetInst(0x15, new UnaryOpInst<int8_t, std::bit_not<int8_t>>("cplsb", "cpl", "sbyte"));
    rootInst.SetInst(0x16, new UnaryOpInst<uint8_t, std::bit_not<uint8_t>>("cplby", "cpl", "byte"));
    rootInst.SetInst(0x17, new UnaryOpInst<int16_t, std::bit_not<int16_t>>("cplsh", "cpl", "short"));
    rootInst.SetInst(0x18, new UnaryOpInst<uint16_t, std::bit_not<uint16_t>>("cplus", "cpl", "ushort"));
    rootInst.SetInst(0x19, new UnaryOpInst<int32_t, std::bit_not<int32_t>>("cplin", "cpl", "int"));
    rootInst.SetInst(0x1A, new UnaryOpInst<uint32_t, std::bit_not<uint32_t>>("cplui", "cpl", "uint"));
    rootInst.SetInst(0x1B, new UnaryOpInst<int64_t, std::bit_not<int64_t>>("cpllo", "cpl", "long"));
    rootInst.SetInst(0x1C, new UnaryOpInst<uint64_t, std::bit_not<uint64_t>>("cplul", "cpl", "ulong"));

    // arithmetic binary operators:
    // ----------------------------

    //  add:
    rootInst.SetInst(0x1D, new BinaryOpInst<int8_t, std::plus<int8_t>>("addsb", "add", "sbyte"));
    rootInst.SetInst(0x1E, new BinaryOpInst<uint8_t, std::plus<uint8_t>>("addby", "add", "byte"));
    rootInst.SetInst(0x1F, new BinaryOpInst<int16_t, std::plus<int16_t>>("addsh", "add", "short"));
    rootInst.SetInst(0x20, new BinaryOpInst<uint16_t, std::plus<uint16_t>>("addus", "add", "ushort"));
    rootInst.SetInst(0x21, new BinaryOpInst<int32_t, std::plus<int32_t>>("addin", "add", "int"));
    rootInst.SetInst(0x22, new BinaryOpInst<uint32_t, std::plus<uint32_t>>("addui", "add", "uint"));
    rootInst.SetInst(0x23, new BinaryOpInst<int64_t, std::plus<int64_t>>("addlo", "add", "long"));
    rootInst.SetInst(0x24, new BinaryOpInst<uint64_t, std::plus<uint64_t>>("addul", "add", "ulong"));
    rootInst.SetInst(0x25, new BinaryOpInst<float, std::plus<float>>("addfl", "add", "float"));
    rootInst.SetInst(0x26, new BinaryOpInst<double, std::plus<double>>("adddo", "add", "double"));

    //  subtract:
    rootInst.SetInst(0x27, new BinaryOpInst<int8_t, std::minus<int8_t>>("subsb", "sub", "sbyte"));
    rootInst.SetInst(0x28, new BinaryOpInst<uint8_t, std::minus<uint8_t>>("subby", "sub", "byte"));
    rootInst.SetInst(0x29, new BinaryOpInst<int16_t, std::minus<int16_t>>("subsh", "sub", "short"));
    rootInst.SetInst(0x2A, new BinaryOpInst<uint16_t, std::minus<uint16_t>>("subus", "sub", "ushort"));
    rootInst.SetInst(0x2B, new BinaryOpInst<int32_t, std::minus<int32_t>>("subin", "sub", "int"));
    rootInst.SetInst(0x2C, new BinaryOpInst<uint32_t, std::minus<uint32_t>>("subui", "sub", "uint"));
    rootInst.SetInst(0x2D, new BinaryOpInst<int64_t, std::minus<int64_t>>("sublo", "sub", "long"));
    rootInst.SetInst(0x2E, new BinaryOpInst<uint64_t, std::minus<uint64_t>>("subul", "sub", "ulong"));
    rootInst.SetInst(0x2F, new BinaryOpInst<float, std::minus<float>>("subfl", "sub", "float"));
    rootInst.SetInst(0x30, new BinaryOpInst<double, std::minus<double>>("subdo", "sub", "double"));

    //  multiply:
    rootInst.SetInst(0x31, new BinaryOpInst<int8_t, std::multiplies<int8_t>>("mulsb", "mul", "sbyte"));
    rootInst.SetInst(0x32, new BinaryOpInst<uint8_t, std::multiplies<uint8_t>>("mulby", "mul", "byte"));
    rootInst.SetInst(0x33, new BinaryOpInst<int16_t, std::multiplies<int16_t>>("mulsh", "mul", "short"));
    rootInst.SetInst(0x34, new BinaryOpInst<uint16_t, std::multiplies<uint16_t>>("mulus", "mul", "ushort"));
    rootInst.SetInst(0x35, new BinaryOpInst<int32_t, std::multiplies<int32_t>>("mulin", "mul", "int"));
    rootInst.SetInst(0x36, new BinaryOpInst<uint32_t, std::multiplies<uint32_t>>("mului", "mul", "uint"));
    rootInst.SetInst(0x37, new BinaryOpInst<int64_t, std::multiplies<int64_t>>("mullo", "mul", "long"));
    rootInst.SetInst(0x38, new BinaryOpInst<uint64_t, std::multiplies<uint64_t>>("mulul", "mul", "ulong"));
    rootInst.SetInst(0x39, new BinaryOpInst<float, std::multiplies<float>>("mulfl", "mul", "float"));
    rootInst.SetInst(0x3A, new BinaryOpInst<double, std::multiplies<double>>("muldo", "mul", "double"));

    //  divide:
    rootInst.SetInst(0x3B, new BinaryOpInst<int8_t, std::divides<int8_t>>("divsb", "div", "sbyte"));
    rootInst.SetInst(0x3C, new BinaryOpInst<uint8_t, std::divides<uint8_t>>("divby", "div", "byte"));
    rootInst.SetInst(0x3D, new BinaryOpInst<int16_t, std::divides<int16_t>>("divsh", "div", "short"));
    rootInst.SetInst(0x3E, new BinaryOpInst<uint16_t, std::divides<uint16_t>>("divus", "div", "ushort"));
    rootInst.SetInst(0x3F, new BinaryOpInst<int32_t, std::divides<int32_t>>("divin", "div", "int"));
    rootInst.SetInst(0x40, new BinaryOpInst<uint32_t, std::divides<uint32_t>>("divui", "div", "uint"));
    rootInst.SetInst(0x41, new BinaryOpInst<int64_t, std::divides<int64_t>>("divlo", "div", "long"));
    rootInst.SetInst(0x42, new BinaryOpInst<uint64_t, std::divides<uint64_t>>("divul", "div", "ulong"));
    rootInst.SetInst(0x43, new BinaryOpInst<float, std::divides<float>>("divfl", "div", "float"));
    rootInst.SetInst(0x44, new BinaryOpInst<double, std::divides<double>>("divdo", "div", "double"));

    //  remainder:
    rootInst.SetInst(0x45, new BinaryOpInst<int8_t, std::modulus<int8_t>>("remsb", "rem", "sbyte"));
    rootInst.SetInst(0x46, new BinaryOpInst<uint8_t, std::modulus<uint8_t>>("remby", "rem", "byte"));
    rootInst.SetInst(0x47, new BinaryOpInst<int16_t, std::modulus<int16_t>>("remsh", "rem", "short"));
    rootInst.SetInst(0x48, new BinaryOpInst<uint16_t, std::modulus<uint16_t>>("remus", "rem", "ushort"));
    rootInst.SetInst(0x49, new BinaryOpInst<int32_t, std::modulus<int32_t>>("remin", "rem", "int"));
    rootInst.SetInst(0x4A, new BinaryOpInst<uint32_t, std::modulus<uint32_t>>("remui", "rem", "uint"));
    rootInst.SetInst(0x4B, new BinaryOpInst<int64_t, std::modulus<int64_t>>("remlo", "rem", "long"));
    rootInst.SetInst(0x4C, new BinaryOpInst<uint64_t, std::modulus<uint64_t>>("remul", "rem", "ulong"));

    //  bitwise and:
    rootInst.SetInst(0x4D, new BinaryOpInst<int8_t, std::bit_and<int8_t>>("andsb", "and", "sbyte"));
    rootInst.SetInst(0x4E, new BinaryOpInst<uint8_t, std::bit_and<uint8_t>>("andby", "and", "byte"));
    rootInst.SetInst(0x4F, new BinaryOpInst<int16_t, std::bit_and<int16_t>>("andsh", "and", "short"));
    rootInst.SetInst(0x50, new BinaryOpInst<uint16_t, std::bit_and<uint16_t>>("andus", "and", "ushort"));
    rootInst.SetInst(0x51, new BinaryOpInst<int32_t, std::bit_and<int32_t>>("andin", "and", "int"));
    rootInst.SetInst(0x52, new BinaryOpInst<uint32_t, std::bit_and<uint32_t>>("andui", "and", "uint"));
    rootInst.SetInst(0x53, new BinaryOpInst<int64_t, std::bit_and<int64_t>>("andlo", "and", "long"));
    rootInst.SetInst(0x54, new BinaryOpInst<uint64_t, std::bit_and<uint64_t>>("andul", "and", "ulong"));

    //  bitwise or:
    rootInst.SetInst(0x55, new BinaryOpInst<int8_t, std::bit_or<int8_t>>("orsb", "or", "sbyte"));
    rootInst.SetInst(0x56, new BinaryOpInst<uint8_t, std::bit_or<uint8_t>>("orby", "or", "byte"));
    rootInst.SetInst(0x57, new BinaryOpInst<int16_t, std::bit_or<int16_t>>("orsh", "or", "short"));
    rootInst.SetInst(0x58, new BinaryOpInst<uint16_t, std::bit_or<uint16_t>>("orus", "or", "ushort"));
    rootInst.SetInst(0x59, new BinaryOpInst<int32_t, std::bit_or<int32_t>>("orin", "or", "int"));
    rootInst.SetInst(0x5A, new BinaryOpInst<uint32_t, std::bit_or<uint32_t>>("orui", "or", "uint"));
    rootInst.SetInst(0x5B, new BinaryOpInst<int64_t, std::bit_or<int64_t>>("orlo", "or", "long"));
    rootInst.SetInst(0x5C, new BinaryOpInst<uint64_t, std::bit_or<uint64_t>>("orul", "or", "ulong"));

    //  bitwise xor:
    rootInst.SetInst(0x5D, new BinaryOpInst<int8_t, std::bit_xor<int8_t>>("xorsb", "xor", "sbyte"));
    rootInst.SetInst(0x5E, new BinaryOpInst<uint8_t, std::bit_xor<uint8_t>>("xorby", "xor", "byte"));
    rootInst.SetInst(0x5F, new BinaryOpInst<int16_t, std::bit_xor<int16_t>>("xorsh", "xor", "short"));
    rootInst.SetInst(0x60, new BinaryOpInst<uint16_t, std::bit_xor<uint16_t>>("xorus", "xor", "ushort"));
    rootInst.SetInst(0x61, new BinaryOpInst<int32_t, std::bit_xor<int32_t>>("xorin", "xor", "int"));
    rootInst.SetInst(0x62, new BinaryOpInst<uint32_t, std::bit_xor<uint32_t>>("xorui", "xor", "uint"));
    rootInst.SetInst(0x63, new BinaryOpInst<int64_t, std::bit_xor<int64_t>>("xorlo", "xor", "long"));
    rootInst.SetInst(0x64, new BinaryOpInst<uint64_t, std::bit_xor<uint64_t>>("xorul", "xor", "ulong"));

    //  shift left:
    rootInst.SetInst(0x65, new BinaryOpInst<int8_t, ShiftLeft<int8_t>>("shlsb", "shl", "sbyte"));
    rootInst.SetInst(0x66, new BinaryOpInst<uint8_t, ShiftLeft<uint8_t>>("shlby", "shl", "byte"));
    rootInst.SetInst(0x67, new BinaryOpInst<int16_t, ShiftLeft<int16_t>>("shlsh", "shl", "short"));
    rootInst.SetInst(0x68, new BinaryOpInst<uint16_t, ShiftLeft<uint16_t>>("shlus", "shl", "ushort"));
    rootInst.SetInst(0x69, new BinaryOpInst<int32_t, ShiftLeft<int32_t>>("shlin", "shl", "int"));
    rootInst.SetInst(0x6A, new BinaryOpInst<uint32_t, ShiftLeft<uint32_t>>("shlui", "shl", "uint"));
    rootInst.SetInst(0x6B, new BinaryOpInst<int64_t, ShiftLeft<int64_t>>("shllo", "shl", "long"));
    rootInst.SetInst(0x6C, new BinaryOpInst<uint64_t, ShiftLeft<uint64_t>>("shlul", "shl", "ulong"));

    //  shift right:
    rootInst.SetInst(0x6D, new BinaryOpInst<int8_t, ShiftRight<int8_t>>("shrsb", "shr", "sbyte"));
    rootInst.SetInst(0x6E, new BinaryOpInst<uint8_t, ShiftRight<uint8_t>>("shrby", "shr", "byte"));
    rootInst.SetInst(0x6F, new BinaryOpInst<int16_t, ShiftRight<int16_t>>("shrsh", "shr", "short"));
    rootInst.SetInst(0x70, new BinaryOpInst<uint16_t, ShiftRight<uint16_t>>("shrus", "shr", "ushort"));
    rootInst.SetInst(0x71, new BinaryOpInst<int32_t, ShiftRight<int32_t>>("shrin", "shr", "int"));
    rootInst.SetInst(0x72, new BinaryOpInst<uint32_t, ShiftRight<uint32_t>>("shrui", "shr", "uint"));
    rootInst.SetInst(0x73, new BinaryOpInst<int64_t, ShiftRight<int64_t>>("shrlo", "shr", "long"));
    rootInst.SetInst(0x74, new BinaryOpInst<uint64_t, ShiftRight<uint64_t>>("shrul", "shr", "ulong"));

    // comparisons:
    // ------------

    //  equal:
    rootInst.SetInst(0x75, new BinaryPredInst<int8_t, std::equal_to<int8_t>>("equalsb", "equal", "sbyte"));
    rootInst.SetInst(0x76, new BinaryPredInst<uint8_t, std::equal_to<uint8_t>>("equalby", "equal", "byte"));
    rootInst.SetInst(0x77, new BinaryPredInst<int16_t, std::equal_to<int16_t>>("equalsh", "equal", "short"));
    rootInst.SetInst(0x78, new BinaryPredInst<uint16_t, std::equal_to<uint16_t>>("equalus", "equal", "ushort"));
    rootInst.SetInst(0x79, new BinaryPredInst<int32_t, std::equal_to<int32_t>>("equalin", "equal", "int"));
    rootInst.SetInst(0x7A, new BinaryPredInst<uint32_t, std::equal_to<uint32_t>>("equalui", "equal", "uint"));
    rootInst.SetInst(0x7B, new BinaryPredInst<int64_t, std::equal_to<int64_t>>("equallo", "equal", "long"));
    rootInst.SetInst(0x7C, new BinaryPredInst<uint64_t, std::equal_to<uint64_t>>("equalul", "equal", "ulong"));
    rootInst.SetInst(0x7D, new BinaryPredInst<float, std::equal_to<float>>("equalfl", "equal", "float"));
    rootInst.SetInst(0x7E, new BinaryPredInst<double, std::equal_to<double>>("equaldo", "equal", "double"));
    rootInst.SetInst(0x7F, new BinaryPredInst<char32_t, std::equal_to<char32_t>>("equalch", "equal", "char"));
    rootInst.SetInst(0x80, new BinaryPredInst<bool, std::equal_to<bool>>("equalbo", "equal", "bool"));
    rootInst.SetInst(0x81, new StringEqualInst());

    //  less:
    rootInst.SetInst(0x83, new BinaryPredInst<int8_t, std::less<int8_t>>("lesssb", "less", "sbyte"));
    rootInst.SetInst(0x84, new BinaryPredInst<uint8_t, std::less<uint8_t>>("lessby", "less", "byte"));
    rootInst.SetInst(0x85, new BinaryPredInst<int16_t, std::less<int16_t>>("lesssh", "less", "short"));
    rootInst.SetInst(0x86, new BinaryPredInst<uint16_t, std::less<uint16_t>>("lessus", "less", "ushort"));
    rootInst.SetInst(0x87, new BinaryPredInst<int32_t, std::less<int32_t>>("lessin", "less", "int"));
    rootInst.SetInst(0x88, new BinaryPredInst<uint32_t, std::less<uint32_t>>("lessui", "less", "uint"));
    rootInst.SetInst(0x89, new BinaryPredInst<int64_t, std::less<int64_t>>("lesslo", "less", "long"));
    rootInst.SetInst(0x8A, new BinaryPredInst<uint64_t, std::less<uint64_t>>("lessul", "less", "ulong"));
    rootInst.SetInst(0x8B, new BinaryPredInst<float, std::less<float>>("lessfl", "less", "float"));
    rootInst.SetInst(0x8C, new BinaryPredInst<double, std::less<double>>("lessdo", "less", "double"));
    rootInst.SetInst(0x8D, new BinaryPredInst<char32_t, std::less<char32_t>>("lessch", "less", "char"));
    rootInst.SetInst(0x8E, new BinaryPredInst<bool, std::less<bool>>("lessbo", "less", "bool"));
    rootInst.SetInst(0x8F, new StringLessInst());

    //  logical operations:
    //  -------------------
    rootInst.SetInst(0x90, new LogicalNotInst());

    // todo 91 - FE:

    //  load & store:
    //  -------------

    rootInst.SetInst(0x91, new LoadLocalInst());
    rootInst.SetInst(0x92, new StoreLocalInst());

    //  conversion group instruction:
    //  -----------------------------
    ContainerInst* convContainerInst = new ContainerInst(*this, "<conversion_container_instruction>", false);
    rootInst.SetInst(0xFF, convContainerInst);

    //  conversions (prefixed by FF):
    //  -----------------------------

    // from sbyte:
    convContainerInst->SetInst(0x00, new ConversionInst<int8_t, uint8_t>("sb2by"));
    convContainerInst->SetInst(0x01, new ConversionInst<int8_t, int16_t>("sb2sh"));
    convContainerInst->SetInst(0x02, new ConversionInst<int8_t, uint16_t>("sb2us"));
    convContainerInst->SetInst(0x03, new ConversionInst<int8_t, int32_t>("sb2in"));
    convContainerInst->SetInst(0x04, new ConversionInst<int8_t, uint32_t>("sb2ui"));
    convContainerInst->SetInst(0x05, new ConversionInst<int8_t, int64_t>("sb2lo"));
    convContainerInst->SetInst(0x06, new ConversionInst<int8_t, uint64_t>("sb2ul"));
    convContainerInst->SetInst(0x07, new ConversionInst<int8_t, float>("sb2fl"));
    convContainerInst->SetInst(0x08, new ConversionInst<int8_t, double>("sb2do"));
    convContainerInst->SetInst(0x0B, new ConversionInst<int8_t, char32_t>("sb2ch"));
    convContainerInst->SetInst(0x0C, new ConversionInst<int8_t, bool>("sb2bo"));

    // from byte:
    convContainerInst->SetInst(0x0D, new ConversionInst<uint8_t, int8_t>("by2sb"));
    convContainerInst->SetInst(0x0E, new ConversionInst<uint8_t, int16_t>("by2sh"));
    convContainerInst->SetInst(0x0F, new ConversionInst<uint8_t, uint16_t>("by2us"));
    convContainerInst->SetInst(0x10, new ConversionInst<uint8_t, int32_t>("by2in"));
    convContainerInst->SetInst(0x11, new ConversionInst<uint8_t, uint32_t>("by2ui"));
    convContainerInst->SetInst(0x12, new ConversionInst<uint8_t, int64_t>("by2lo"));
    convContainerInst->SetInst(0x13, new ConversionInst<uint8_t, uint64_t>("by2ul"));
    convContainerInst->SetInst(0x14, new ConversionInst<uint8_t, float>("by2fl"));
    convContainerInst->SetInst(0x15, new ConversionInst<uint8_t, double>("by2do"));
    convContainerInst->SetInst(0x18, new ConversionInst<uint8_t, char32_t>("by2ch"));
    convContainerInst->SetInst(0x19, new ConversionInst<uint8_t, bool>("by2bo"));

    // from short:
    convContainerInst->SetInst(0x1A, new ConversionInst<int16_t, int8_t>("sh2sb"));
    convContainerInst->SetInst(0x1B, new ConversionInst<int16_t, uint8_t>("sh2by"));
    convContainerInst->SetInst(0x1C, new ConversionInst<int16_t, uint16_t>("sh2us"));
    convContainerInst->SetInst(0x1D, new ConversionInst<int16_t, int32_t>("sh2in"));
    convContainerInst->SetInst(0x1E, new ConversionInst<int16_t, uint32_t>("sh2ui"));
    convContainerInst->SetInst(0x1F, new ConversionInst<int16_t, int64_t>("sh2lo"));
    convContainerInst->SetInst(0x20, new ConversionInst<int16_t, uint64_t>("sh2ul"));
    convContainerInst->SetInst(0x21, new ConversionInst<int16_t, float>("sh2fl"));
    convContainerInst->SetInst(0x22, new ConversionInst<int16_t, double>("sh2do"));
    convContainerInst->SetInst(0x25, new ConversionInst<int16_t, char32_t>("sh2ch"));
    convContainerInst->SetInst(0x26, new ConversionInst<int16_t, bool>("sh2bo"));

    // from ushort:
    convContainerInst->SetInst(0x27, new ConversionInst<uint16_t, int8_t>("us2sb"));
    convContainerInst->SetInst(0x28, new ConversionInst<uint16_t, uint8_t>("us2by"));
    convContainerInst->SetInst(0x29, new ConversionInst<uint16_t, int16_t>("us2sh"));
    convContainerInst->SetInst(0x2A, new ConversionInst<uint16_t, int32_t>("us2in"));
    convContainerInst->SetInst(0x2B, new ConversionInst<uint16_t, uint32_t>("us2ui"));
    convContainerInst->SetInst(0x2C, new ConversionInst<uint16_t, int64_t>("us2lo"));
    convContainerInst->SetInst(0x2D, new ConversionInst<uint16_t, uint64_t>("us2ul"));
    convContainerInst->SetInst(0x2E, new ConversionInst<uint16_t, float>("us2fl"));
    convContainerInst->SetInst(0x2F, new ConversionInst<uint16_t, double>("us2do"));
    convContainerInst->SetInst(0x32, new ConversionInst<uint16_t, char32_t>("us2ch"));
    convContainerInst->SetInst(0x33, new ConversionInst<uint16_t, bool>("us2bo"));

    // from int:
    convContainerInst->SetInst(0x34, new ConversionInst<int32_t, int8_t>("in2sb"));
    convContainerInst->SetInst(0x35, new ConversionInst<int32_t, uint8_t>("in2by"));
    convContainerInst->SetInst(0x36, new ConversionInst<int32_t, int16_t>("in2sh"));
    convContainerInst->SetInst(0x37, new ConversionInst<int32_t, uint16_t>("in2us"));
    convContainerInst->SetInst(0x38, new ConversionInst<int32_t, uint32_t>("in2ui"));
    convContainerInst->SetInst(0x39, new ConversionInst<int32_t, int64_t>("in2lo"));
    convContainerInst->SetInst(0x3A, new ConversionInst<int32_t, uint64_t>("in2ul"));
    convContainerInst->SetInst(0x3B, new ConversionInst<int32_t, float>("in2fl"));
    convContainerInst->SetInst(0x3C, new ConversionInst<int32_t, double>("in2do"));
    convContainerInst->SetInst(0x3F, new ConversionInst<int32_t, char32_t>("in2ch"));
    convContainerInst->SetInst(0x40, new ConversionInst<int32_t, bool>("in2bo"));

    // from uint:
    convContainerInst->SetInst(0x41, new ConversionInst<uint32_t, int8_t>("ui2sb"));
    convContainerInst->SetInst(0x42, new ConversionInst<uint32_t, uint8_t>("ui2by"));
    convContainerInst->SetInst(0x43, new ConversionInst<uint32_t, int16_t>("ui2sh"));
    convContainerInst->SetInst(0x44, new ConversionInst<uint32_t, uint16_t>("ui2us"));
    convContainerInst->SetInst(0x45, new ConversionInst<uint32_t, int32_t>("ui2in"));
    convContainerInst->SetInst(0x46, new ConversionInst<uint32_t, int64_t>("ui2lo"));
    convContainerInst->SetInst(0x47, new ConversionInst<uint32_t, uint64_t>("ui2ul"));
    convContainerInst->SetInst(0x48, new ConversionInst<uint32_t, float>("ui2fl"));
    convContainerInst->SetInst(0x49, new ConversionInst<uint32_t, double>("ui2do"));
    convContainerInst->SetInst(0x4C, new ConversionInst<uint32_t, char32_t>("ui2ch"));
    convContainerInst->SetInst(0x4D, new ConversionInst<uint32_t, bool>("ui2bo"));

    // from long:
    convContainerInst->SetInst(0x4E, new ConversionInst<int64_t, int8_t>("lo2sb"));
    convContainerInst->SetInst(0x4F, new ConversionInst<int64_t, uint8_t>("lo2by"));
    convContainerInst->SetInst(0x50, new ConversionInst<int64_t, int16_t>("lo2sh"));
    convContainerInst->SetInst(0x51, new ConversionInst<int64_t, uint16_t>("lo2us"));
    convContainerInst->SetInst(0x52, new ConversionInst<int64_t, int32_t>("lo2in"));
    convContainerInst->SetInst(0x53, new ConversionInst<int64_t, uint32_t>("lo2ui"));
    convContainerInst->SetInst(0x54, new ConversionInst<int64_t, uint64_t>("lo2ul"));
    convContainerInst->SetInst(0x55, new ConversionInst<int64_t, float>("lo2fl"));
    convContainerInst->SetInst(0x56, new ConversionInst<int64_t, double>("lo2do"));
    convContainerInst->SetInst(0x59, new ConversionInst<int64_t, char32_t>("lo2ch"));
    convContainerInst->SetInst(0x5A, new ConversionInst<int64_t, bool>("lo2bo"));

    // from ulong:
    convContainerInst->SetInst(0x5B, new ConversionInst<uint64_t, int8_t>("ul2sb"));
    convContainerInst->SetInst(0x5C, new ConversionInst<uint64_t, uint8_t>("ul2by"));
    convContainerInst->SetInst(0x5D, new ConversionInst<uint64_t, int16_t>("ul2sh"));
    convContainerInst->SetInst(0x5E, new ConversionInst<uint64_t, uint16_t>("ul2us"));
    convContainerInst->SetInst(0x5F, new ConversionInst<uint64_t, int32_t>("ul2in"));
    convContainerInst->SetInst(0x60, new ConversionInst<uint64_t, uint32_t>("ul2ui"));
    convContainerInst->SetInst(0x61, new ConversionInst<uint64_t, int64_t>("ul2lo"));
    convContainerInst->SetInst(0x62, new ConversionInst<uint64_t, float>("ul2fl"));
    convContainerInst->SetInst(0x63, new ConversionInst<uint64_t, double>("ul2do"));
    convContainerInst->SetInst(0x66, new ConversionInst<uint64_t, char32_t>("ul2ch"));
    convContainerInst->SetInst(0x67, new ConversionInst<uint64_t, bool>("ul2bo"));

    // from float:
    convContainerInst->SetInst(0x68, new ConversionInst<float, int8_t>("fl2sb"));
    convContainerInst->SetInst(0x69, new ConversionInst<float, uint8_t>("fl2by"));
    convContainerInst->SetInst(0x6A, new ConversionInst<float, int16_t>("fl2sh"));
    convContainerInst->SetInst(0x6B, new ConversionInst<float, uint16_t>("fl2us"));
    convContainerInst->SetInst(0x6C, new ConversionInst<float, int32_t>("fl2in"));
    convContainerInst->SetInst(0x6D, new ConversionInst<float, uint32_t>("fl2ui"));
    convContainerInst->SetInst(0x6E, new ConversionInst<float, int64_t>("fl2lo"));
    convContainerInst->SetInst(0x6F, new ConversionInst<float, uint64_t>("fl2ul"));
    convContainerInst->SetInst(0x70, new ConversionInst<float, double>("fl2do"));
    convContainerInst->SetInst(0x73, new ConversionInst<float, char32_t>("fl2ch"));
    convContainerInst->SetInst(0x74, new ConversionInst<float, bool>("fl2bo"));

    // from double:
    convContainerInst->SetInst(0x75, new ConversionInst<double, int8_t>("do2sb"));
    convContainerInst->SetInst(0x76, new ConversionInst<double, uint8_t>("do2by"));
    convContainerInst->SetInst(0x77, new ConversionInst<double, int16_t>("do2sh"));
    convContainerInst->SetInst(0x78, new ConversionInst<double, uint16_t>("do2us"));
    convContainerInst->SetInst(0x79, new ConversionInst<double, int32_t>("do2in"));
    convContainerInst->SetInst(0x7A, new ConversionInst<double, uint32_t>("do2ui"));
    convContainerInst->SetInst(0x7B, new ConversionInst<double, int64_t>("do2lo"));
    convContainerInst->SetInst(0x7C, new ConversionInst<double, uint64_t>("do2ul"));
    convContainerInst->SetInst(0x7D, new ConversionInst<double, float>("do2fl"));
    convContainerInst->SetInst(0x80, new ConversionInst<double, char32_t>("do2ch"));
    convContainerInst->SetInst(0x81, new ConversionInst<double, bool>("do2bo"));

    // from char:
    convContainerInst->SetInst(0xA2, new ConversionInst<char32_t, int8_t>("ch2sb"));
    convContainerInst->SetInst(0xA3, new ConversionInst<char32_t, uint8_t>("ch2by"));
    convContainerInst->SetInst(0xA4, new ConversionInst<char32_t, int16_t>("ch2sh"));
    convContainerInst->SetInst(0xA5, new ConversionInst<char32_t, uint16_t>("ch2us"));
    convContainerInst->SetInst(0xA6, new ConversionInst<char32_t, int32_t>("ch2in"));
    convContainerInst->SetInst(0xA7, new ConversionInst<char32_t, uint32_t>("ch2ui"));
    convContainerInst->SetInst(0xA8, new ConversionInst<char32_t, int64_t>("ch2lo"));
    convContainerInst->SetInst(0xA9, new ConversionInst<char32_t, uint64_t>("ch2ul"));
    convContainerInst->SetInst(0xAA, new ConversionInst<char32_t, float>("ch2fl"));
    convContainerInst->SetInst(0xAB, new ConversionInst<char32_t, double>("ch2fl"));
    convContainerInst->SetInst(0xAE, new ConversionInst<char32_t, bool>("ch2bo"));
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
        return std::unique_ptr<Instruction>(inst->Clone());
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
    return std::unique_ptr<Instruction>(inst->Clone());
}

std::unique_ptr<Assembly> Machine::ReadAssemblyFile(const std::string& fileName_)
{
    std::unique_ptr<Assembly> assembly(new Assembly(fileName_));
    Reader reader(*this, fileName_);
    assembly->Read(reader);
    return assembly;
}

} } // namespace cminor::machine
