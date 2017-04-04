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
#include <cminor/util/TextUtils.hpp>
#include <cminor/vmlib/File.hpp>
#include <stdexcept>
#include <iostream>

namespace cminor { namespace db {

using namespace cminor::vmlib;

struct BinaryModeChanger
{
    BinaryModeChanger(Shell& shell_) : shell(shell_)
    {
        shell.SwitchToBinaryMode();
    }
    ~BinaryModeChanger()
    {
        shell.SwitchToTextMode();
    }
    Shell& shell;
};

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
    SwitchToTextMode();
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
        BinaryModeChanger changer(*this);
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
        BinaryModeChanger changer(*this);
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
        BinaryModeChanger changer(*this);
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
    Print(value, formatter);
}

void Shell::Print(IntegralValue value, CodeFormatter& formatter)
{
    value.Dump(formatter);
}

void Shell::Print(IntegralValue value, TypeSymbol* type, CodeFormatter& formatter)
{
    if (type->IsArrayType())
    {
        ArrayTypeSymbol* arrayType = static_cast<ArrayTypeSymbol*>(type);
        TypeSymbol* elementType = arrayType->ElementType();
        IntegralValue length = GetManagedMemoryPool().GetField(ObjectReference(value.Value()), 1);
        if (length.GetType() != ValueType::intType)
        {
            throw std::runtime_error("int type expected");
        }
        int n = length.AsInt();
        formatter.WriteLine("array of " + std::to_string(n) + " '" + ToUtf8(elementType->FullName()) + "':");
        formatter.WriteLine("[");
        formatter.IncIndent();
        IntegralValue elements = GetManagedMemoryPool().GetField(ObjectReference(value.Value()), 2);
        if (elements.GetType() != ValueType::allocationHandle)
        {
            throw std::runtime_error("allocation handle expected");
        }
        void* arrayElements = GetManagedMemoryPool().GetAllocationNoThrowNoLock(AllocationHandle(elements.Value()));
        for (int i = 0; i < n; ++i)
        {
            formatter.Write(std::to_string(i) + ": ");
            IntegralValue elementValue = GetElement(arrayElements, i);
            Print(elementValue, elementType, formatter);
        }
        formatter.DecIndent();
        formatter.WriteLine("]");
    }
    else if (type->IsClassType())
    {
        if (type->FullName() == U"System.String")
        {
            ObjectReference strReference(value.Value());
            if (strReference.IsNull())
            {
                formatter.WriteLine("null");
            }
            else
            {
                std::string str = GetManagedMemoryPool().GetUtf8String(strReference);
                std::string s = "\"" + StringStr(str) + "\"";
                formatter.WriteLine(s);
            }
        }
        else
        {
            ClassTypeSymbol* classType = static_cast<ClassTypeSymbol*>(type);
            ObjectReference objectRef(value.Value());
            if (objectRef.IsNull())
            {
                formatter.WriteLine("object of class '" + ToUtf8(classType->FullName()) + "': null");
            }
            else
            {
                formatter.WriteLine("object of class '" + ToUtf8(classType->FullName()) + "':");
                formatter.WriteLine("{");
                formatter.IncIndent();
                int n = int(classType->MemberVariables().size());
                for (int i = 0; i < n; ++i)
                {
                    MemberVariableSymbol* memberVariableSymbol = classType->MemberVariables()[i];
                    uint32_t index = memberVariableSymbol->Index();
                    IntegralValue fieldValue = GetManagedMemoryPool().GetField(objectRef, i + 1);
                    Print(fieldValue, memberVariableSymbol->GetType(), formatter);
                }
                formatter.DecIndent();
                formatter.WriteLine("}");
            }
        }
    }
    else
    {
        Print(value, formatter);
    }
}

void Shell::Print(const std::string& name)
{
    if (!machine.MainThread().GetStack().IsEmpty())
    {
        Frame* frame = machine.MainThread().GetStack().CurrentFrame();
        Function& fun = frame->Fun();
        FunctionSymbol* functionSymbol = static_cast<FunctionSymbol*>(fun.FunctionSymbol());
        ContainerScope* scope = functionSymbol->GetMappedContainerScopeForPC(frame->PC());
        if (scope)
        {
            utf32_string s = ToUtf32(name);
            Symbol* symbol = scope->Lookup(StringPtr(s.c_str()), ScopeLookup::this_and_base_and_parent);
            if (symbol)
            {
                VariableSymbol* variableSymbol = dynamic_cast<VariableSymbol*>(symbol);
                if (variableSymbol)
                {
                    ParameterSymbol* parameterSymbol = dynamic_cast<ParameterSymbol*>(variableSymbol);
                    if (parameterSymbol)
                    {
                        int index = parameterSymbol->Index();
                        LocalVariable& localVariable = frame->Local(index);
                        CodeFormatter formatter(std::cout);
                        Print(localVariable.GetValue(), parameterSymbol->GetType(), formatter);
                    }
                    else
                    {
                        LocalVariableSymbol* localVariableSymbol = dynamic_cast<LocalVariableSymbol*>(variableSymbol);
                        if (localVariableSymbol)
                        {
                            int index = localVariableSymbol->Index();
                            CodeFormatter formatter(std::cout);
                            LocalVariable& localVariable = frame->Local(index);
                            Print(localVariable.GetValue(), localVariableSymbol->GetType(), formatter);
                        }
                        else
                        {
                            MemberVariableSymbol* memberVariableSymbol = dynamic_cast<MemberVariableSymbol*>(variableSymbol);
                            if (memberVariableSymbol)
                            {
                                Symbol* thisSymbol = scope->Lookup(StringPtr(U"this"), ScopeLookup::this_and_base_and_parent);
                                if (thisSymbol)
                                {
                                    LocalVariableSymbol* thisVariableSymbol = dynamic_cast<LocalVariableSymbol*>(thisSymbol);
                                    if (thisVariableSymbol)
                                    {
                                        int thisIndex = thisVariableSymbol->Index();
                                        LocalVariable& thisVariable = frame->Local(thisIndex);
                                        int index = memberVariableSymbol->Index();
                                        CodeFormatter formatter(std::cout);
                                        IntegralValue fieldValue = GetManagedMemoryPool().GetField(ObjectReference(thisVariable.GetValue().Value()), index);
                                        Print(fieldValue, memberVariableSymbol->GetType(), formatter);
                                    }
                                }
                                else
                                {
                                    throw std::runtime_error("'this' not found from current scope");
                                }
                            }
                            else
                            {
                                throw std::runtime_error("symbol '" + name + "' is not a parameter, local variable or member variable symbol");
                            }
                        }
                    }
                }
                else
                {
                    throw std::runtime_error("symbol '" + name + "' is not a variable symbol");
                }
            }
            else
            {
                throw std::runtime_error("symbol '" + name + "' not found");
            }
        }
        else
        {
            throw std::runtime_error("no scope for current instruction");
        }
    }
    else
    {
        throw std::runtime_error("no stack frame");
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
    void* allocation = GetManagedMemoryPool().GetAllocationNoThrowNoLock(allocationHandle);
    ManagedAllocationHeader* header = GetAllocationHeader(allocation);
    if (header->IsObject())
    {
        ObjectHeader* objectHeader = &header->objectHeader;
        ObjectType* type = objectHeader->GetType();
        utf32_string us = type->Name().Value();
        std::string s = ToUtf8(us);
        std::cout << "object " << s << " : " << GetManagedMemoryPool().GetFieldCount(ObjectReference(allocationHandle.Value())) << std::endl;
    }
    else if (header->IsArrayElements())
    {
        ArrayElementsHeader* arrayElementsHeader = &header->arrayElementsHeader;
        Type* type = arrayElementsHeader->GetElementType();
        std::cout << "array of " << ToUtf8(type->Name().Value()) << " : " << arrayElementsHeader->NumElements() << std::endl;
    }
    else if (header->IsStringCharacters())
    {
        StringCharactersHeader* stringCharsHeader = &header->stringCharactersHeader;
        const char32_t* str = stringCharsHeader->Str();
        int n = stringCharsHeader->NumChars();
        if (n > 0)
        {
            std::cout << "string \"";
            std::string s;
            for (int i = 0; i < n; ++i)
            {
                char32_t c = str[i];
                char d = static_cast<char>(c);
                s.append(1, d);
            }
            std::cout << s << "\"" << std::endl;
        }
    }
}

void Shell::PrintField(int handle, int index)
{
    Frame* frame = machine.MainThread().GetStack().CurrentFrame();
    AllocationHandle allocationHandle(handle);
    void* allocation = GetManagedMemoryPool().GetAllocationNoThrowNoLock(allocationHandle);
    ManagedAllocationHeader* header = GetAllocationHeader(allocation);
    if (header->IsObject())
    {
        IntegralValue value = GetManagedMemoryPool().GetField(ObjectReference(allocationHandle.Value()), index);
        Print(value);
    }
    else if (header->IsArrayElements())
    {
        IntegralValue value = GetElement(allocation, index);
        Print(value);
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

void Shell::SwitchToTextMode()
{
    if (HandlesSetToBinaryMode())
    {
        SetHandleToTextMode(0);
        SetHandleToTextMode(1);
        SetHandleToTextMode(2);
    }
}

void Shell::SwitchToBinaryMode()
{
    if (HandlesSetToBinaryMode())
    {
        SetHandleToBinaryMode(0);
        SetHandleToBinaryMode(1);
        SetHandleToBinaryMode(2);
    }
}

} } // namespace cminor::db
