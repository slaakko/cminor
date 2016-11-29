// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_DB_SHELL_INCLUDED
#define CMINOR_DB_SHELL_INCLUDED
#include <cminor/machine/Machine.hpp>

namespace cminor { namespace db {

using namespace cminor::machine;

class Shell
{
public:
    Shell(Machine& machine_);
    void StartMachine();
    void Run();
    void Exit();
    void Step();
    void Next();
    void Local(int index);
    void Stack(int index);
    void Print(IntegralValue value);
private:
    Machine& machine;
    bool exit;
};

} } // namespace cminor::db

#endif // CMINOR_DB_SHELL_INCLUDED
