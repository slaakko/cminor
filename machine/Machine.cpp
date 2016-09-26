// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Machine.hpp>
#include <functional>

namespace cminor { namespace machine {

Machine::Machine() : rootInst(*this, "<root_instruction>", true)
{
    // no operation:
    rootInst.SetInst(0x00, new NopInst());

    //  arithmetic unary operators:
    //  ---------------------------

    //  unary plus:
    rootInst.SetInst(0x01, new UnaryOpInst<int8_t, std::identity<int8_t>>("uplussb"));
    rootInst.SetInst(0x02, new UnaryOpInst<uint8_t, std::identity<uint8_t>>("uplusby"));
    rootInst.SetInst(0x03, new UnaryOpInst<int16_t, std::identity<int16_t>>("uplussh"));
    rootInst.SetInst(0x04, new UnaryOpInst<uint16_t, std::identity<uint16_t>>("uplusus"));
    rootInst.SetInst(0x05, new UnaryOpInst<int32_t, std::identity<int32_t>>("uplusin"));
    rootInst.SetInst(0x06, new UnaryOpInst<uint32_t, std::identity<uint32_t>>("uplusui"));
    rootInst.SetInst(0x07, new UnaryOpInst<int64_t, std::identity<int64_t>>("upluslo"));
    rootInst.SetInst(0x08, new UnaryOpInst<uint64_t, std::identity<uint64_t>>("uplusul"));
    rootInst.SetInst(0x09, new UnaryOpInst<float, std::identity<float>>("uplusfl"));
    rootInst.SetInst(0x0A, new UnaryOpInst<double, std::identity<double>>("uplusdo"));

    //  unary minus:
    rootInst.SetInst(0x0B, new UnaryOpInst<int8_t, std::negate<int8_t>>("negsb"));
    rootInst.SetInst(0x0C, new UnaryOpInst<uint8_t, std::negate<uint8_t>>("negby"));
    rootInst.SetInst(0x0D, new UnaryOpInst<int16_t, std::negate<int16_t>>("negsh"));
    rootInst.SetInst(0x0E, new UnaryOpInst<uint16_t, std::negate<uint16_t>>("negus"));
    rootInst.SetInst(0x0F, new UnaryOpInst<int32_t, std::negate<int32_t>>("negin"));
    rootInst.SetInst(0x10, new UnaryOpInst<uint32_t, std::negate<uint32_t>>("negui"));
    rootInst.SetInst(0x11, new UnaryOpInst<int64_t, std::negate<int64_t>>("neglo"));
    rootInst.SetInst(0x12, new UnaryOpInst<uint64_t, std::negate<uint64_t>>("negul"));
    rootInst.SetInst(0x13, new UnaryOpInst<float, std::negate<float>>("negfl"));
    rootInst.SetInst(0x14, new UnaryOpInst<double, std::negate<double>>("negdo"));

    //  bitwise complement:
    rootInst.SetInst(0x15, new UnaryOpInst<int8_t, std::bit_not<int8_t>>("cplsb"));
    rootInst.SetInst(0x16, new UnaryOpInst<uint8_t, std::bit_not<uint8_t>>("cplby"));
    rootInst.SetInst(0x17, new UnaryOpInst<int16_t, std::bit_not<int16_t>>("cplsh"));
    rootInst.SetInst(0x18, new UnaryOpInst<uint16_t, std::bit_not<uint16_t>>("cplus"));
    rootInst.SetInst(0x19, new UnaryOpInst<int32_t, std::bit_not<int32_t>>("cplin"));
    rootInst.SetInst(0x1A, new UnaryOpInst<uint32_t, std::bit_not<uint32_t>>("cplui"));
    rootInst.SetInst(0x1B, new UnaryOpInst<int64_t, std::bit_not<int64_t>>("cpllo"));
    rootInst.SetInst(0x1C, new UnaryOpInst<uint64_t, std::bit_not<uint64_t>>("cplul"));

    // arithmetic binary operators:
    // ----------------------------

    //  add:
    rootInst.SetInst(0x1D, new BinaryOpInst<int8_t, std::plus<int8_t>>("addsb"));
    rootInst.SetInst(0x1E, new BinaryOpInst<uint8_t, std::plus<uint8_t>>("addby"));
    rootInst.SetInst(0x1F, new BinaryOpInst<int16_t, std::plus<int16_t>>("addsh"));
    rootInst.SetInst(0x20, new BinaryOpInst<uint16_t, std::plus<uint16_t>>("addus"));
    rootInst.SetInst(0x21, new BinaryOpInst<int32_t, std::plus<int32_t>>("addin"));
    rootInst.SetInst(0x22, new BinaryOpInst<uint32_t, std::plus<uint32_t>>("addui"));
    rootInst.SetInst(0x23, new BinaryOpInst<int64_t, std::plus<int64_t>>("addlo"));
    rootInst.SetInst(0x24, new BinaryOpInst<uint64_t, std::plus<uint64_t>>("addul"));
    rootInst.SetInst(0x25, new BinaryOpInst<float, std::plus<float>>("addfl"));
    rootInst.SetInst(0x26, new BinaryOpInst<double, std::plus<double>>("adddo"));

    //  subtract:
    rootInst.SetInst(0x27, new BinaryOpInst<int8_t, std::minus<int8_t>>("subsb"));
    rootInst.SetInst(0x28, new BinaryOpInst<uint8_t, std::minus<uint8_t>>("subby"));
    rootInst.SetInst(0x29, new BinaryOpInst<int16_t, std::minus<int16_t>>("subsh"));
    rootInst.SetInst(0x2A, new BinaryOpInst<uint16_t, std::minus<uint16_t>>("subus"));
    rootInst.SetInst(0x2B, new BinaryOpInst<int32_t, std::minus<int32_t>>("subin"));
    rootInst.SetInst(0x2C, new BinaryOpInst<uint32_t, std::minus<uint32_t>>("subui"));
    rootInst.SetInst(0x2D, new BinaryOpInst<int64_t, std::minus<int64_t>>("sublo"));
    rootInst.SetInst(0x2E, new BinaryOpInst<uint64_t, std::minus<uint64_t>>("subul"));
    rootInst.SetInst(0x2F, new BinaryOpInst<float, std::minus<float>>("subfl"));
    rootInst.SetInst(0x30, new BinaryOpInst<double, std::minus<double>>("subdo"));

    //  multiply:
    rootInst.SetInst(0x31, new BinaryOpInst<int8_t, std::multiplies<int8_t>>("mulsb"));
    rootInst.SetInst(0x32, new BinaryOpInst<uint8_t, std::multiplies<uint8_t>>("mulby"));
    rootInst.SetInst(0x33, new BinaryOpInst<int16_t, std::multiplies<int16_t>>("mulsh"));
    rootInst.SetInst(0x34, new BinaryOpInst<uint16_t, std::multiplies<uint16_t>>("mulus"));
    rootInst.SetInst(0x35, new BinaryOpInst<int32_t, std::multiplies<int32_t>>("mulin"));
    rootInst.SetInst(0x36, new BinaryOpInst<uint32_t, std::multiplies<uint32_t>>("mului"));
    rootInst.SetInst(0x37, new BinaryOpInst<int64_t, std::multiplies<int64_t>>("mullo"));
    rootInst.SetInst(0x38, new BinaryOpInst<uint64_t, std::multiplies<uint64_t>>("mulul"));
    rootInst.SetInst(0x39, new BinaryOpInst<float, std::multiplies<float>>("mulfl"));
    rootInst.SetInst(0x3A, new BinaryOpInst<double, std::multiplies<double>>("muldo"));

    //  divide:
    rootInst.SetInst(0x3B, new BinaryOpInst<int8_t, std::divides<int8_t>>("divsb"));
    rootInst.SetInst(0x3C, new BinaryOpInst<uint8_t, std::divides<uint8_t>>("divby"));
    rootInst.SetInst(0x3D, new BinaryOpInst<int16_t, std::divides<int16_t>>("divsh"));
    rootInst.SetInst(0x3E, new BinaryOpInst<uint16_t, std::divides<uint16_t>>("divus"));
    rootInst.SetInst(0x3F, new BinaryOpInst<int32_t, std::divides<int32_t>>("divin"));
    rootInst.SetInst(0x40, new BinaryOpInst<uint32_t, std::divides<uint32_t>>("divui"));
    rootInst.SetInst(0x41, new BinaryOpInst<int64_t, std::divides<int64_t>>("divlo"));
    rootInst.SetInst(0x42, new BinaryOpInst<uint64_t, std::divides<uint64_t>>("divul"));
    rootInst.SetInst(0x43, new BinaryOpInst<float, std::divides<float>>("divfl"));
    rootInst.SetInst(0x44, new BinaryOpInst<double, std::divides<double>>("divdo"));

    //  remainder:
    rootInst.SetInst(0x45, new BinaryOpInst<int8_t, std::modulus<int8_t>>("remsb"));
    rootInst.SetInst(0x46, new BinaryOpInst<uint8_t, std::modulus<uint8_t>>("remby"));
    rootInst.SetInst(0x47, new BinaryOpInst<int16_t, std::modulus<int16_t>>("remsh"));
    rootInst.SetInst(0x48, new BinaryOpInst<uint16_t, std::modulus<uint16_t>>("remus"));
    rootInst.SetInst(0x49, new BinaryOpInst<int32_t, std::modulus<int32_t>>("remin"));
    rootInst.SetInst(0x4A, new BinaryOpInst<uint32_t, std::modulus<uint32_t>>("remui"));
    rootInst.SetInst(0x4B, new BinaryOpInst<int64_t, std::modulus<int64_t>>("remlo"));
    rootInst.SetInst(0x4C, new BinaryOpInst<uint64_t, std::modulus<uint64_t>>("remul"));

    //  bitwise and:
    rootInst.SetInst(0x4D, new BinaryOpInst<int8_t, std::bit_and<int8_t>>("andsb"));
    rootInst.SetInst(0x4E, new BinaryOpInst<uint8_t, std::bit_and<uint8_t>>("andby"));
    rootInst.SetInst(0x4F, new BinaryOpInst<int16_t, std::bit_and<int16_t>>("andsh"));
    rootInst.SetInst(0x50, new BinaryOpInst<uint16_t, std::bit_and<uint16_t>>("andus"));
    rootInst.SetInst(0x51, new BinaryOpInst<int32_t, std::bit_and<int32_t>>("andin"));
    rootInst.SetInst(0x52, new BinaryOpInst<uint32_t, std::bit_and<uint32_t>>("andui"));
    rootInst.SetInst(0x53, new BinaryOpInst<int64_t, std::bit_and<int64_t>>("andlo"));
    rootInst.SetInst(0x54, new BinaryOpInst<uint64_t, std::bit_and<uint64_t>>("andul"));

    //  bitwise or:
    rootInst.SetInst(0x55, new BinaryOpInst<int8_t, std::bit_or<int8_t>>("orsb"));
    rootInst.SetInst(0x56, new BinaryOpInst<uint8_t, std::bit_or<uint8_t>>("orby"));
    rootInst.SetInst(0x57, new BinaryOpInst<int16_t, std::bit_or<int16_t>>("orsh"));
    rootInst.SetInst(0x58, new BinaryOpInst<uint16_t, std::bit_or<uint16_t>>("orus"));
    rootInst.SetInst(0x59, new BinaryOpInst<int32_t, std::bit_or<int32_t>>("orin"));
    rootInst.SetInst(0x5A, new BinaryOpInst<uint32_t, std::bit_or<uint32_t>>("orui"));
    rootInst.SetInst(0x5B, new BinaryOpInst<int64_t, std::bit_or<int64_t>>("orlo"));
    rootInst.SetInst(0x5C, new BinaryOpInst<uint64_t, std::bit_or<uint64_t>>("orul"));

    //  bitwise xor:
    rootInst.SetInst(0x5D, new BinaryOpInst<int8_t, std::bit_xor<int8_t>>("xorsb"));
    rootInst.SetInst(0x5E, new BinaryOpInst<uint8_t, std::bit_xor<uint8_t>>("xorby"));
    rootInst.SetInst(0x5F, new BinaryOpInst<int16_t, std::bit_xor<int16_t>>("xorsh"));
    rootInst.SetInst(0x60, new BinaryOpInst<uint16_t, std::bit_xor<uint16_t>>("xorus"));
    rootInst.SetInst(0x61, new BinaryOpInst<int32_t, std::bit_xor<int32_t>>("xorin"));
    rootInst.SetInst(0x62, new BinaryOpInst<uint32_t, std::bit_xor<uint32_t>>("xorui"));
    rootInst.SetInst(0x63, new BinaryOpInst<int64_t, std::bit_xor<int64_t>>("xorlo"));
    rootInst.SetInst(0x64, new BinaryOpInst<uint64_t, std::bit_xor<uint64_t>>("xorul"));

    //  shift left:
    rootInst.SetInst(0x65, new BinaryOpInst<int8_t, ShiftLeft<int8_t>>("shlsb"));
    rootInst.SetInst(0x66, new BinaryOpInst<uint8_t, ShiftLeft<uint8_t>>("shlby"));
    rootInst.SetInst(0x67, new BinaryOpInst<int16_t, ShiftLeft<int16_t>>("shlsh"));
    rootInst.SetInst(0x68, new BinaryOpInst<uint16_t, ShiftLeft<uint16_t>>("shlus"));
    rootInst.SetInst(0x69, new BinaryOpInst<int32_t, ShiftLeft<int32_t>>("shlin"));
    rootInst.SetInst(0x6A, new BinaryOpInst<uint32_t, ShiftLeft<uint32_t>>("shlui"));
    rootInst.SetInst(0x6B, new BinaryOpInst<int64_t, ShiftLeft<int64_t>>("shllo"));
    rootInst.SetInst(0x6C, new BinaryOpInst<uint64_t, ShiftLeft<uint64_t>>("shlul"));

    //  shift right:
    rootInst.SetInst(0x6D, new BinaryOpInst<int8_t, ShiftRight<int8_t>>("shrsb"));
    rootInst.SetInst(0x6E, new BinaryOpInst<uint8_t, ShiftRight<uint8_t>>("shrby"));
    rootInst.SetInst(0x6F, new BinaryOpInst<int16_t, ShiftRight<int16_t>>("shrsh"));
    rootInst.SetInst(0x70, new BinaryOpInst<uint16_t, ShiftRight<uint16_t>>("shrus"));
    rootInst.SetInst(0x71, new BinaryOpInst<int32_t, ShiftRight<int32_t>>("shrin"));
    rootInst.SetInst(0x72, new BinaryOpInst<uint32_t, ShiftRight<uint32_t>>("shrui"));
    rootInst.SetInst(0x73, new BinaryOpInst<int64_t, ShiftRight<int64_t>>("shlro"));
    rootInst.SetInst(0x74, new BinaryOpInst<uint64_t, ShiftRight<uint64_t>>("shrul"));

    // comparisons:
    // ------------

    //  equal:
    rootInst.SetInst(0x75, new BinaryPredInst<int8_t, std::equal_to<int8_t>>("equalb"));
    rootInst.SetInst(0x76, new BinaryPredInst<uint8_t, std::equal_to<uint8_t>>("equalby"));
    rootInst.SetInst(0x77, new BinaryPredInst<int16_t, std::equal_to<int16_t>>("equalsh"));
    rootInst.SetInst(0x78, new BinaryPredInst<uint16_t, std::equal_to<uint16_t>>("equalus"));
    rootInst.SetInst(0x79, new BinaryPredInst<int32_t, std::equal_to<int32_t>>("equalin"));
    rootInst.SetInst(0x7A, new BinaryPredInst<uint32_t, std::equal_to<uint32_t>>("equalui"));
    rootInst.SetInst(0x7B, new BinaryPredInst<int64_t, std::equal_to<int64_t>>("equallo"));
    rootInst.SetInst(0x7C, new BinaryPredInst<uint64_t, std::equal_to<uint64_t>>("equalul"));
    rootInst.SetInst(0x7D, new BinaryPredInst<float, std::equal_to<float>>("equalfl"));
    rootInst.SetInst(0x7E, new BinaryPredInst<double, std::equal_to<double>>("equaldo"));
    rootInst.SetInst(0x7F, new BinaryPredInst<char, std::equal_to<char>>("equalch"));
    rootInst.SetInst(0x80, new BinaryPredInst<char16_t, std::equal_to<char16_t>>("equalwc"));
    rootInst.SetInst(0x81, new BinaryPredInst<char32_t, std::equal_to<char32_t>>("equaluc"));
    rootInst.SetInst(0x82, new BinaryPredInst<bool, std::equal_to<bool>>("equalbo"));

    //  less:
    rootInst.SetInst(0x83, new BinaryPredInst<int8_t, std::less<int8_t>>("lesssb"));
    rootInst.SetInst(0x84, new BinaryPredInst<uint8_t, std::less<uint8_t>>("lessby"));
    rootInst.SetInst(0x85, new BinaryPredInst<int16_t, std::less<int16_t>>("lesssh"));
    rootInst.SetInst(0x86, new BinaryPredInst<uint16_t, std::less<uint16_t>>("lessus"));
    rootInst.SetInst(0x87, new BinaryPredInst<int32_t, std::less<int32_t>>("lessin"));
    rootInst.SetInst(0x88, new BinaryPredInst<uint32_t, std::less<uint32_t>>("lessui"));
    rootInst.SetInst(0x89, new BinaryPredInst<int64_t, std::less<int64_t>>("lesslo"));
    rootInst.SetInst(0x8A, new BinaryPredInst<uint64_t, std::less<uint64_t>>("lessul"));
    rootInst.SetInst(0x8B, new BinaryPredInst<float, std::less<float>>("lessfl"));
    rootInst.SetInst(0x8C, new BinaryPredInst<double, std::less<double>>("lessdo"));
    rootInst.SetInst(0x8D, new BinaryPredInst<char, std::equal_to<char>>("lessch"));
    rootInst.SetInst(0x8E, new BinaryPredInst<char16_t, std::equal_to<char16_t>>("lesswc"));
    rootInst.SetInst(0x8F, new BinaryPredInst<char32_t, std::equal_to<char32_t>>("lessuc"));
    rootInst.SetInst(0x90, new BinaryPredInst<bool, std::equal_to<bool>>("lessbo"));

    //  logical operations:
    //  -------------------
    rootInst.SetInst(0x91, new LogicalNotInst());

    // todo 92 - FE:

    //  load & store:
    //  -------------

    rootInst.SetInst(0x92, new LoadLocalInst());
    rootInst.SetInst(0x93, new StoreLocalInst());

    //  conversion group instruction:
    //  -----------------------------
    GroupInst* convGroupInst = new GroupInst(*this, "<conversion_group_instruction>", false);
    rootInst.SetInst(0xFF, convGroupInst);

    //  conversions (prefixed by FF):
    //  -----------------------------

    // from sbyte:
    convGroupInst->SetInst(0x00, new ConversionInst<int8_t, uint8_t>("sb2by"));
    convGroupInst->SetInst(0x01, new ConversionInst<int8_t, int16_t>("sb2sh"));
    convGroupInst->SetInst(0x02, new ConversionInst<int8_t, uint16_t>("sb2us"));
    convGroupInst->SetInst(0x03, new ConversionInst<int8_t, int32_t>("sb2in"));
    convGroupInst->SetInst(0x04, new ConversionInst<int8_t, uint32_t>("sb2ui"));
    convGroupInst->SetInst(0x05, new ConversionInst<int8_t, int64_t>("sb2lo"));
    convGroupInst->SetInst(0x06, new ConversionInst<int8_t, uint64_t>("sb2ul"));
    convGroupInst->SetInst(0x07, new ConversionInst<int8_t, float>("sb2fl"));
    convGroupInst->SetInst(0x08, new ConversionInst<int8_t, double>("sb2do"));
    convGroupInst->SetInst(0x09, new ConversionInst<int8_t, char>("sb2ch"));
    convGroupInst->SetInst(0x0A, new ConversionInst<int8_t, char16_t>("sb2wc"));
    convGroupInst->SetInst(0x0B, new ConversionInst<int8_t, char32_t>("sb2uc"));
    convGroupInst->SetInst(0x0C, new ConversionInst<int8_t, bool>("sb2bo"));

    // from byte:
    convGroupInst->SetInst(0x0D, new ConversionInst<uint8_t, int8_t>("by2sb"));
    convGroupInst->SetInst(0x0E, new ConversionInst<uint8_t, int16_t>("by2sh"));
    convGroupInst->SetInst(0x0F, new ConversionInst<uint8_t, uint16_t>("by2us"));
    convGroupInst->SetInst(0x10, new ConversionInst<uint8_t, int32_t>("by2in"));
    convGroupInst->SetInst(0x11, new ConversionInst<uint8_t, uint32_t>("by2ui"));
    convGroupInst->SetInst(0x12, new ConversionInst<uint8_t, int64_t>("by2lo"));
    convGroupInst->SetInst(0x13, new ConversionInst<uint8_t, uint64_t>("by2ul"));
    convGroupInst->SetInst(0x14, new ConversionInst<uint8_t, float>("by2fl"));
    convGroupInst->SetInst(0x15, new ConversionInst<uint8_t, double>("by2do"));
    convGroupInst->SetInst(0x16, new ConversionInst<uint8_t, char>("by2ch"));
    convGroupInst->SetInst(0x17, new ConversionInst<uint8_t, char16_t>("by2wc"));
    convGroupInst->SetInst(0x18, new ConversionInst<uint8_t, char32_t>("by2uc"));
    convGroupInst->SetInst(0x19, new ConversionInst<uint8_t, bool>("by2bo"));

    // from short:
    convGroupInst->SetInst(0x1A, new ConversionInst<int16_t, int8_t>("sh2sb"));
    convGroupInst->SetInst(0x1B, new ConversionInst<int16_t, uint8_t>("sh2by"));
    convGroupInst->SetInst(0x1C, new ConversionInst<int16_t, uint16_t>("sh2us"));
    convGroupInst->SetInst(0x1D, new ConversionInst<int16_t, int32_t>("sh2in"));
    convGroupInst->SetInst(0x1E, new ConversionInst<int16_t, uint32_t>("sh2ui"));
    convGroupInst->SetInst(0x1F, new ConversionInst<int16_t, int64_t>("sh2lo"));
    convGroupInst->SetInst(0x20, new ConversionInst<int16_t, uint64_t>("sh2ul"));
    convGroupInst->SetInst(0x21, new ConversionInst<int16_t, float>("sh2fl"));
    convGroupInst->SetInst(0x22, new ConversionInst<int16_t, double>("sh2do"));
    convGroupInst->SetInst(0x23, new ConversionInst<int16_t, char>("sh2ch"));
    convGroupInst->SetInst(0x24, new ConversionInst<int16_t, char16_t>("sh2wc"));
    convGroupInst->SetInst(0x25, new ConversionInst<int16_t, char32_t>("sh2uc"));
    convGroupInst->SetInst(0x26, new ConversionInst<int16_t, bool>("sh2bo"));

    // from ushort:
    convGroupInst->SetInst(0x27, new ConversionInst<uint16_t, int8_t>("us2sb"));
    convGroupInst->SetInst(0x28, new ConversionInst<uint16_t, uint8_t>("us2by"));
    convGroupInst->SetInst(0x29, new ConversionInst<uint16_t, int16_t>("us2sh"));
    convGroupInst->SetInst(0x2A, new ConversionInst<uint16_t, int32_t>("us2in"));
    convGroupInst->SetInst(0x2B, new ConversionInst<uint16_t, uint32_t>("us2ui"));
    convGroupInst->SetInst(0x2C, new ConversionInst<uint16_t, int64_t>("us2lo"));
    convGroupInst->SetInst(0x2D, new ConversionInst<uint16_t, uint64_t>("us2ul"));
    convGroupInst->SetInst(0x2E, new ConversionInst<uint16_t, float>("us2fl"));
    convGroupInst->SetInst(0x2F, new ConversionInst<uint16_t, double>("us2do"));
    convGroupInst->SetInst(0x30, new ConversionInst<uint16_t, char>("us2ch"));
    convGroupInst->SetInst(0x31, new ConversionInst<uint16_t, char16_t>("us2wc"));
    convGroupInst->SetInst(0x32, new ConversionInst<uint16_t, char32_t>("us2uc"));
    convGroupInst->SetInst(0x33, new ConversionInst<uint16_t, bool>("us2bo"));

    // from int:
    convGroupInst->SetInst(0x34, new ConversionInst<int32_t, int8_t>("in2sb"));
    convGroupInst->SetInst(0x35, new ConversionInst<int32_t, uint8_t>("in2by"));
    convGroupInst->SetInst(0x36, new ConversionInst<int32_t, int16_t>("in2sh"));
    convGroupInst->SetInst(0x37, new ConversionInst<int32_t, uint16_t>("in2us"));
    convGroupInst->SetInst(0x38, new ConversionInst<int32_t, uint32_t>("in2ui"));
    convGroupInst->SetInst(0x39, new ConversionInst<int32_t, int64_t>("in2lo"));
    convGroupInst->SetInst(0x3A, new ConversionInst<int32_t, uint64_t>("in2ul"));
    convGroupInst->SetInst(0x3B, new ConversionInst<int32_t, float>("in2fl"));
    convGroupInst->SetInst(0x3C, new ConversionInst<int32_t, double>("in2do"));
    convGroupInst->SetInst(0x3D, new ConversionInst<int32_t, char>("in2ch"));
    convGroupInst->SetInst(0x3E, new ConversionInst<int32_t, char16_t>("in2wc"));
    convGroupInst->SetInst(0x3F, new ConversionInst<int32_t, char32_t>("in2uc"));
    convGroupInst->SetInst(0x40, new ConversionInst<int32_t, bool>("in2bo"));

    // from uint:
    convGroupInst->SetInst(0x41, new ConversionInst<uint32_t, int8_t>("ui2sb"));
    convGroupInst->SetInst(0x42, new ConversionInst<uint32_t, uint8_t>("ui2by"));
    convGroupInst->SetInst(0x43, new ConversionInst<uint32_t, int16_t>("ui2sh"));
    convGroupInst->SetInst(0x44, new ConversionInst<uint32_t, uint16_t>("ui2us"));
    convGroupInst->SetInst(0x45, new ConversionInst<uint32_t, int32_t>("ui2in"));
    convGroupInst->SetInst(0x46, new ConversionInst<uint32_t, int64_t>("ui2lo"));
    convGroupInst->SetInst(0x47, new ConversionInst<uint32_t, uint64_t>("ui2ul"));
    convGroupInst->SetInst(0x48, new ConversionInst<uint32_t, float>("ui2fl"));
    convGroupInst->SetInst(0x49, new ConversionInst<uint32_t, double>("ui2do"));
    convGroupInst->SetInst(0x4A, new ConversionInst<uint32_t, char>("ui2ch"));
    convGroupInst->SetInst(0x4B, new ConversionInst<uint32_t, char16_t>("ui2wc"));
    convGroupInst->SetInst(0x4C, new ConversionInst<uint32_t, char32_t>("ui2uc"));
    convGroupInst->SetInst(0x4D, new ConversionInst<uint32_t, bool>("ui2bo"));

    // from long:
    convGroupInst->SetInst(0x4E, new ConversionInst<int64_t, int8_t>("lo2sb"));
    convGroupInst->SetInst(0x4F, new ConversionInst<int64_t, uint8_t>("lo2by"));
    convGroupInst->SetInst(0x50, new ConversionInst<int64_t, int16_t>("lo2sh"));
    convGroupInst->SetInst(0x51, new ConversionInst<int64_t, uint16_t>("lo2us"));
    convGroupInst->SetInst(0x52, new ConversionInst<int64_t, int32_t>("lo2in"));
    convGroupInst->SetInst(0x53, new ConversionInst<int64_t, uint32_t>("lo2ui"));
    convGroupInst->SetInst(0x54, new ConversionInst<int64_t, uint64_t>("lo2ul"));
    convGroupInst->SetInst(0x55, new ConversionInst<int64_t, float>("lo2fl"));
    convGroupInst->SetInst(0x56, new ConversionInst<int64_t, double>("lo2do"));
    convGroupInst->SetInst(0x57, new ConversionInst<int64_t, char>("lo2ch"));
    convGroupInst->SetInst(0x58, new ConversionInst<int64_t, char16_t>("lo2wc"));
    convGroupInst->SetInst(0x59, new ConversionInst<int64_t, char32_t>("lo2uc"));
    convGroupInst->SetInst(0x5A, new ConversionInst<int64_t, bool>("lo2bo"));

    // from ulong:
    convGroupInst->SetInst(0x5B, new ConversionInst<uint64_t, int8_t>("ul2sb"));
    convGroupInst->SetInst(0x5C, new ConversionInst<uint64_t, uint8_t>("ul2by"));
    convGroupInst->SetInst(0x5D, new ConversionInst<uint64_t, int16_t>("ul2sh"));
    convGroupInst->SetInst(0x5E, new ConversionInst<uint64_t, uint16_t>("ul2us"));
    convGroupInst->SetInst(0x5F, new ConversionInst<uint64_t, int32_t>("ul2in"));
    convGroupInst->SetInst(0x60, new ConversionInst<uint64_t, uint32_t>("ul2ui"));
    convGroupInst->SetInst(0x61, new ConversionInst<uint64_t, int64_t>("ul2lo"));
    convGroupInst->SetInst(0x62, new ConversionInst<uint64_t, float>("ul2fl"));
    convGroupInst->SetInst(0x63, new ConversionInst<uint64_t, double>("ul2do"));
    convGroupInst->SetInst(0x64, new ConversionInst<uint64_t, char>("ul2ch"));
    convGroupInst->SetInst(0x65, new ConversionInst<uint64_t, char16_t>("ul2wc"));
    convGroupInst->SetInst(0x66, new ConversionInst<uint64_t, char32_t>("ul2uc"));
    convGroupInst->SetInst(0x67, new ConversionInst<uint64_t, bool>("ul2bo"));

    // from float:
    convGroupInst->SetInst(0x68, new ConversionInst<float, int8_t>("fl2sb"));
    convGroupInst->SetInst(0x69, new ConversionInst<float, uint8_t>("fl2by"));
    convGroupInst->SetInst(0x6A, new ConversionInst<float, int16_t>("fl2sh"));
    convGroupInst->SetInst(0x6B, new ConversionInst<float, uint16_t>("fl2us"));
    convGroupInst->SetInst(0x6C, new ConversionInst<float, int32_t>("fl2in"));
    convGroupInst->SetInst(0x6D, new ConversionInst<float, uint32_t>("fl2ui"));
    convGroupInst->SetInst(0x6E, new ConversionInst<float, int64_t>("fl2lo"));
    convGroupInst->SetInst(0x6F, new ConversionInst<float, uint64_t>("fl2ul"));
    convGroupInst->SetInst(0x70, new ConversionInst<float, double>("fl2do"));
    convGroupInst->SetInst(0x71, new ConversionInst<float, char>("fl2ch"));
    convGroupInst->SetInst(0x72, new ConversionInst<float, char16_t>("fl2wc"));
    convGroupInst->SetInst(0x73, new ConversionInst<float, char32_t>("fl2uc"));
    convGroupInst->SetInst(0x74, new ConversionInst<float, bool>("fl2bo"));

    // from double:
    convGroupInst->SetInst(0x75, new ConversionInst<double, int8_t>("do2sb"));
    convGroupInst->SetInst(0x76, new ConversionInst<double, uint8_t>("do2by"));
    convGroupInst->SetInst(0x77, new ConversionInst<double, int16_t>("do2sh"));
    convGroupInst->SetInst(0x78, new ConversionInst<double, uint16_t>("do2us"));
    convGroupInst->SetInst(0x79, new ConversionInst<double, int32_t>("do2in"));
    convGroupInst->SetInst(0x7A, new ConversionInst<double, uint32_t>("do2ui"));
    convGroupInst->SetInst(0x7B, new ConversionInst<double, int64_t>("do2lo"));
    convGroupInst->SetInst(0x7C, new ConversionInst<double, uint64_t>("do2ul"));
    convGroupInst->SetInst(0x7D, new ConversionInst<double, float>("do2fl"));
    convGroupInst->SetInst(0x7E, new ConversionInst<double, char>("do2ch"));
    convGroupInst->SetInst(0x7F, new ConversionInst<double, char16_t>("do2wc"));
    convGroupInst->SetInst(0x80, new ConversionInst<double, char32_t>("do2uc"));
    convGroupInst->SetInst(0x81, new ConversionInst<double, bool>("do2bo"));

    // from char:
    convGroupInst->SetInst(0x82, new ConversionInst<char, int8_t>("ch2sb"));
    convGroupInst->SetInst(0x83, new ConversionInst<char, uint8_t>("ch2by"));
    convGroupInst->SetInst(0x84, new ConversionInst<char, int16_t>("ch2sh"));
    convGroupInst->SetInst(0x85, new ConversionInst<char, uint16_t>("ch2us"));
    convGroupInst->SetInst(0x86, new ConversionInst<char, int32_t>("ch2in"));
    convGroupInst->SetInst(0x87, new ConversionInst<char, uint32_t>("ch2ui"));
    convGroupInst->SetInst(0x88, new ConversionInst<char, int64_t>("ch2lo"));
    convGroupInst->SetInst(0x89, new ConversionInst<char, uint64_t>("ch2ul"));
    convGroupInst->SetInst(0x90, new ConversionInst<char, float>("ch2fl"));
    convGroupInst->SetInst(0x91, new ConversionInst<char, double>("ch2fl"));
    convGroupInst->SetInst(0x92, new ConversionInst<char, char16_t>("ch2wc"));
    convGroupInst->SetInst(0x93, new ConversionInst<char, char32_t>("ch2uc"));
    convGroupInst->SetInst(0x94, new ConversionInst<char, bool>("ch2bo"));

    // from wchar:
    convGroupInst->SetInst(0x95, new ConversionInst<char16_t, int8_t>("wc2sb"));
    convGroupInst->SetInst(0x96, new ConversionInst<char16_t, uint8_t>("wc2by"));
    convGroupInst->SetInst(0x97, new ConversionInst<char16_t, int16_t>("wc2sh"));
    convGroupInst->SetInst(0x98, new ConversionInst<char16_t, uint16_t>("wc2us"));
    convGroupInst->SetInst(0x99, new ConversionInst<char16_t, int32_t>("wc2in"));
    convGroupInst->SetInst(0x9A, new ConversionInst<char16_t, uint32_t>("wc2ui"));
    convGroupInst->SetInst(0x9B, new ConversionInst<char16_t, int64_t>("wc2lo"));
    convGroupInst->SetInst(0x9C, new ConversionInst<char16_t, uint64_t>("wc2ul"));
    convGroupInst->SetInst(0x9D, new ConversionInst<char16_t, float>("wc2fl"));
    convGroupInst->SetInst(0x9E, new ConversionInst<char16_t, double>("wc2fl"));
    convGroupInst->SetInst(0x9F, new ConversionInst<char16_t, char>("wc2ch"));
    convGroupInst->SetInst(0xA0, new ConversionInst<char16_t, char32_t>("wc2uc"));
    convGroupInst->SetInst(0xA1, new ConversionInst<char16_t, bool>("wc2bo"));

    // from uchar:
    convGroupInst->SetInst(0xA2, new ConversionInst<char32_t, int8_t>("uc2sb"));
    convGroupInst->SetInst(0xA3, new ConversionInst<char32_t, uint8_t>("uc2by"));
    convGroupInst->SetInst(0xA4, new ConversionInst<char32_t, int16_t>("uc2sh"));
    convGroupInst->SetInst(0xA5, new ConversionInst<char32_t, uint16_t>("uc2us"));
    convGroupInst->SetInst(0xA6, new ConversionInst<char32_t, int32_t>("uc2in"));
    convGroupInst->SetInst(0xA7, new ConversionInst<char32_t, uint32_t>("uc2ui"));
    convGroupInst->SetInst(0xA8, new ConversionInst<char32_t, int64_t>("uc2lo"));
    convGroupInst->SetInst(0xA9, new ConversionInst<char32_t, uint64_t>("uc2ul"));
    convGroupInst->SetInst(0xAA, new ConversionInst<char32_t, float>("uc2fl"));
    convGroupInst->SetInst(0xAB, new ConversionInst<char32_t, double>("uc2fl"));
    convGroupInst->SetInst(0xAC, new ConversionInst<char32_t, char>("uc2ch"));
    convGroupInst->SetInst(0xAD, new ConversionInst<char32_t, char16_t>("uc2wc"));
    convGroupInst->SetInst(0xAE, new ConversionInst<char32_t, bool>("uc2bo"));
}

void Machine::AddInst(Instruction* inst)
{
    instructionMap[inst->Name()] = inst;
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
