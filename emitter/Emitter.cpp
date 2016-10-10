// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/emitter/Emitter.hpp>
#include <cminor/binder/BoundNodeVisitor.hpp>
#include <cminor/binder/BoundFunction.hpp>
#include <cminor/symbols/FunctionSymbol.hpp>

namespace cminor { namespace emitter {

using namespace cminor::symbols;

class EmitterVisitor : public BoundNodeVisitor
{
public:
    EmitterVisitor(Machine& machine_);
    void Visit(BoundCompileUnit& boundCompileUnit) override;
    void Visit(BoundFunction& boundFunction) override;
    void Visit(BoundCompoundStatement& boundCompoundStatement) override;
    void Visit(BoundConstructionStatement& boundConstructionStatement) override;
    void Visit(BoundAssignmentStatement& boundAssignmentStatement) override;
    void Visit(BoundLiteral& boundLiteral) override;
    void Visit(BoundConstant& boundConstant) override;
    void Visit(BoundLocalVariable& boundLocalVariable) override;
    void Visit(BoundParameter& boundParameter) override;
    void Visit(BoundConversion& boundConversion) override;
    void Visit(BoundFunctionCall& boundFunctionCall) override;
private:
    Machine& machine;
    Function* function;
};

EmitterVisitor::EmitterVisitor(Machine& machine_) : machine(machine_), function(nullptr)
{
}

void EmitterVisitor::Visit(BoundCompileUnit& boundCompileUnit)
{
    int n = int(boundCompileUnit.BoundNodes().size());
    for (int i = 0; i < n; ++i)
    {
        BoundNode* boundNode = boundCompileUnit.BoundNodes()[i].get();
        boundNode->Accept(*this);
    }
}

void EmitterVisitor::Visit(BoundFunction& boundFunction)
{
    boundFunction.GetFunctionSymbol()->CreateMachineFunction();
    Function* prevFunction = function;
    function = boundFunction.GetFunctionSymbol()->MachineFunction();
    boundFunction.Body()->Accept(*this);
    function = prevFunction;
}

void EmitterVisitor::Visit(BoundCompoundStatement& boundCompoundStatement)
{
    int n = int(boundCompoundStatement.Statements().size());
    for (int i = 0; i < n; ++i)
    {
        BoundStatement* boundStatement = boundCompoundStatement.Statements()[i].get();
        boundStatement->Accept(*this);
    }
}

void EmitterVisitor::Visit(BoundConstructionStatement& boundConstructionStatement)
{
    boundConstructionStatement.ConstructorCall()->Accept(*this);
}

void EmitterVisitor::Visit(BoundAssignmentStatement& boundAssignmentStatement)
{
    boundAssignmentStatement.AssignmentCall()->Accept(*this);
}

void EmitterVisitor::Visit(BoundLiteral& boundLiteral)
{
    boundLiteral.GenLoad(machine, *function);
}

void EmitterVisitor::Visit(BoundConstant& boundConstant)
{
    boundConstant.GenLoad(machine, *function);
}

void EmitterVisitor::Visit(BoundLocalVariable& boundLocalVariable)
{
    boundLocalVariable.GenLoad(machine, *function);
}

void EmitterVisitor::Visit(BoundParameter& boundParameter)
{
    boundParameter.GenLoad(machine, *function);
}

void EmitterVisitor::Visit(BoundConversion& boundConversion)
{
    boundConversion.GenLoad(machine, *function);
}

void EmitterVisitor::Visit(BoundFunctionCall& boundFunctionCall)
{
    boundFunctionCall.GenLoad(machine, *function);
}

void GenerateCode(BoundCompileUnit& boundCompileUnit, Machine& machine)
{
    EmitterVisitor emitterVisitor(machine);
    boundCompileUnit.Accept(emitterVisitor);
}

} } // namespace cminor::emitter
