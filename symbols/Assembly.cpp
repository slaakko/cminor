// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolReader.hpp>
#include <cminor/symbols/GlobalFlags.hpp>
#include <cminor/ast/Project.hpp>
#include <boost/filesystem.hpp>

namespace cminor { namespace symbols {

using namespace cminor::ast;

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

Assembly::Assembly() : filePath(), constantPool(), symbolTable(this), name()
{
}

Assembly::Assembly(const utf32_string& name_, const std::string& filePath_) : filePath(filePath_), constantPool(), symbolTable(this),
    name(constantPool.GetConstant(constantPool.Install(StringPtr(name_.c_str()))))
{
}

void Assembly::Write(SymbolWriter& writer)
{
    AssemblyTag tag;
    tag.Write(writer);
    writer.SetAssembly(this);
    static_cast<Writer&>(writer).Put(filePath);
    int32_t n = int32_t(importedAssemblies.size());
    static_cast<Writer&>(writer).Put(n);
    for (int32_t i = 0; i < n; ++i)
    {
        const std::unique_ptr<Assembly>& importedAssembly = importedAssemblies[i];
        static_cast<Writer&>(writer).Put(importedAssembly->FilePath());
    }
    constantPool.Write(writer);
    ConstantId nameId = constantPool.GetIdFor(name);
    Assert(nameId != noConstantId, "constant id for name constant not found");
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
        throw std::runtime_error("invalid cminor assembly tag read from file '" + filePath + "'. (not an assembly file?)");
    }
    reader.SetAssembly(this);
    filePath = reader.GetUtf8String();
    int32_t n = reader.GetInt();
    for (int32_t i = 0; i < n; ++i)
    {
        std::string referenceFilePath = reader.GetUtf8String();
        referenceFilePaths.push_back(referenceFilePath);
    }
    constantPool.Read(reader);
    ConstantId nameId;
    nameId.Read(reader);
    name = constantPool.GetConstant(nameId);
    symbolTable.Read(reader);
}

bool Assembly::IsSystemAssembly() const
{
    return StringPtr(name.Value().AsStringLiteral()) == StringPtr(U"system");
}

void Assembly::ImportAssemblies(Machine& machine)
{
    ImportAssemblies(machine, referenceFilePaths);
}

void Assembly::ImportAssemblies(Machine& machine, const std::vector<std::string>& assemblyReferences)
{
    std::unordered_set<std::string> importSet;
    std::vector<std::string> allAssemblyReferences;
    if (!IsSystemAssembly())
    {
        allAssemblyReferences.push_back(CminorSystemAssemblyFilePath(GetConfig()));
    }
    allAssemblyReferences.insert(allAssemblyReferences.end(), assemblyReferences.cbegin(), assemblyReferences.cend());
    Import(machine, allAssemblyReferences, importSet);
}

void Assembly::Import(Machine& machine, const std::vector<std::string>& assemblyReferences, std::unordered_set<std::string>& importSet)
{
    for (const std::string& assemblyReference : assemblyReferences)
    {
        if (importSet.find(assemblyReference) == importSet.cend())
        {
            importSet.insert(assemblyReference);
            std::unique_ptr<Assembly> referencedAssembly(new Assembly());
            SymbolReader reader(machine, assemblyReference);
            referencedAssembly->Read(reader);
            symbolTable.Import(referencedAssembly->symbolTable);
            Assembly* referencedAssemblyPtr = referencedAssembly.get();
            importedAssemblies.push_back(std::move(referencedAssembly));
            Import(machine, referencedAssemblyPtr->referenceFilePaths, importSet);
        }
    }
}

} } // namespace cminor::symbols
