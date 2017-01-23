// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_DB_SHELL_INCLUDED
#define CMINOR_DB_SHELL_INCLUDED
#include <cminor/symbols/FunctionSymbol.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/db/Command.hpp>
#include <set>

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
    void Operand(int index);
    void Print(IntegralValue value);
    void RepeatLastCommand();
    void PrintAllocation(int handle);
    void PrintField(int handle, int index);
    void List(const std::string& sourceFileName, int line);
    void Break(const std::string& sourceFileName, int line);
    void Clear(int bp);
    void ShowBreakpoints();
    void Stack();
private:
    Machine& machine;
    bool exit;
    std::unique_ptr<Command> prevCommand;
    std::vector<utf32_string> programArguments;
    ObjectType* argsArrayObjectType;
    int numListLines;
    int currentLineNumber;
    SourceFile* currentSourceFile;
    std::set<int> breakpoints;
    bool ended;
};

} } // namespace cminor::db

#endif // CMINOR_DB_SHELL_INCLUDED
