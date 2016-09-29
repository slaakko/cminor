// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_ASSEMBLY_INCLUDED
#define CMINOR_MACHINE_ASSEMBLY_INCLUDED
#include <cminor/machine/Constant.hpp>
#include <cminor/machine/Type.hpp>

namespace cminor { namespace machine {

class Reader;

class Assembly
{
public:
    Assembly(const std::string& fileName_);
    void Read(Reader& reader);
    ConstantPool& GetConstantPool() { return constantPool; }
    TypeRepository& GetTypeRepository() { return typeRepository; }
private:
    std::string fileName;
    ConstantPool constantPool;
    TypeRepository typeRepository;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_ASSEMBLY_INCLUDED
