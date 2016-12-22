// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_DB_SHELL_INCLUDED
#define CMINOR_DB_SHELL_INCLUDED
#include <cminor/symbols/FunctionSymbol.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/db/Command.hpp>

namespace cminor { namespace db {

using namespace cminor::machine;
using namespace cminor::symbols;

class Command;

class Shell
{
public:
    Shell(Machine& machine_);
    void StartMachine();
    void Run(FunctionSymbol* mainFun, Assembly& assembly, const std::vector<utf32_string>& programArguments_, ObjectType* argsArrayObjectType_);
    void Exit();
    void Step();
    void Next();
    void Run();
    void Local(int index);
    void Stack(int index);
    void Print(IntegralValue value);
    void RepeatLastCommand();
private:
    Machine& machine;
    bool exit;
    std::unique_ptr<Command> prevCommand;
    std::vector<utf32_string> programArguments;
    ObjectType* argsArrayObjectType;
};

} } // namespace cminor::db

#endif // CMINOR_DB_SHELL_INCLUDED
