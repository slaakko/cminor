// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/db/Command.hpp>
#include <cminor/db/Shell.hpp>
#include <iostream>

namespace cminor { namespace db {

Command::~Command()
{
}

void HelpCommand::Execute(Shell& shell)
{
    std::cout <<
        "start       : start debugging\n" <<
        "e(xit)      : stop debugging\n" <<
        "q(uit)      : stop debugging\n" << 
        "s(tep)      : single step\n" << 
        "n(ext)      : step over\n" <<
        "l(ocal) <n> : print value of local <n>\n" <<
        "(stac)k <n> : print value of operand <n> in stack (0 is top)\n" <<
        "ENTER       : repeat last command\n" <<
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

LocalCommand::LocalCommand(int index_) : index(index_)
{
}

void LocalCommand::Execute(Shell& shell)
{
    shell.Local(index);
}

StackCommand::StackCommand(int index_) : index(index_)
{
}

void StackCommand::Execute(Shell& shell)
{
    shell.Stack(index);
}

void PrevCommand::Execute(Shell& shell)
{
    shell.RepeatLastCommand();
}

} } // namespace cminor::db
