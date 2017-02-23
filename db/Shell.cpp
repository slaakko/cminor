// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/db/Shell.hpp>
#include <cminor/db/Command.hpp>
#include <cminor/db/CommandGrammar.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/VariableSymbol.hpp>
#include <cminor/machine/OsInterface.hpp>
#include <stdexcept>
#include <iostream>

namespace cminor { namespace db {

Shell::Shell(Machine& machine_) : machine(machine_), exit(false), numListLines(10), currentLineNumber(0), currentSourceFile(nullptr), ended(false), argsArrayObjectType(nullptr)
{
}

void Shell::StartMachine()
{
    if (mainFun->Arity() != 1 && mainFun->Arity() != 0)
    {
        throw std::runtime_error("main function has invalid arity " + std::to_string(mainFun->Arity()));
    }
    bool startWithArguments = mainFun->Arity() == 1;
    machine.Start(startWithArguments, programArguments, argsArrayObjectType);
    ended = false;
}

void Shell::Run(FunctionSymbol* mainFun_, Assembly& assembly, const std::vector<utf32_string>& programArguments_)
{
    mainFun = mainFun_;
    programArguments = programArguments_;
    argsArrayObjectType = nullptr;
    if (mainFun->Arity() != 1 && mainFun->Arity() != 0)
    {
        throw std::runtime_error("main function has invalid arity " + std::to_string(mainFun->Arity()));
    }
    if (mainFun->Arity() == 1)
    {
        TypeSymbol* argsParamType = mainFun->Parameters()[0]->GetType();
        ArrayTypeSymbol* argsArrayType = dynamic_cast<ArrayTypeSymbol*>(argsParamType);
        if (argsArrayType && argsArrayType->ElementType() == assembly.GetSymbolTable().GetType(U"System.String"))
        {
            argsArrayObjectType = argsArrayType->GetObjectType();
        }
        else
        {
            throw Exception("parameter type of program main function is not string[]", mainFun->GetSpan());
        }
    }
    StartMachine();
    CommandGrammar* commandGrammar = CommandGrammar::Create();
    if (!machine.MainThread().GetStack().IsEmpty())
    {
        Frame* frame = machine.MainThread().GetStack().CurrentFrame();
        if (frame->Fun().HasSourceFilePath())
        {
            currentSourceFile = SourceFileTable::GetSourceFile(frame->Fun().SourceFilePath());
        }
    }
    while (!exit)
    {
        try
        {
            CodeFormatter codeFormatter(std::cout);
            if (!machine.MainThread().GetStack().IsEmpty())
            {
                int n = int(machine.MainThread().GetStack().Frames().size());
                for (int i = n - 1; i >= 0; --i)
                {
                    Frame* frame = machine.MainThread().GetStack().Frames()[i];
                    if (frame->PC() < frame->Fun().NumInsts())
                    {
                        frame->Fun().Dump(codeFormatter, frame->PC());
                        break;
                    }
                }
            }
            else
            {
                if (!ended)
                {
                    if (mainFun->ReturnType() == assembly.GetSymbolTable().GetType(U"System.Int32"))
                    {
                        IntegralValue programReturnValue = machine.MainThread().OpStack().Pop();
                        codeFormatter.WriteLine("program ended with exit code " + std::to_string(programReturnValue.AsInt()));
                        ended = true;
                    }
                    else
                    {
                        codeFormatter.WriteLine("program ended normally");
                        ended = true;
                    }
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
        catch (const Exception& ex)
        {
            std::cerr << ex.What() << std::endl;
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
    if (ended)
    {
        std::cout << "program ended" << std::endl;
    }
    else
    {
        machine.MainThread().Step();
        if (!machine.MainThread().GetStack().IsEmpty())
        {
            Frame* frame = machine.MainThread().GetStack().CurrentFrame();
            if (frame->Fun().HasSourceFilePath())
            {
                currentSourceFile = SourceFileTable::GetSourceFile(frame->Fun().SourceFilePath());
            }
        }
    }
}

void Shell::Next()
{
    if (ended)
    {
        std::cout << "program ended" << std::endl;
    }
    else
    {
        machine.MainThread().Next();
        if (!machine.MainThread().GetStack().IsEmpty())
        {
            Frame* frame = machine.MainThread().GetStack().CurrentFrame();
            if (frame->Fun().HasSourceFilePath())
            {
                currentSourceFile = SourceFileTable::GetSourceFile(frame->Fun().SourceFilePath());
            }
        }
    }
}

void Shell::Run()
{
    if (ended)
    {
        std::cout << "program ended" << std::endl;
    }
    else
    {
        machine.MainThread().RunDebug();
        if (!machine.MainThread().GetStack().IsEmpty())
        {
            Frame* frame = machine.MainThread().GetStack().CurrentFrame();
            if (frame->Fun().HasSourceFilePath())
            {
                currentSourceFile = SourceFileTable::GetSourceFile(frame->Fun().SourceFilePath());
            }
        }
    }
}

void Shell::Local(int index)
{
    Frame* frame = machine.MainThread().GetStack().CurrentFrame();
    if (index < 0)
    {
        throw std::runtime_error("invalid local index");
    }
    else if (index >= int32_t(frame->Fun().NumLocals()))
    {
        throw std::runtime_error("function has only " + std::to_string(frame->Fun().NumLocals()) + " locals");
    }
    IntegralValue value = frame->Local(index).GetValue();
    Print(value);
}

void Shell::Operand(int index)
{
    Frame* frame = machine.MainThread().GetStack().CurrentFrame();
    int32_t n = int32_t(frame->OpStack().Values().size());
    if (index < 0)
    {
        throw std::runtime_error("invalid operand index");
    }
    else if (index >= n)
    {
        throw std::runtime_error("stack has only " + std::to_string(n) + " operands");
    }
    IntegralValue value = frame->OpStack().GetValue(index + 1);
    Print(value);
}

void Shell::Print(IntegralValue value)
{
    CodeFormatter formatter(std::cout);
    value.Dump(formatter);
}

void Shell::Exit()
{
    exit = true;
}

void Shell::RepeatLastCommand()
{
    if (prevCommand)
    {
        if (ListCommand* listCommand = dynamic_cast<ListCommand*>(prevCommand.get()))
        {
            listCommand->SetLine(currentLineNumber);
        }
        prevCommand->Execute(*this);
    }
    else
    {
        throw std::runtime_error("no previous command to execute");
    }
}

void Shell::PrintAllocation(int handle)
{
    Frame* frame = machine.MainThread().GetStack().CurrentFrame();
    AllocationHandle allocationHandle(handle);
    ManagedAllocation* allocation = GetManagedMemoryPool().GetAllocation(allocationHandle);
    Object* o = dynamic_cast<Object*>(allocation);
    if (o)
    {
        ObjectType* type = o->GetType();
        utf32_string us = type->Name().Value();
        std::string s = ToUtf8(us);
        std::cout << "object " << s << " : " << o->FieldCount() << std::endl;
    }
    else
    {
        ArrayElements* a = dynamic_cast<ArrayElements*>(allocation);
        if (a)
        {
            Type* type = a->GetElementType();
            std::cout << "array of " << ToUtf8(type->Name().Value()) << " : " << a->NumElements() << std::endl;
        }
        else
        {
            StringCharacters* s = dynamic_cast<StringCharacters*>(allocation);
            if (s)
            {
                std::cout << "string \"";
                std::string str;
                int n = s->NumChars();
                for (int i = 0; i < n; ++i)
                {
                    IntegralValue v = s->GetChar(i);
                    char32_t c = v.AsChar();
                    char d = static_cast<char>(c);
                    str.append(1, d);
                }
                std::cout << str << "\"" << std::endl;
            }
            else
            {
                throw std::runtime_error("invalid allocation " + std::to_string(handle));
            }
        }
    }
}

void Shell::PrintField(int handle, int index)
{
    Frame* frame = machine.MainThread().GetStack().CurrentFrame();
    AllocationHandle allocationHandle(handle);
    ManagedAllocation* allocation = GetManagedMemoryPool().GetAllocation(allocationHandle);
    Object* o = dynamic_cast<Object*>(allocation);
    if (o)
    {
        IntegralValue value = o->GetField(index);
        Print(value);
    }
    else
    {
        ArrayElements* a = dynamic_cast<ArrayElements*>(allocation);
        if (a)
        {
            IntegralValue value = a->GetElement(index);
            Print(value);
        }
        else
        {
            throw std::runtime_error("handle does not denote an object or an array " + std::to_string(handle));
        }
    }
}

void Shell::List(const std::string& sourceFileName, int line)
{
    SourceFile* sourceFile = nullptr;
    if (sourceFileName.empty())
    {
        if (currentSourceFile)
        {
            Constant sourceFilePath = currentSourceFile->SourceFilePath();
            sourceFile = SourceFileTable::GetSourceFile(sourceFilePath);
        }
        else
        {
            throw std::runtime_error("current source file not set");
        }
    }
    else
    {
        sourceFile = SourceFileTable::GetSourceFile(sourceFileName);
    }
    if (line == -1)
    {
        Frame* frame = machine.MainThread().GetStack().CurrentFrame();
        uint32_t pc = frame->PC();
        line = frame->Fun().GetSourceLine(pc);
        while (line == -1 && pc != -1)
        {
            --pc;
            line = frame->Fun().GetSourceLine(pc);
        }
    }
    if (line <= 0)
    {
        line = 1;
    }
    std::string sfp = ToUtf8(sourceFile->SourceFilePath().Value().AsStringLiteral());
    WriteInGreenToConsole(sfp + ":");
    sourceFile->List(line, numListLines);
    currentSourceFile = sourceFile;
    currentLineNumber = line + numListLines;
}

void Shell::Break(const std::string& sourceFileName, int line)
{
    Constant sourceFilePath;
    if (sourceFileName.empty())
    {
        if (currentSourceFile)
        {
            sourceFilePath = currentSourceFile->SourceFilePath();
        }
        else
        {
            throw std::runtime_error("current source file not set");
        }
    }
    else
    {
        sourceFilePath = SourceFileTable::GetSourceFilePath(sourceFileName);
    }
    int bp = SourceFileTable::SetBreakPoint(sourceFilePath, line);
    std::cout << "breakpoint number " << bp << " set" << std::endl;
    breakpoints.insert(bp);
}

void Shell::Clear(int bp)
{
    SourceFileTable::RemoveBreakPoint(bp);
    breakpoints.erase(bp);
}

void Shell::ShowBreakpoints()
{
    for (int bp : breakpoints)
    {
        const BreakPoint* breakPoint = SourceFileTable::GetBreakPoint(bp);
        std::cout << "breakpoint " << bp << " at " << ToUtf8(breakPoint->GetFunction()->CallName().Value().AsStringLiteral()) << " line " << breakPoint->Line() << " pc " << breakPoint->PC() << std::endl;
    }
}

void Shell::Stack()
{
    Frame* frame = machine.MainThread().GetStack().CurrentFrame();
    std::string stackTrace = ToUtf8(frame->GetThread().GetStackTrace());
    std::cout << stackTrace << std::endl;
}

} } // namespace cminor::db
