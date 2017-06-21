// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/db/Command.hpp>
#include <cminor/db/Shell.hpp>
#include <cminor/util/Unicode.hpp>
#include <iostream>

namespace cminor { namespace db {

using namespace cminor::unicode;

Command::~Command()
{
}

void HelpCommand::Execute(Shell& shell)
{
    std::cout <<
        "start                 : start debugging\n" <<
        "e(xit)                : stop debugging\n" <<
        "q(uit)                : stop debugging\n" << 
        "r(un)                 : run\n" <<
        "s(tep)                : single step\n" << 
        "n(ext)                : step over\n" <<
        "(stac)k               : print stack trace\n" <<
        "p(rint) <name>        : print value of variable <name>\n" <<
        "l(ocal) <n>           : print value of local <n>\n" <<
        "o(perand) <n>         : print value of operand <n> in operand stack (0 is top)\n" <<
        "a(llocation) <n>      : print info for allocation <n>\n" <<
        "f(ield) <n> <m>       : print field or element <m> of object or array <n>\n" <<
        "(lis)t [<file>:]<n>   : list source file <file> from line <n>\n" <<
        "                        (if <file> is missing use current file)\n" <<
        "b(reak) [<file>:]<n>  : set breakpoint at <file> line <n>\n" <<
        "                        (if <file> is missing use current file)\n" <<
        "c(lear) <n>           : clear breakpoint number <n>\n" <<
        "show breakpoints      : show breakpoints\n" <<
        "ENTER                 : repeat last command\n" <<
        std::endl;
}

void StartCommand::Execute(Shell& shell)
{
    shell.StartMachine();
}

void ExitCommand::Execute(Shell& shell)
{
    shell.Exit();
}

void StepCommand::Execute(Shell& shell)
{
    shell.Step();
}

void NextCommand::Execute(Shell& shell)
{
    shell.Next();
}

void RunCommand::Execute(Shell& shell)
{
    shell.Run();
}

LocalCommand::LocalCommand(int index_) : index(index_)
{
}

void LocalCommand::Execute(Shell& shell)
{
    shell.Local(index);
}

OperandCommand::OperandCommand(int index_) : index(index_)
{
}

void OperandCommand::Execute(Shell& shell)
{
    shell.Operand(index);
}

void PrevCommand::Execute(Shell& shell)
{
    shell.RepeatLastCommand();
}

AllocationCommand::AllocationCommand(int handle_) : handle(handle_)
{
}

void AllocationCommand::Execute(Shell& shell)
{
    shell.PrintAllocation(handle);
}

FieldCommand::FieldCommand(int handle_, int index_) : handle(handle_), index(index_)
{
}

void FieldCommand::Execute(Shell& shell)
{
    shell.PrintField(handle, index);
}

ListCommand::ListCommand(const std::string& sourceFileName_, int line_) : sourceFileName(sourceFileName_), line(line_)
{
}

void ListCommand::Execute(Shell& shell)
{
    shell.List(sourceFileName, line);
}

BreakCommand::BreakCommand(const std::string& sourceFileName_, int line_) : sourceFileName(sourceFileName_), line(line_)
{
}

void BreakCommand::Execute(Shell& shell)
{
    shell.Break(sourceFileName, line);
}

ClearCommand::ClearCommand(int bp_) : bp(bp_)
{
}

void ClearCommand::Execute(Shell& shell)
{
    shell.Clear(bp);
}

void ShowBreakpointsCommand::Execute(Shell& shell)
{
    shell.ShowBreakpoints();
}

void StackCommand::Execute(Shell& shell)
{
    shell.Stack();
}

PrintCommand::PrintCommand(const std::u32string& name_) : name(name_)
{
}

void PrintCommand::Execute(Shell& shell)
{
    shell.Print(ToUtf8(name));
}

} } // namespace cminor::db
