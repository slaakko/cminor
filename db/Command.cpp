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
        "start  : start debugging\n" <<
        "e(xit) : stop debugging\n" <<
        "q(uit) : stop debugging\n" << 
        "s(tep) : single step\n" << 
        "n(ext) : step over\n" <<
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

} } // namespace cminor::db
