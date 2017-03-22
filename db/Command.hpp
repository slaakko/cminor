// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_DB_COMMAND_INCLUDED
#define CMINOR_DB_COMMAND_INCLUDED
#include <string>

namespace cminor { namespace db {

class Shell;

class Command
{
public:
    virtual ~Command();
    virtual void Execute(Shell& shell) = 0;
};

class HelpCommand : public Command
{
public:
    void Execute(Shell& shell) override;
};

class StartCommand : public Command
{
public:
    void Execute(Shell& shell) override;
};

class ExitCommand : public Command
{
public:
    void Execute(Shell& shell) override;
};

class StepCommand : public Command
{
public:
    void Execute(Shell& shell) override;
};

class NextCommand : public Command
{
public:
    void Execute(Shell& shell) override;
};

class RunCommand : public Command
{
public:
    void Execute(Shell& shell) override;
};

class LocalCommand : public Command
{
public:
    LocalCommand(int index_);
    void Execute(Shell& shell) override;
private:
    int index;
};

class OperandCommand : public Command
{
public:
    OperandCommand(int index_);
    void Execute(Shell& shell) override;
private:
    int index;
};

class PrevCommand : public Command
{
public:
    void Execute(Shell& shell) override;
};

class AllocationCommand : public Command
{
public:
    AllocationCommand(int handle_);
    void Execute(Shell& shell) override;
private:
    int handle;
};

class FieldCommand : public Command
{
public:
    FieldCommand(int handle_, int index_);
    void Execute(Shell& shell) override;
private:
    int handle;
    int index;
};

class ListCommand : public Command
{
public:
    ListCommand(const std::string& sourceFileName_, int line_);
    void Execute(Shell& shell) override;
    void SetLine(int line_) { line = line_;  }
private:
    std::string sourceFileName;
    int line;
};

class BreakCommand : public Command
{
public:
    BreakCommand(const std::string& sourceFileName_, int line_);
    void Execute(Shell& shell) override;
private:
    std::string sourceFileName;
    int line;
};

class ClearCommand : public Command
{
public:
    ClearCommand(int bp_);
    void Execute(Shell& shell) override;
private:
    int bp;
};

class ShowBreakpointsCommand : public Command
{
public:
    void Execute(Shell& shell) override;
};

class StackCommand : public Command
{
public:
    void Execute(Shell& shell) override;
};

class PrintCommand : public Command
{
public:
    PrintCommand(const std::string& name_);
    void Execute(Shell& shell) override;
private:
    std::string name;
};

} } // namespace cminor::db

#endif // CMINOR_DB_COMMAND_INCLUDED
