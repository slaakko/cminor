// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/emitter/Emitter.hpp>
#include <cminor/binder/BoundNodeVisitor.hpp>
#include <cminor/binder/BoundFunction.hpp>
#include <cminor/symbols/FunctionSymbol.hpp>
#include <cminor/machine/Machine.hpp>

namespace cminor { namespace emitter {

using namespace cminor::symbols;

class EmitterVisitor : public BoundNodeVisitor, public Emitter
{
public:
    EmitterVisitor(Machine& machine_);
    void Visit(BoundCompileUnit& boundCompileUnit) override;
    void Visit(BoundFunction& boundFunction) override;
    void Visit(BoundCompoundStatement& boundCompoundStatement) override;
    void Visit(BoundReturnStatement& boundReturnStatement) override;
    void Visit(BoundIfStatement& boundIfStatement) override;
    void Visit(BoundWhileStatement& boundWhileStatement) override;
    void Visit(BoundDoStatement& boundDoStatement) override;
    void Visit(BoundForStatement& boundForStatement) override;
    void Visit(BoundBreakStatement& boundBreakStatement) override;
    void Visit(BoundContinueStatement& boundContinueStatement) override;
    void Visit(BoundConstructionStatement& boundConstructionStatement) override;
    void Visit(BoundAssignmentStatement& boundAssignmentStatement) override;
    void Visit(BoundExpressionStatement& boundExpressionStatement) override;
    void Visit(BoundEmptyStatement& boundEmptyStatement) override;
    void Visit(BoundLiteral& boundLiteral) override;
    void Visit(BoundConstant& boundConstant) override;
    void Visit(BoundLocalVariable& boundLocalVariable) override;
    void Visit(BoundParameter& boundParameter) override;
    void Visit(BoundConversion& boundConversion) override;
    void Visit(BoundFunctionCall& boundFunctionCall) override;
    void SetFirstInstIndex(int32_t index) override;
    int32_t FistInstIndex() const override;
private:
    Machine& machine;
    Function* function;
    int32_t firstInstIndex;
    std::vector<Instruction*>* nextSet;
    std::vector<Instruction*>* trueSet;
    std::vector<Instruction*>* falseSet;
    std::vector<Instruction*>* breakSet;
    std::vector<Instruction*>* continueSet;
    std::vector<BoundCompoundStatement*> blockStack;
    BoundStatement* breakTarget;
    BoundStatement* continueTarget;
    bool genJumpingBoolCode;
    void GenJumpingBoolCode();
    void Backpatch(std::vector<Instruction*>* set, int32_t target);
    void Merge(std::vector<Instruction*>* fromSet, std::vector<Instruction*>* toSet);
    void ExitBlocks(BoundCompoundStatement* targetBlock);
};

EmitterVisitor::EmitterVisitor(Machine& machine_) : 
    machine(machine_), function(nullptr), firstInstIndex(endOfFunction), nextSet(nullptr), trueSet(nullptr), falseSet(nullptr), breakSet(nullptr), continueSet(nullptr), 
    breakTarget(nullptr), continueTarget(nullptr), genJumpingBoolCode(false)
{
}

void EmitterVisitor::Backpatch(std::vector<Instruction*>* set, int32_t target)
{
    Assert(set, "set not set");
    for (Instruction* inst : *set)
    {
        inst->SetTarget(target);
    }
}

void EmitterVisitor::Merge(std::vector<Instruction*>* fromSet, std::vector<Instruction*>* toSet)
{
    Assert(fromSet, "set not set");
    Assert(toSet, "set not set");
    toSet->insert(toSet->end(), fromSet->cbegin(), fromSet->cend());
    fromSet->clear();
}

void EmitterVisitor::ExitBlocks(BoundCompoundStatement* targetBlock)
{
    BoundCompoundStatement* block = nullptr;
    int n = int(blockStack.size());
    if (n > 0)
    {
        block = blockStack[n - 1];
    }
    while (block && block != targetBlock)
    {
        std::unique_ptr<Instruction> exitBlockInst = machine.CreateInst("exitblock");
        function->AddInst(std::move(exitBlockInst));
        --n;
        if (n > 0)
        {
            block = blockStack[n - 1];
        }
        else
        {
            block = nullptr;
        }
    }
}

void EmitterVisitor::SetFirstInstIndex(int32_t index)
{
    firstInstIndex = index;
}

int32_t EmitterVisitor::FistInstIndex() const
{
    return firstInstIndex;
}

void EmitterVisitor::GenJumpingBoolCode()
{
    if (!genJumpingBoolCode) return;
    std::unique_ptr<Instruction> jumpTrue = machine.CreateInst("jumptrue");
    trueSet->push_back(jumpTrue.get());
    function->AddInst(std::move(jumpTrue));
    std::unique_ptr<Instruction> jumpFalse = machine.CreateInst("jump");
    falseSet->push_back(jumpFalse.get());
    function->AddInst(std::move(jumpFalse));
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
    Emitter* prevEmitter = function->GetEmitter();
    function->SetEmitter(this);
    boundFunction.Body()->Accept(*this);
    function->SetEmitter(prevEmitter);
    function = prevFunction;
}

void EmitterVisitor::Visit(BoundCompoundStatement& boundCompoundStatement)
{
    blockStack.push_back(&boundCompoundStatement);
    std::unique_ptr<Instruction> enterBlockInst = machine.CreateInst("enterblock");
    function->AddInst(std::move(enterBlockInst));
    int n = int(boundCompoundStatement.Statements().size());
    std::vector<Instruction*> prevNext;
    for (int i = 0; i < n; ++i)
    {
        std::vector<Instruction*> next;
        std::vector<Instruction*>* prevNextSet = nextSet;
        nextSet = &next;
        BoundStatement* boundStatement = boundCompoundStatement.Statements()[i].get();
        int32_t prevFirstInstIndex = firstInstIndex;
        firstInstIndex = endOfFunction;
        boundStatement->Accept(*this);
        int32_t statementTarget = firstInstIndex;
        firstInstIndex = prevFirstInstIndex;
        if (statementTarget != endOfFunction)
        {
            Backpatch(&prevNext, statementTarget);
            prevNext = std::move(next);
        }
        else
        {
            prevNext.insert(prevNext.end(), next.cbegin(), next.cend());
        }
        nextSet = prevNextSet;
    }
    int32_t prevFirstInstIndex = firstInstIndex;
    firstInstIndex = endOfFunction;
    std::unique_ptr<Instruction> exitBlockInst = machine.CreateInst("exitblock");
    function->AddInst(std::move(exitBlockInst));
    int32_t exitBlockTarget = firstInstIndex;
    firstInstIndex = prevFirstInstIndex;
    Backpatch(&prevNext, exitBlockTarget);
    blockStack.pop_back();
}

void EmitterVisitor::Visit(BoundReturnStatement& boundReturnStatement)
{
    BoundFunctionCall* returnFunctionCall = boundReturnStatement.ReturnFunctionCall();
    if (returnFunctionCall)
    {
        returnFunctionCall->Accept(*this);
    }
    ExitBlocks(nullptr);
    std::unique_ptr<Instruction> inst = machine.CreateInst("jump");
    inst->SetTarget(endOfFunction);
    function->AddInst(std::move(inst));
}

void EmitterVisitor::Visit(BoundIfStatement& boundIfStatement)
{
    std::vector<Instruction*>* prevTrueSet = trueSet;
    std::vector<Instruction*>* prevFalseSet = falseSet;
    std::vector<Instruction*> true_;
    std::vector<Instruction*> false_;
    trueSet = &true_;
    falseSet = &false_;
    bool prevGenJumpingBoolCode = genJumpingBoolCode;
    genJumpingBoolCode = true;
    boundIfStatement.Condition()->Accept(*this);
    genJumpingBoolCode = prevGenJumpingBoolCode;
    int32_t prevFirstInstIndex = firstInstIndex;
    firstInstIndex = endOfFunction;
    boundIfStatement.ThenS()->Accept(*this);
    int32_t thenTarget = firstInstIndex;
    firstInstIndex = prevFirstInstIndex;
    Backpatch(&true_, thenTarget);
    if (boundIfStatement.ElseS())
    {
        std::unique_ptr<Instruction> inst = machine.CreateInst("jump");
        nextSet->push_back(inst.get());
        function->AddInst(std::move(inst));
        prevFirstInstIndex = firstInstIndex;
        firstInstIndex = endOfFunction;
        boundIfStatement.ElseS()->Accept(*this);
        firstInstIndex = prevFirstInstIndex;
    }
    else
    {
        Merge(&false_, nextSet);
    }
    trueSet = prevTrueSet;
    falseSet = prevFalseSet;
}

void EmitterVisitor::Visit(BoundWhileStatement& boundWhileStatement)
{
    BoundStatement* prevBreakTarget = breakTarget;
    breakTarget = &boundWhileStatement;
    BoundStatement* prevContinueTarget = continueTarget;
    continueTarget = &boundWhileStatement;
    std::vector<Instruction*>* prevTrueSet = trueSet;
    std::vector<Instruction*>* prevFalseSet = falseSet;
    std::vector<Instruction*>* prevBreakSet = breakSet;
    std::vector<Instruction*>* prevContinueSet = continueSet;
    std::vector<Instruction*> true_;
    std::vector<Instruction*> false_;
    std::vector<Instruction*> break_;
    std::vector<Instruction*> continue_;
    trueSet = &true_;
    falseSet = &false_;
    breakSet = &break_;
    continueSet = &continue_;
    bool prevGenJumpingBoolCode = genJumpingBoolCode;
    genJumpingBoolCode = true;
    int32_t prevFirstInstIndex = firstInstIndex;
    firstInstIndex = endOfFunction;
    boundWhileStatement.Condition()->Accept(*this);
    int32_t conditionTarget = firstInstIndex;
    firstInstIndex = prevFirstInstIndex;
    genJumpingBoolCode = prevGenJumpingBoolCode;
    prevFirstInstIndex = firstInstIndex;
    firstInstIndex = endOfFunction;
    boundWhileStatement.Statement()->Accept(*this);
    Backpatch(&true_, firstInstIndex);
    firstInstIndex = prevFirstInstIndex;
    std::unique_ptr<Instruction> inst = machine.CreateInst("jump");
    inst->SetTarget(conditionTarget);
    function->AddInst(std::move(inst));
    Backpatch(&continue_, conditionTarget);
    Merge(&false_, nextSet);
    Merge(&break_, nextSet);
    trueSet = prevTrueSet;
    falseSet = prevFalseSet;
    breakSet = prevBreakSet;
    continueSet = prevContinueSet;
    breakTarget = prevBreakTarget;
    continueTarget = prevContinueTarget;
}

void EmitterVisitor::Visit(BoundDoStatement& boundDoStatement)
{
    BoundStatement* prevBreakTarget = breakTarget;
    breakTarget = &boundDoStatement;
    BoundStatement* prevContinueTarget = continueTarget;
    continueTarget = &boundDoStatement;
    std::vector<Instruction*>* prevTrueSet = trueSet;
    std::vector<Instruction*>* prevFalseSet = falseSet;
    std::vector<Instruction*>* prevBreakSet = breakSet;
    std::vector<Instruction*>* prevContinueSet = continueSet;
    std::vector<Instruction*> true_;
    std::vector<Instruction*> false_;
    std::vector<Instruction*> break_;
    std::vector<Instruction*> continue_;
    breakSet = &break_;
    continueSet = &continue_;
    int32_t prevFirstInstIndex = firstInstIndex;
    firstInstIndex = endOfFunction;
    boundDoStatement.Statement()->Accept(*this);
    int32_t statementTarget = firstInstIndex;
    firstInstIndex = prevFirstInstIndex;
    trueSet = &true_;
    falseSet = &false_;
    bool prevGenJumpingBoolCode = genJumpingBoolCode;
    genJumpingBoolCode = true;
    prevFirstInstIndex = firstInstIndex;
    firstInstIndex = endOfFunction;
    boundDoStatement.Condition()->Accept(*this);
    int32_t conditionTarget = firstInstIndex;
    firstInstIndex = prevFirstInstIndex;
    genJumpingBoolCode = prevGenJumpingBoolCode;
    Backpatch(&true_, statementTarget);
    Backpatch(&continue_, conditionTarget);
    Merge(&false_, nextSet);
    Merge(&break_, nextSet);
    trueSet = prevTrueSet;
    falseSet = prevFalseSet;
    breakSet = prevBreakSet;
    continueSet = prevContinueSet;
    breakTarget = prevBreakTarget;
    continueTarget = prevContinueTarget;
}

void EmitterVisitor::Visit(BoundForStatement& boundForStatement)
{
    BoundStatement* prevBreakTarget = breakTarget;
    breakTarget = &boundForStatement;
    BoundStatement* prevContinueTarget = continueTarget;
    continueTarget = &boundForStatement;
    std::vector<Instruction*>* prevTrueSet = trueSet;
    std::vector<Instruction*>* prevFalseSet = falseSet;
    std::vector<Instruction*>* prevBreakSet = breakSet;
    std::vector<Instruction*>* prevContinueSet = continueSet;
    std::vector<Instruction*> true_;
    std::vector<Instruction*> false_;
    std::vector<Instruction*> break_;
    std::vector<Instruction*> continue_;
    boundForStatement.InitS()->Accept(*this);
    trueSet = &true_;
    falseSet = &false_;
    breakSet = &break_;
    continueSet = &continue_;
    bool prevGenJumpingBoolCode = genJumpingBoolCode;
    genJumpingBoolCode = true;
    int32_t prevFirstInstIndex = firstInstIndex;
    firstInstIndex = endOfFunction;
    boundForStatement.Condition()->Accept(*this);
    int32_t conditionTarget = firstInstIndex;
    firstInstIndex = prevFirstInstIndex;
    genJumpingBoolCode = prevGenJumpingBoolCode;
    prevFirstInstIndex = firstInstIndex;
    firstInstIndex = endOfFunction;
    boundForStatement.ActionS()->Accept(*this);
    int32_t actionTarget = firstInstIndex;
    firstInstIndex = prevFirstInstIndex;
    Backpatch(&true_, actionTarget);
    prevFirstInstIndex = firstInstIndex;
    firstInstIndex = endOfFunction;
    boundForStatement.LoopS()->Accept(*this);
    int32_t loopTarget = firstInstIndex;
    firstInstIndex = prevFirstInstIndex;
    Backpatch(&continue_, loopTarget);
    std::unique_ptr<Instruction> jumpToCond = machine.CreateInst("jump");
    jumpToCond->SetTarget(conditionTarget);
    function->AddInst(std::move(jumpToCond));
    Merge(&false_, nextSet);
    Merge(&break_, nextSet);
    trueSet = prevTrueSet;
    falseSet = prevFalseSet;
    breakSet = prevBreakSet;
    continueSet = prevContinueSet;
    breakTarget = prevBreakTarget;
    continueTarget = prevContinueTarget;
}

void EmitterVisitor::Visit(BoundBreakStatement& boundBreakStatement)
{
    Assert(breakSet, "break set not set");
    BoundCompoundStatement* targetBlock = breakTarget->Parent()->Block();
    ExitBlocks(targetBlock);
    std::unique_ptr<Instruction> jump = machine.CreateInst("jump");
    breakSet->push_back(jump.get());
    function->AddInst(std::move(jump));
}

void EmitterVisitor::Visit(BoundContinueStatement& boundContinueStatement)
{
    Assert(continueSet, "containue set not set");
    BoundCompoundStatement* targetBlock = continueTarget->Parent()->Block();
    ExitBlocks(targetBlock);
    std::unique_ptr<Instruction> jump = machine.CreateInst("jump");
    continueSet->push_back(jump.get());
    function->AddInst(std::move(jump));
}

void EmitterVisitor::Visit(BoundConstructionStatement& boundConstructionStatement)
{
    boundConstructionStatement.ConstructorCall()->Accept(*this);
}

void EmitterVisitor::Visit(BoundAssignmentStatement& boundAssignmentStatement)
{
    boundAssignmentStatement.AssignmentCall()->Accept(*this);
}

void EmitterVisitor::Visit(BoundExpressionStatement& boundExpressionStatement)
{
    boundExpressionStatement.Expression()->Accept(*this);
}

void EmitterVisitor::Visit(BoundEmptyStatement& boundEmptyStatement)
{
    std::unique_ptr<Instruction> nop = machine.CreateInst("nop");
    function->AddInst(std::move(nop));
}

void EmitterVisitor::Visit(BoundLiteral& boundLiteral)
{
    boundLiteral.GenLoad(machine, *function);
    GenJumpingBoolCode();
}

void EmitterVisitor::Visit(BoundConstant& boundConstant)
{
    boundConstant.GenLoad(machine, *function);
    GenJumpingBoolCode();
}

void EmitterVisitor::Visit(BoundLocalVariable& boundLocalVariable)
{
    boundLocalVariable.GenLoad(machine, *function);
    GenJumpingBoolCode();
}

void EmitterVisitor::Visit(BoundParameter& boundParameter)
{
    boundParameter.GenLoad(machine, *function);
    GenJumpingBoolCode();
}

void EmitterVisitor::Visit(BoundConversion& boundConversion)
{
    boundConversion.GenLoad(machine, *function);
    GenJumpingBoolCode();
}

void EmitterVisitor::Visit(BoundFunctionCall& boundFunctionCall)
{
    boundFunctionCall.GenLoad(machine, *function);
    GenJumpingBoolCode();
}

void GenerateCode(BoundCompileUnit& boundCompileUnit, Machine& machine)
{
    EmitterVisitor emitterVisitor(machine);
    boundCompileUnit.Accept(emitterVisitor);
}

} } // namespace cminor::emitter
