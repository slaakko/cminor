// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/db/Shell.hpp>
#include <cminor/db/Command.hpp>
#include <cminor/db/CommandGrammar.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <stdexcept>
#include <iostream>

namespace cminor { namespace db {

Shell::Shell(Machine& machine_) : machine(machine_), exit(false)
{
}

void Shell::StartMachine()
{
    machine.Start(programArguments, argsArrayObjectType);
}

void Shell::Run(FunctionSymbol* mainFun, Assembly& assembly, const std::vector<utf32_string>& programArguments_, ObjectType* argsArrayObjectType_)
{
    programArguments = programArguments_;
    argsArrayObjectType = argsArrayObjectType_;
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
                if (mainFun->ReturnType() == assembly.GetSymbolTable().GetType(U"System.Int32"))
                {
                    IntegralValue programReturnValue = machine.MainThread().OpStack().Pop();
                    codeFormatter.WriteLine("program ended with exit code " + std::to_string(programReturnValue.AsInt()));
                }
                else
                {
                    codeFormatter.WriteLine("program ended normally");
                }
            }
            std::cout << "cminordb> ";
            std::string line;
            std::getline(std::cin, line);
            std::unique_ptr<Command> command(commandGrammar->Parse(line.c_str(), line.c_str() + line.length(), 0, ""));
            command->Execute(*this);
            if (!dynamic_cast<PrevCommand*>(command.get()))
            {
                prevCommand.reset(command.release());
            }
            if (exit)
            {
                break;
            }
        }
        catch (const std::exception& ex)
        {
            std::cout << ex.what() << std::endl;
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

void Shell::Local(int index)
{
    Frame& frame = machine.MainThread().Frames().back();
    if (index < 0)
    {
        throw std::runtime_error("invalid local index");
    }
    else if (index >= int32_t(frame.Fun().NumLocals()))
    {
        throw std::runtime_error("function has only " + std::to_string(frame.Fun().NumLocals()) + " locals");
    }
    IntegralValue value = frame.Local(index).GetValue();
    Print(value);
}

void Shell::Stack(int index)
{
    Frame& frame = machine.MainThread().Frames().back();
    int32_t n = int32_t(frame.OpStack().Values().size());
    if (index < 0)
    {
        throw std::runtime_error("invalid operand index");
    }
    else if (index >= n)
    {
        throw std::runtime_error("stack has only " + std::to_string(n) + " operands");
    }
    IntegralValue value = frame.OpStack().GetValue(index + 1);
    Print(value);
}

void Shell::Print(IntegralValue value)
{
    switch (value.GetType())
    {
        case ValueType::stringLiteral:
        {
            std::cout << "string literal \"" + ToUtf8(value.AsStringLiteral()) + "\"" << std::endl;
            break;
        }
        case ValueType::boolType:
        {
            std::string s = "true";
            if (!value.AsBool())
            {
                s = "false";
            }
            std::cout << ValueTypeStr(value.GetType()) << " " << s << std::endl;
            break;
        }
        default:
        {
            std::cout << ValueTypeStr(value.GetType()) << " " << value.AsULong() << std::endl;
            break;
        }
    }
}

void Shell::Exit()
{
    exit = true;
}

void Shell::RepeatLastCommand()
{
    if (prevCommand)
    {
        prevCommand->Execute(*this);
    }
    else
    {
        throw std::runtime_error("no previous command to execute");
    }
}

} } // namespace cminor::db
