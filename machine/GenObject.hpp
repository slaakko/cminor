// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_GEN_OBJECT_INCLUDED
#define CMINOR_MACHINE_GEN_OBJECT_INCLUDED

namespace cminor { namespace machine {

class Machine;
class Function;

class GenObject
{
public:
    virtual ~GenObject();
    virtual void GenLoad(Machine& machine, Function& function) = 0;
    virtual void GenStore(Machine& machine, Function& function) = 0;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_GEN_OBJECT_INCLUDED
