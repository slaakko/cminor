// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_GEN_OBJECT_INCLUDED
#define CMINOR_MACHINE_GEN_OBJECT_INCLUDED
#include <cminor/machine/MachineApi.hpp>

namespace cminor { namespace machine {

class Machine;
class Function;
class GenObject;

class MACHINE_API GenVisitor
{
public:
    virtual ~GenVisitor();
    virtual void Visit(GenObject& object) = 0;
};

class MACHINE_API GenObject
{
public:
    virtual ~GenObject();
    virtual void GenLoad(Machine& machine, Function& function) = 0;
    virtual void GenStore(Machine& machine, Function& function) = 0;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_GEN_OBJECT_INCLUDED
