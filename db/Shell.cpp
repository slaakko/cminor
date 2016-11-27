// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/db/Shell.hpp>
#include <cminor/db/Command.hpp>
#include <cminor/db/CommandGrammar.hpp>
#include <stdexcept>
#include <iostream>

namespace cminor { namespace db {

Shell::Shell(Machine& machine_) : machine(machine_), exit(false)
{
}

void Shell::StartMachine()
{
    machine.Start();
}

void Shell::Run()
{
    StartMachine();
    CommandGrammar* commandGrammar = CommandGrammar::Create();
    while (!exit)
    {
        try
        {
            CodeFormatter codeFormatter(std::cout);
            if (!machine.MainThread().Frames().empty())
            {
                Frame& frame = machine.MainThread().Frames().back();
                frame.Fun().Dump(codeFormatter, frame.PC());
            }
            else
            {
                codeFormatter.WriteLine("program ended");
            }
            std::cout << "cminordb> ";
            std::string line;
            std::getline(std::cin, line);
            std::unique_ptr<Command> command(commandGrammar->Parse(line.c_str(), line.c_str() + line.length(), 0, ""));
            command->Execute(*this);
            if (exit)
            {
                break;
            }
        }
        catch (const std::exception&)
        {
            std::cout << "unknown command, try help" << std::endl;
        }
    }
}

void Shell::Step()
{
    machine.MainThread().Step();
}

void Shell::Next()
{
    machine.MainThread().Next();
}

void Shell::Exit()
{
    exit = true;
}

} } // namespace cminor::db
