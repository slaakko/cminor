// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_DB_COMMAND_INCLUDED
#define CMINOR_DB_COMMAND_INCLUDED

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

class StackCommand : public Command
{
public:
    StackCommand(int index_);
    void Execute(Shell& shell) override;
private:
    int index;
};

class PrevCommand : public Command
{
public:
    void Execute(Shell& shell) override;
};

} } // namespace cminor::db

#endif // CMINOR_DB_COMMAND_INCLUDED
