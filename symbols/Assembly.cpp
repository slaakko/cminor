// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolReader.hpp>

namespace cminor { namespace symbols {

const char* cminorAssemblyTag = "CMNA";

class AssemblyTag
{
public:
    AssemblyTag();
    void Write(Writer& writer);
    void Read(Reader& reader);
    char operator[](int index) const { return tag[index]; }
private:
    char tag[4];
};

AssemblyTag::AssemblyTag()
{
    tag[0] = cminorAssemblyTag[0];
    tag[1] = cminorAssemblyTag[1];
    tag[2] = cminorAssemblyTag[2];
    tag[3] = cminorAssemblyTag[3];
}

void AssemblyTag::Write(Writer& writer)
{
    writer.Put(static_cast<uint8_t>(tag[0]));
    writer.Put(static_cast<uint8_t>(tag[1]));
    writer.Put(static_cast<uint8_t>(tag[2]));
    writer.Put(static_cast<uint8_t>(tag[3]));
}

void AssemblyTag::Read(Reader& reader)
{
    tag[0] = static_cast<char>(reader.GetByte());
    tag[1] = static_cast<char>(reader.GetByte());
    tag[2] = static_cast<char>(reader.GetByte());
    tag[3] = static_cast<char>(reader.GetByte());
}

inline bool operator==(const AssemblyTag& left, const AssemblyTag& right)
{
    return left[0] == right[0] && left[1] == right[1] && left[2] == right[2] && left[3] == right[3];
}

inline bool operator!=(const AssemblyTag& left, const AssemblyTag& right)
{
    return !(left == right);
}

Assembly::Assembly() : fileName(), constantPool(), symbolTable(this), name()
{
}

Assembly::Assembly(const utf32_string& name_, const std::string& fileName_) : fileName(fileName_), constantPool(), symbolTable(this),
    name(constantPool.GetConstant(constantPool.Install(StringPtr(name_.c_str()))))
{
}

void Assembly::Write(SymbolWriter& writer)
{
    AssemblyTag tag;
    tag.Write(writer);
    writer.SetAssembly(this);
    static_cast<Writer&>(writer).Put(fileName);
    constantPool.Write(writer);
    ConstantId nameId = constantPool.GetIdFor(name);
    nameId.Write(writer);
    symbolTable.Write(writer);
}

void Assembly::Read(SymbolReader& reader)
{
    AssemblyTag defaultTag;
    AssemblyTag tagRead;
    tagRead.Read(reader);
    if (tagRead != defaultTag)
    {
        throw std::runtime_error("invalid cminor assembly tag read from file '" + fileName + "'. (not an assembly file?)");
    }
    reader.SetAssembly(this);
    fileName = reader.GetUtf8String();
    constantPool.Read(reader);
    ConstantId nameId;
    nameId.Read(reader);
    name = constantPool.GetConstant(nameId);
    symbolTable.Read(reader);
}

} } // namespace cminor::symbols
