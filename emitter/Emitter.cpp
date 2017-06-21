// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/emitter/Emitter.hpp>
#include <cminor/binder/BoundNodeVisitor.hpp>
#include <cminor/binder/BoundClass.hpp>
#include <cminor/binder/BoundFunction.hpp>
#include <cminor/symbols/FunctionSymbol.hpp>
#include <cminor/symbols/GlobalFlags.hpp>
#include <cminor/machine/Machine.hpp>

namespace cminor { namespace emitter {

using namespace cminor::symbols;

class EmitterVisitor : public BoundNodeVisitor, public Emitter
{
public:
    EmitterVisitor(Machine& machine_, BoundCompileUnit& boundCompileUnit_);
    void Visit(BoundCompileUnit& boundCompileUnit) override;
    void Visit(BoundClass& boundClass) override;
    void Visit(BoundFunction& boundFunction) override;
    void Visit(BoundCompoundStatement& boundCompoundStatement) override;
    void Visit(BoundReturnStatement& boundReturnStatement) override;
    void Visit(BoundIfStatement& boundIfStatement) override;
    void Visit(BoundWhileStatement& boundWhileStatement) override;
    void Visit(BoundDoStatement& boundDoStatement) override;
    void Visit(BoundForStatement& boundForStatement) override;
    void Visit(BoundSwitchStatement& boundSwitchStatement) override;
    void Visit(BoundCaseStatement& boundCaseStatement) override;
    void Visit(BoundDefaultStatement& boundDefaultStatement) override;
    void Visit(BoundGotoCaseStatement& boundGotoCaseStatement) override;
    void Visit(BoundGotoDefaultStatement& boundGotoDefaultStatement) override;
    void Visit(BoundBreakStatement& boundBreakStatement) override;
    void Visit(BoundContinueStatement& boundContinueStatement) override;
    void Visit(BoundGotoStatement& boundGotoStatement) override;
    void Visit(BoundConstructionStatement& boundConstructionStatement) override;
    void Visit(BoundAssignmentStatement& boundAssignmentStatement) override;
    void Visit(BoundExpressionStatement& boundExpressionStatement) override;
    void Visit(BoundEmptyStatement& boundEmptyStatement) override;
    void Visit(BoundThrowStatement& boundThrowStatement) override;
    void Visit(BoundTryStatement& boundTryStatement) override;
    void Visit(BoundCatchStatement& boundCatchStatement) override;
    void Visit(BoundStaticInitStatement& boundStaticInitStatement) override;
    void Visit(BoundDoneStaticInitStatement& boundDoneStaticInitStatement) override;
    void Visit(BoundLiteral& boundLiteral) override;
    void Visit(BoundConstant& boundConstant) override;
    void Visit(BoundEnumConstant& boundEnumConstant) override;
    void Visit(BoundLocalVariable& boundLocalVariable) override;
    void Visit(BoundMemberVariable& boundMemberVariable) override;
    void Visit(BoundProperty& boundProperty) override;
    void Visit(BoundParameter& boundParameter) override;
    void Visit(BoundConversion& boundConversion) override;
    void Visit(BoundFunctionCall& boundFunctionCall) override;
    void Visit(BoundDelegateCall& boundDelegateCall) override;
    void Visit(BoundClassDelegateCall& boundClassDelegateCall) override;
    void Visit(BoundNewExpression& boundNewExpression) override;
    void Visit(BoundConjunction& boundConjunction) override;
    void Visit(BoundDisjunction& boundDisjunction) override;
    void Visit(BoundIsExpression& boundIsExpression) override;
    void Visit(BoundAsExpression& boundAsExpression) override;
    void Visit(BoundLocalRefExpression& boundLocalRefExpression) override;
    void Visit(BoundDefaultExpression& boundDefaultExpression) override;
    void Visit(GenObject& genObject) override;
    bool CreatePCRange() const override;
    bool SetPCRangeEnd() const override;
    void DoCreatePCRange(int32_t start) override;
    void DoSetPCRangeEnd(int32_t end) override;
    void BackpatchConDisSet(int32_t index) override;
private:
    Machine& machine;
    BoundCompileUnit& boundCompileUnit;
    BoundFunction* boundFunction;
    Function* function;
    std::vector<Instruction*>* nextSet;
    std::vector<Instruction*>* trueSet;
    std::vector<Instruction*>* falseSet;
    std::vector<Instruction*>* breakSet;
    std::vector<Instruction*>* continueSet;
    std::vector<Instruction*>* conDisSet;
    std::vector<BoundCompoundStatement*> blockStack;
    BoundStatement* breakTarget;
    BoundStatement* continueTarget;
    std::vector<std::pair<IntegralValue, int32_t>>* caseTargets;
    std::unordered_map<IntegralValue, Instruction*, IntegralValueHash>* gotoCaseJumps;
    std::vector<Instruction*>* gotoDefaultJumps;
    bool genJumpingBoolCode;
    bool createPCRange;
    bool setPCRangeEnd;
    ExceptionBlock* currentExceptionBlock;
    std::vector<std::unique_ptr<Instruction>> nextInsts;
    uint32_t currentContainerScopeId;
    void GenJumpingBoolCode();
    void Backpatch(std::vector<Instruction*>& set, int32_t target);
    void Merge(std::vector<Instruction*>& fromSet, std::vector<Instruction*>& toSet);
    void ExitBlocks(BoundCompoundStatement* targetBlock);
    void AddNextInst(std::unique_ptr<Instruction>&& nextInst);
    void AddingInst(int32_t pc) override;
};

EmitterVisitor::EmitterVisitor(Machine& machine_, BoundCompileUnit& boundCompileUnit_) : 
    machine(machine_), boundCompileUnit(boundCompileUnit_), boundFunction(nullptr), function(nullptr), nextSet(nullptr), trueSet(nullptr), falseSet(nullptr), breakSet(nullptr), 
    continueSet(nullptr), conDisSet(nullptr), breakTarget(nullptr), continueTarget(nullptr), caseTargets(nullptr), gotoCaseJumps(nullptr), gotoDefaultJumps(nullptr), genJumpingBoolCode(false), 
    createPCRange(false), setPCRangeEnd(false), currentExceptionBlock(nullptr), currentContainerScopeId(-1)
{
}

void EmitterVisitor::Backpatch(std::vector<Instruction*>& set, int32_t target)
{
    for (Instruction* inst : set)
    {
        inst->SetTarget(target);
    }
}

void EmitterVisitor::Merge(std::vector<Instruction*>& fromSet, std::vector<Instruction*>& toSet)
{
    toSet.insert(toSet.end(), fromSet.cbegin(), fromSet.cend());
    fromSet.clear();
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
        if (block->FinallyBlock())
        {
            block->FinallyBlock()->Accept(*this);
        }
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

void EmitterVisitor::AddNextInst(std::unique_ptr<Instruction>&& nextInst)
{
    nextInsts.push_back(std::move(nextInst));
}

void EmitterVisitor::AddingInst(int32_t pc) 
{
    boundFunction->GetFunctionSymbol()->MapPCToContainerScopeId(pc, currentContainerScopeId);
}

bool EmitterVisitor::CreatePCRange() const
{
    return createPCRange;
}

bool EmitterVisitor::SetPCRangeEnd() const
{
    return setPCRangeEnd;
}

void EmitterVisitor::DoCreatePCRange(int32_t start)
{
    PCRange pcRange;
    pcRange.SetStart(start);
    currentExceptionBlock->AddPCRange(pcRange);
    createPCRange = false;
}

void EmitterVisitor::DoSetPCRangeEnd(int32_t end)
{
    PCRange& pcRange = currentExceptionBlock->GetLastPCRange();
    pcRange.SetEnd(end);
}

void EmitterVisitor::BackpatchConDisSet(int32_t index)
{
    if (conDisSet)
    {
        Backpatch(*conDisSet, index);
        conDisSet->clear();
    }
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

void EmitterVisitor::Visit(BoundClass& boundClass)
{
    int n = int(boundClass.Members().size());
    for (int i = 0; i < n; ++i)
    {
        BoundNode* member = boundClass.Members()[i].get();
        member->Accept(*this);
    }
}

void EmitterVisitor::Visit(BoundFunction& boundFunction)
{
    if (boundFunction.GetFunctionSymbol()->IsIntrinsic())
    {
        return;
    }
    boundFunction.GetFunctionSymbol()->CreateMachineFunction();
    Function* prevFunction = function;
    function = boundFunction.GetFunctionSymbol()->MachineFunction();
    if (function->AlreadyGenerated())
    {
        function = prevFunction;
        return;
    }
    Emitter* prevEmitter = function->GetEmitter();
    function->SetEmitter(this);
    this->boundFunction = &boundFunction;
    uint32_t prevContainerScopeId = currentContainerScopeId;
    currentContainerScopeId = boundFunction.GetContainerScope()->NonDefaultId();
    if (boundFunction.Body())
    {
        boundFunction.Body()->Accept(*this);
    }
    if (function->CanThrow())
    {
        boundFunction.GetFunctionSymbol()->SetCanThrow();
    }
    function->SetEmitter(prevEmitter);
    function = prevFunction;
    currentContainerScopeId = prevContainerScopeId;
}

void EmitterVisitor::Visit(BoundCompoundStatement& boundCompoundStatement)
{
    uint32_t prevContainerScopeId = currentContainerScopeId;
    currentContainerScopeId = boundCompoundStatement.GetContainerScope()->NonDefaultId();
    SetCurrentSourceLine(boundCompoundStatement.GetSpan().LineNumber());
    blockStack.push_back(&boundCompoundStatement);
    std::unique_ptr<Instruction> enterBlockInst = machine.CreateInst("enterblock");
    InstIndexRequest enterBlock;
    AddIndexRequest(&enterBlock);
    function->AddInst(std::move(enterBlockInst));
    boundCompoundStatement.SetFirstInstIndex(enterBlock.Index());
    function->MapPCToSourceLine(enterBlock.Index(), boundCompoundStatement.BeginBraceSpan().LineNumber());
    int n = int(boundCompoundStatement.Statements().size());
    std::vector<Instruction*> prevNext;
    for (int i = 0; i < n; ++i)
    {
        std::vector<Instruction*> next;
        std::vector<Instruction*>* prevNextSet = nextSet;
        nextSet = &next;
        BoundStatement* boundStatement = boundCompoundStatement.Statements()[i].get();
        InstIndexRequest startStatement;
        AddIndexRequest(&startStatement);
        boundStatement->Accept(*this);
        function->MapPCToSourceLine(startStatement.Index(), boundStatement->GetSpan().LineNumber());
        int32_t statementTarget = startStatement.Index();
        boundStatement->SetFirstInstIndex(statementTarget);
        if (statementTarget != endOfFunction)
        {
            Backpatch(prevNext, statementTarget);
            prevNext = std::move(next);
        }
        else
        {
            prevNext.insert(prevNext.end(), next.cbegin(), next.cend());
        }
        nextSet = prevNextSet;
    }
    InstIndexRequest exitBlock;
    AddIndexRequest(&exitBlock);
    std::unique_ptr<Instruction> exitBlockInst = machine.CreateInst("exitblock");
    function->AddInst(std::move(exitBlockInst));
    function->MapPCToSourceLine(exitBlock.Index(), boundCompoundStatement.EndBraceSpan().LineNumber());
    int32_t exitBlockTarget = exitBlock.Index();
    Backpatch(prevNext, exitBlockTarget);
    blockStack.pop_back();
    currentContainerScopeId = prevContainerScopeId;
}

void EmitterVisitor::Visit(BoundReturnStatement& boundReturnStatement)
{
    uint32_t prevContainerScopeId = currentContainerScopeId;
    currentContainerScopeId = boundReturnStatement.GetContainerScope()->NonDefaultId();
    SetCurrentSourceLine(boundReturnStatement.GetSpan().LineNumber());
    BoundFunctionCall* returnFunctionCall = boundReturnStatement.ReturnFunctionCall();
    if (returnFunctionCall)
    {
        std::vector<Instruction*>* prevConDisSet = conDisSet;
        std::vector<Instruction*> conDis;
        conDisSet = &conDis;
        returnFunctionCall->Accept(*this);
        conDisSet = prevConDisSet;
    }
    ExitBlocks(nullptr);
    std::unique_ptr<Instruction> inst = machine.CreateInst("jump");
    inst->SetTarget(endOfFunction);
    function->AddInst(std::move(inst));
    currentContainerScopeId = prevContainerScopeId;
}

void EmitterVisitor::Visit(BoundIfStatement& boundIfStatement)
{
    uint32_t prevContainerScopeId = currentContainerScopeId;
    currentContainerScopeId = boundIfStatement.GetContainerScope()->NonDefaultId();
    SetCurrentSourceLine(boundIfStatement.GetSpan().LineNumber());
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
    InstIndexRequest startThen;
    AddIndexRequest(&startThen);
    boundIfStatement.ThenS()->Accept(*this);
    function->MapPCToSourceLine(startThen.Index(), boundIfStatement.ThenS()->GetSpan().LineNumber());
    int32_t thenTarget = startThen.Index();
    boundIfStatement.ThenS()->SetFirstInstIndex(thenTarget);
    Backpatch(true_, thenTarget);
    if (boundIfStatement.ElseS())
    {
        std::unique_ptr<Instruction> inst = machine.CreateInst("jump");
        nextSet->push_back(inst.get());
        function->AddInst(std::move(inst));
        InstIndexRequest startElse;
        AddIndexRequest(&startElse);
        boundIfStatement.ElseS()->Accept(*this);
        function->MapPCToSourceLine(startElse.Index(), boundIfStatement.ElseS()->GetSpan().LineNumber());
        int32_t elseTarget = startElse.Index();
        boundIfStatement.ElseS()->SetFirstInstIndex(elseTarget);
        Backpatch(false_, elseTarget);
    }
    else
    {
        Assert(nextSet, "next set not set");
        Merge(false_, *nextSet);
    }
    trueSet = prevTrueSet;
    falseSet = prevFalseSet;
    currentContainerScopeId = prevContainerScopeId;
}

void EmitterVisitor::Visit(BoundWhileStatement& boundWhileStatement)
{
    uint32_t prevContainerScopeId = currentContainerScopeId;
    currentContainerScopeId = boundWhileStatement.GetContainerScope()->NonDefaultId();
    SetCurrentSourceLine(boundWhileStatement.GetSpan().LineNumber());
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
    InstIndexRequest startCond;
    AddIndexRequest(&startCond);
    boundWhileStatement.Condition()->Accept(*this);
    function->MapPCToSourceLine(startCond.Index(), boundWhileStatement.GetSpan().LineNumber());
    int32_t conditionTarget = startCond.Index();
    genJumpingBoolCode = prevGenJumpingBoolCode;
    InstIndexRequest startStatement;
    AddIndexRequest(&startStatement);
    std::vector<Instruction*> statementNext;
    std::vector<Instruction*>* prevNextSet = nextSet;
    nextSet = &statementNext;
    boundWhileStatement.Statement()->Accept(*this);
    function->MapPCToSourceLine(startStatement.Index(), boundWhileStatement.Statement()->GetSpan().LineNumber());
    boundWhileStatement.Statement()->SetFirstInstIndex(startStatement.Index());
    Backpatch(true_, startStatement.Index());
    nextSet = prevNextSet;
    InstIndexRequest gcPoint;
    AddIndexRequest(&gcPoint);
    std::unique_ptr<Instruction> gcpoll = machine.CreateInst("gcpoll");
    function->AddInst(std::move(gcpoll));
    Backpatch(statementNext, gcPoint.Index());
    std::unique_ptr<Instruction> inst = machine.CreateInst("jump");
    inst->SetTarget(conditionTarget);
    function->AddInst(std::move(inst));
    Backpatch(continue_, conditionTarget);
    Assert(nextSet, "next set not set");
    Merge(false_, *nextSet);
    Merge(break_, *nextSet);
    trueSet = prevTrueSet;
    falseSet = prevFalseSet;
    breakSet = prevBreakSet;
    continueSet = prevContinueSet;
    breakTarget = prevBreakTarget;
    continueTarget = prevContinueTarget;
    currentContainerScopeId = prevContainerScopeId;
}

void EmitterVisitor::Visit(BoundDoStatement& boundDoStatement)
{
    uint32_t prevContainerScopeId = currentContainerScopeId;
    currentContainerScopeId = boundDoStatement.GetContainerScope()->NonDefaultId();
    SetCurrentSourceLine(boundDoStatement.GetSpan().LineNumber());
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
    std::vector<Instruction*> statementNext;
    std::vector<Instruction*>* prevNextSet = nextSet;
    nextSet = &statementNext;
    InstIndexRequest startStatement;
    AddIndexRequest(&startStatement);
    boundDoStatement.Statement()->Accept(*this);
    function->MapPCToSourceLine(startStatement.Index(), boundDoStatement.GetSpan().LineNumber());
    int32_t statementTarget = startStatement.Index();
    boundDoStatement.Statement()->SetFirstInstIndex(statementTarget);
    nextSet = prevNextSet;
    trueSet = &true_;
    falseSet = &false_;
    bool prevGenJumpingBoolCode = genJumpingBoolCode;
    genJumpingBoolCode = true;
    InstIndexRequest startCond;
    AddIndexRequest(&startCond);
    std::unique_ptr<Instruction> gcpoll = machine.CreateInst("gcpoll");
    function->AddInst(std::move(gcpoll));
    boundDoStatement.Condition()->Accept(*this);
    int32_t conditionTarget = startCond.Index();
    Backpatch(statementNext, conditionTarget);
    genJumpingBoolCode = prevGenJumpingBoolCode;
    Backpatch(true_, statementTarget);
    Backpatch(continue_, conditionTarget);
    Assert(nextSet, "next set not set");
    Merge(false_, *nextSet);
    Merge(break_, *nextSet);
    trueSet = prevTrueSet;
    falseSet = prevFalseSet;
    breakSet = prevBreakSet;
    continueSet = prevContinueSet;
    breakTarget = prevBreakTarget;
    continueTarget = prevContinueTarget;
    currentContainerScopeId = prevContainerScopeId;
}

void EmitterVisitor::Visit(BoundForStatement& boundForStatement)
{
    uint32_t prevContainerScopeId = currentContainerScopeId;
    currentContainerScopeId = boundForStatement.GetContainerScope()->NonDefaultId();
    SetCurrentSourceLine(boundForStatement.GetSpan().LineNumber());
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
    InstIndexRequest startInitS;
    AddIndexRequest(&startInitS);
    boundForStatement.InitS()->Accept(*this);
    function->MapPCToSourceLine(startInitS.Index(), boundForStatement.GetSpan().LineNumber());
    boundForStatement.InitS()->SetFirstInstIndex(startInitS.Index());
    trueSet = &true_;
    falseSet = &false_;
    breakSet = &break_;
    continueSet = &continue_;
    bool prevGenJumpingBoolCode = genJumpingBoolCode;
    genJumpingBoolCode = true;
    InstIndexRequest startCond;
    AddIndexRequest(&startCond);
    boundForStatement.Condition()->Accept(*this);
    int32_t conditionTarget = startCond.Index();
    genJumpingBoolCode = prevGenJumpingBoolCode;
    std::vector<Instruction*> actionNext;
    std::vector<Instruction*>* prevNextSet = nextSet;
    nextSet = &actionNext;
    InstIndexRequest startAction;
    AddIndexRequest(&startAction);
    boundForStatement.ActionS()->Accept(*this);
    function->MapPCToSourceLine(startAction.Index(), boundForStatement.ActionS()->GetSpan().LineNumber());
    int32_t actionTarget = startAction.Index();
    boundForStatement.ActionS()->SetFirstInstIndex(actionTarget);
    Backpatch(true_, actionTarget);
    nextSet = prevNextSet;
    std::vector<Instruction*> loopNext;
    prevNextSet = nextSet;
    nextSet = &loopNext;
    InstIndexRequest startLoop;
    AddIndexRequest(&startLoop);
    boundForStatement.LoopS()->Accept(*this);
    int32_t loopTarget = startLoop.Index();
    boundForStatement.LoopS()->SetFirstInstIndex(loopTarget);
    Backpatch(continue_, loopTarget);
    nextSet = prevNextSet;
    Backpatch(actionNext, loopTarget);
    InstIndexRequest gcPoint;
    AddIndexRequest(&gcPoint);
    std::unique_ptr<Instruction> gcpoll = machine.CreateInst("gcpoll");
    function->AddInst(std::move(gcpoll));
    Backpatch(loopNext, gcPoint.Index());
    std::unique_ptr<Instruction> jumpToCond = machine.CreateInst("jump");
    jumpToCond->SetTarget(conditionTarget);
    function->AddInst(std::move(jumpToCond));
    Assert(nextSet, "next set not set");
    Merge(false_, *nextSet);
    Merge(break_, *nextSet);
    trueSet = prevTrueSet;
    falseSet = prevFalseSet;
    breakSet = prevBreakSet;
    continueSet = prevContinueSet;
    breakTarget = prevBreakTarget;
    continueTarget = prevContinueTarget;
    currentContainerScopeId = prevContainerScopeId;
}

void EmitterVisitor::Visit(BoundSwitchStatement& boundSwitchStatement)
{
    uint32_t prevContainerScopeId = currentContainerScopeId;
    currentContainerScopeId = boundSwitchStatement.GetContainerScope()->NonDefaultId();
    SetCurrentSourceLine(boundSwitchStatement.GetSpan().LineNumber());
    BoundStatement* prevBreakTarget = breakTarget;
    breakTarget = &boundSwitchStatement;
    std::vector<Instruction*>* prevBreakSet = breakSet;
    std::vector<Instruction*> break_;
    breakSet = &break_;
    InstIndexRequest startCond;
    AddIndexRequest(&startCond);
    boundSwitchStatement.Condition()->Accept(*this);
    function->MapPCToSourceLine(startCond.Index(), boundSwitchStatement.GetSpan().LineNumber());
    std::unique_ptr<Instruction> inst = machine.CreateInst("cswitch");
    ContinuousSwitchInst* switchInst = dynamic_cast<ContinuousSwitchInst*>(inst.get());
    switchInst->SetCondType(boundSwitchStatement.Condition()->GetType()->GetValueType());
    int instIndex = function->NumInsts();
    function->AddInst(std::move(inst));
    std::vector<std::pair<IntegralValue, int32_t>>* prevCaseTargets = caseTargets;
    std::vector<std::pair<IntegralValue, int32_t>> caseTargets_;
    std::unordered_map<IntegralValue, Instruction*, IntegralValueHash>* prevGotoCaseJumps = gotoCaseJumps;
    std::unordered_map<IntegralValue, Instruction*, IntegralValueHash> gotoCaseJumps_;
    std::vector<Instruction*>* prevGotoDefaultJumps = gotoDefaultJumps;
    std::vector<Instruction*> gotoDefaultJumps_;
    gotoDefaultJumps = &gotoDefaultJumps_;
    gotoCaseJumps = &gotoCaseJumps_;
    caseTargets = &caseTargets_;
    for (const std::unique_ptr<BoundCaseStatement>& caseStatement : boundSwitchStatement.CaseStatements())
    {
        caseStatement->Accept(*this);
    }
    InstIndexRequest defaultStart;
    if (boundSwitchStatement.DefaultStatement())
    {
        AddIndexRequest(&defaultStart);
        boundSwitchStatement.DefaultStatement()->Accept(*this);
        switchInst->SetDefaultTarget(defaultStart.Index());
        Backpatch(gotoDefaultJumps_, defaultStart.Index());
    }
    for (const std::pair<IntegralValue, Instruction*>& gotoCaseJump : gotoCaseJumps_)
    {
        IntegralValue value = gotoCaseJump.first;
        bool found = false;
        for (const std::pair<IntegralValue, int32_t>& case_ : caseTargets_)
        {
            if (case_.first == value)
            {
                gotoCaseJump.second->SetTarget(case_.second);
                found = true;
                break;
            }
        }
        if (!found)
        {
            throw std::runtime_error("goto case target not found");
        }
    }
    for (Instruction* gotoDefaultJump : gotoDefaultJumps_)
    {
        if (defaultStart.Index() == -1)
        {
            throw std::runtime_error("default target not found");
        }
        gotoDefaultJump->SetTarget(defaultStart.Index());
    }
    gotoCaseJumps = prevGotoCaseJumps;
    gotoDefaultJumps = prevGotoDefaultJumps;
    bool instReplaced = false;
    BinarySearchSwitchInst* binSearchSwitchInst = nullptr;
    if (!caseTargets_.empty())
    {
        std::sort(caseTargets_.begin(), caseTargets_.end(), IntegralValueLess());
        IntegralValue begin = caseTargets_.front().first;
        IntegralValue end = caseTargets_.back().first;
        switchInst->SetBegin(begin);
        switchInst->SetEnd(end);
        uint64_t n = end.Value() - begin.Value() + 1;
        if (n <= 64)
        {
            std::unordered_set<int> indexSet;
            for (uint64_t i = 0; i < n; ++i)
            {
                switchInst->AddTarget(defaultStart.Index());
                indexSet.insert(int(i));
            }
            for (const std::pair<IntegralValue, int32_t>& p : caseTargets_)
            {
                uint64_t index = p.first.Value() - begin.Value();
                switchInst->SetTarget(int(index), p.second);
                indexSet.erase(int(index));
            }
            if (defaultStart.Index() == -1)
            {
                for (int index : indexSet)
                {
                    switchInst->AddNextTargetIndex(index);
                }
            }
        }
        else
        {
            inst = std::move(machine.CreateInst("bswitch"));
            binSearchSwitchInst = dynamic_cast<BinarySearchSwitchInst*>(inst.get());
            binSearchSwitchInst->SetCondType(boundSwitchStatement.Condition()->GetType()->GetValueType());
            binSearchSwitchInst->SetTargets(caseTargets_);
            binSearchSwitchInst->SetDefaultTarget(defaultStart.Index());
            function->SetInst(instIndex, std::move(inst));
            instReplaced = true;
        }
    }
    if (instReplaced)
    {
        nextSet->push_back(binSearchSwitchInst);
    }
    else
    {
        nextSet->push_back(switchInst);
    }
    caseTargets = prevCaseTargets;
    Merge(break_, *nextSet);
    breakSet = prevBreakSet;
    breakTarget = prevBreakTarget;
    currentContainerScopeId = prevContainerScopeId;
}

void EmitterVisitor::Visit(BoundCaseStatement& boundCaseStatement)
{
    uint32_t prevContainerScopeId = currentContainerScopeId;
    currentContainerScopeId = boundCaseStatement.GetContainerScope()->NonDefaultId();
    SetCurrentSourceLine(boundCaseStatement.GetSpan().LineNumber());
    InstIndexRequest startCase;
    AddIndexRequest(&startCase);
    boundCaseStatement.CompoundStatement()->Accept(*this);
    function->MapPCToSourceLine(startCase.Index(), boundCaseStatement.GetSpan().LineNumber());
    int32_t caseTarget = startCase.Index();
    boundCaseStatement.SetFirstInstIndex(caseTarget);
    for (IntegralValue caseValue : boundCaseStatement.CaseValues())
    {
        caseTargets->push_back(std::make_pair(caseValue, caseTarget));
    }
    currentContainerScopeId = prevContainerScopeId;
}

void EmitterVisitor::Visit(BoundDefaultStatement& boundDefaultStatement)
{
    uint32_t prevContainerScopeId = currentContainerScopeId;
    currentContainerScopeId = boundDefaultStatement.GetContainerScope()->NonDefaultId();
    SetCurrentSourceLine(boundDefaultStatement.GetSpan().LineNumber());
    InstIndexRequest startDefault;
    AddIndexRequest(&startDefault);
    boundDefaultStatement.CompoundStatement()->Accept(*this);
    function->MapPCToSourceLine(startDefault.Index(), boundDefaultStatement.GetSpan().LineNumber());
    boundDefaultStatement.SetFirstInstIndex(startDefault.Index());
    currentContainerScopeId = prevContainerScopeId;
}

void EmitterVisitor::Visit(BoundGotoCaseStatement& boundGotoCaseStatement)
{
    uint32_t prevContainerScopeId = currentContainerScopeId;
    currentContainerScopeId = boundGotoCaseStatement.GetContainerScope()->NonDefaultId();
    SetCurrentSourceLine(boundGotoCaseStatement.GetSpan().LineNumber());
    ExitBlocks(breakTarget->Block());
    if (breakTarget->FirstInstIndex() != -1 && breakTarget->FirstInstIndex() < function->NumInsts())
    {
        std::unique_ptr<Instruction> gcpoll = machine.CreateInst("gcpoll");
        function->AddInst(std::move(gcpoll));
    }
    std::unique_ptr<Instruction> jump = machine.CreateInst("jump");
    Instruction* gotoCaseJump = jump.get();
    function->AddInst(std::move(jump));
    gotoCaseJumps->insert(std::make_pair(boundGotoCaseStatement.CaseValue(), gotoCaseJump));
    currentContainerScopeId = prevContainerScopeId;
}

void EmitterVisitor::Visit(BoundGotoDefaultStatement& boundGotoDefaultStatement)
{
    uint32_t prevContainerScopeId = currentContainerScopeId;
    currentContainerScopeId = boundGotoDefaultStatement.GetContainerScope()->NonDefaultId();
    SetCurrentSourceLine(boundGotoDefaultStatement.GetSpan().LineNumber());
    ExitBlocks(breakTarget->Block());
    if (breakTarget->FirstInstIndex() != -1 && breakTarget->FirstInstIndex() < function->NumInsts())
    {
        std::unique_ptr<Instruction> gcpoll = machine.CreateInst("gcpoll");
        function->AddInst(std::move(gcpoll));
    }
    std::unique_ptr<Instruction> jump = machine.CreateInst("jump");
    Instruction* gotoDefaultJump = jump.get();
    function->AddInst(std::move(jump));
    gotoDefaultJumps->push_back(gotoDefaultJump);
    currentContainerScopeId = prevContainerScopeId;
}

void EmitterVisitor::Visit(BoundBreakStatement& boundBreakStatement)
{
    uint32_t prevContainerScopeId = currentContainerScopeId;
    currentContainerScopeId = boundBreakStatement.GetContainerScope()->NonDefaultId();
    SetCurrentSourceLine(boundBreakStatement.GetSpan().LineNumber());
    Assert(breakSet, "break set not set");
    BoundCompoundStatement* targetBlock = breakTarget->Parent()->Block();
    ExitBlocks(targetBlock);
    std::unique_ptr<Instruction> jump = machine.CreateInst("jump");
    breakSet->push_back(jump.get());
    function->AddInst(std::move(jump));
    currentContainerScopeId = prevContainerScopeId;
}

void EmitterVisitor::Visit(BoundContinueStatement& boundContinueStatement)
{
    uint32_t prevContainerScopeId = currentContainerScopeId;
    currentContainerScopeId = boundContinueStatement.GetContainerScope()->NonDefaultId();
    SetCurrentSourceLine(boundContinueStatement.GetSpan().LineNumber());
    Assert(continueSet, "continue set not set");
    BoundCompoundStatement* targetBlock = continueTarget->Parent()->Block();
    ExitBlocks(targetBlock);
    std::unique_ptr<Instruction> jump = machine.CreateInst("jump");
    continueSet->push_back(jump.get());
    function->AddInst(std::move(jump));
    currentContainerScopeId = prevContainerScopeId;
}

void EmitterVisitor::Visit(BoundGotoStatement& boundGotoStatement)
{
    uint32_t prevContainerScopeId = currentContainerScopeId;
    currentContainerScopeId = boundGotoStatement.GetContainerScope()->NonDefaultId();
    SetCurrentSourceLine(boundGotoStatement.GetSpan().LineNumber());
    BoundCompoundStatement* targetBlock = boundGotoStatement.TargetBlock();
    ExitBlocks(targetBlock);
    if (boundGotoStatement.TargetStatement()->FirstInstIndex() != -1 && boundGotoStatement.TargetStatement()->FirstInstIndex() < function->NumInsts())
    {
        std::unique_ptr<Instruction> gcpoll = machine.CreateInst("gcpoll");
        function->AddInst(std::move(gcpoll));
    }
    std::unique_ptr<Instruction> jump = machine.CreateInst("jump");
    if (boundGotoStatement.TargetStatement()->FirstInstIndex() != -1)
    {
        jump->SetTarget(boundGotoStatement.TargetStatement()->FirstInstIndex());
    }
    else
    {
        boundGotoStatement.TargetStatement()->AddJumpToThis(jump.get());
    }
    function->AddInst(std::move(jump));
    currentContainerScopeId = prevContainerScopeId;
}

void EmitterVisitor::Visit(BoundConstructionStatement& boundConstructionStatement)
{
    uint32_t prevContainerScopeId = currentContainerScopeId;
    currentContainerScopeId = boundConstructionStatement.GetContainerScope()->NonDefaultId();
    SetCurrentSourceLine(boundConstructionStatement.GetSpan().LineNumber());
    std::vector<Instruction*>* prevConDisSet = conDisSet;
    std::vector<Instruction*> conDis;
    conDisSet = &conDis;
    boundConstructionStatement.ConstructorCall()->Accept(*this);
    conDisSet = prevConDisSet;
    currentContainerScopeId = prevContainerScopeId;
}

void EmitterVisitor::Visit(BoundAssignmentStatement& boundAssignmentStatement)
{
    uint32_t prevContainerScopeId = currentContainerScopeId;
    currentContainerScopeId = boundAssignmentStatement.GetContainerScope()->NonDefaultId();
    SetCurrentSourceLine(boundAssignmentStatement.GetSpan().LineNumber());
    std::vector<Instruction*>* prevConDisSet = conDisSet;
    std::vector<Instruction*> conDis;
    conDisSet = &conDis;
    boundAssignmentStatement.AssignmentCall()->Accept(*this);
    conDisSet = prevConDisSet;
    currentContainerScopeId = prevContainerScopeId;
}

void EmitterVisitor::Visit(BoundExpressionStatement& boundExpressionStatement)
{
    uint32_t prevContainerScopeId = currentContainerScopeId;
    currentContainerScopeId = boundExpressionStatement.GetContainerScope()->NonDefaultId();
    SetCurrentSourceLine(boundExpressionStatement.GetSpan().LineNumber());
    std::vector<Instruction*>* prevConDisSet = conDisSet;
    std::vector<Instruction*> conDis;
    conDisSet = &conDis;
    boundExpressionStatement.Expression()->Accept(*this);
    conDisSet = prevConDisSet;
    if (boundExpressionStatement.Expression()->ReturnsValue())
    {
        function->AddInst(machine.CreateInst("pop"));
    }
    currentContainerScopeId = prevContainerScopeId;
}

void EmitterVisitor::Visit(BoundEmptyStatement& boundEmptyStatement)
{
    uint32_t prevContainerScopeId = currentContainerScopeId;
    currentContainerScopeId = boundEmptyStatement.GetContainerScope()->NonDefaultId();
    SetCurrentSourceLine(boundEmptyStatement.GetSpan().LineNumber());
    std::unique_ptr<Instruction> nop = machine.CreateInst("nop");
    function->AddInst(std::move(nop));
    currentContainerScopeId = prevContainerScopeId;
}

void EmitterVisitor::Visit(BoundThrowStatement& boundThrowStatement)
{
    uint32_t prevContainerScopeId = currentContainerScopeId;
    currentContainerScopeId = boundThrowStatement.GetContainerScope()->NonDefaultId();
    SetCurrentSourceLine(boundThrowStatement.GetSpan().LineNumber());
    if (boundThrowStatement.Expression())
    {
        boundThrowStatement.Expression()->Accept(*this);
        std::unique_ptr<Instruction> throwInst = machine.CreateInst("throw");
        InstIndexRequest startThrow;
        AddIndexRequest(&startThrow);
        function->AddInst(std::move(throwInst));
        function->SetCanThrow();
        function->MapPCToSourceLine(startThrow.Index(), CurrentSourceLine());
    }
    else
    {
        std::unique_ptr<Instruction> rethrowInst = machine.CreateInst("rethrow");
        InstIndexRequest startRethrow;
        AddIndexRequest(&startRethrow);
        function->AddInst(std::move(rethrowInst));
        function->MapPCToSourceLine(startRethrow.Index(), CurrentSourceLine());
    }
    currentContainerScopeId = prevContainerScopeId;
}

void EmitterVisitor::Visit(BoundTryStatement& boundTryStatement)
{
    uint32_t prevContainerScopeId = currentContainerScopeId;
    currentContainerScopeId = boundTryStatement.GetContainerScope()->NonDefaultId();
    SetCurrentSourceLine(boundTryStatement.GetSpan().LineNumber());
    InstIndexRequest startTry;
    AddIndexRequest(&startTry);
    std::unique_ptr<ExceptionBlock> exceptionBlock(new ExceptionBlock(function->GetNextExeptionBlockId()));
    ExceptionBlock* parentExceptionBlock = currentExceptionBlock;
    if (parentExceptionBlock)
    {
        exceptionBlock->SetParentId(parentExceptionBlock->Id());
    }
    currentExceptionBlock = exceptionBlock.get();
    function->AddExceptionBlock(std::move(exceptionBlock));
    bool prevCreatePCRange = createPCRange;
    bool prevSetPCRangeEnd = setPCRangeEnd;
    createPCRange = true;
    setPCRangeEnd = true;
    std::unique_ptr<Instruction> beginTryInst = machine.CreateInst("begintry");
    beginTryInst->SetIndex(currentExceptionBlock->Id());
    function->AddInst(std::move(beginTryInst));
    function->MapPCToSourceLine(startTry.Index(), CurrentSourceLine());
    std::vector<Instruction*> nextJumps;
    if (boundTryStatement.FinallyBlock())
    {
        boundTryStatement.TryBlock()->SetFinallyBlock(boundTryStatement.FinallyBlock());
    }
    boundTryStatement.TryBlock()->Accept(*this);
    std::unique_ptr<Instruction> endTryInst = machine.CreateInst("endtry");
    endTryInst->SetIndex(currentExceptionBlock->Id());
    function->AddInst(std::move(endTryInst));
    setPCRangeEnd = prevSetPCRangeEnd;
    createPCRange = prevCreatePCRange;
    if (boundTryStatement.FinallyBlock())
    {
        boundTryStatement.FinallyBlock()->Accept(*this);
    }
    std::unique_ptr<Instruction> jumpInst = machine.CreateInst("jump");
    nextJumps.push_back(jumpInst.get());
    function->AddInst(std::move(jumpInst));
    std::unique_ptr<Instruction> nextInst = machine.CreateInst("next");
    NextInst* next = static_cast<NextInst*>(nextInst.get());
    next->SetExceptionBlock(currentExceptionBlock);
    AddNextInst(std::move(nextInst));
    int n = int(boundTryStatement.Catches().size());
    if (n > 0)
    {
        std::unique_ptr<Instruction> beginCatchSectionInst = machine.CreateInst("begincatchsection");
        beginCatchSectionInst->SetIndex(currentExceptionBlock->Id());
        function->AddInst(std::move(beginCatchSectionInst));
    }
    for (int i = 0; i < n; ++i)
    {
        const std::unique_ptr<BoundCatchStatement>& catchStatement = boundTryStatement.Catches()[i];
        catchStatement->Accept(*this);
        if (boundTryStatement.FinallyBlock())
        {
            boundTryStatement.FinallyBlock()->Accept(*this);
        }
        std::unique_ptr<Instruction> endCatchInst = machine.CreateInst("endcatch");
        endCatchInst->SetIndex(catchStatement->CatchBlockId());
        function->AddInst(std::move(endCatchInst));
        std::unique_ptr<Instruction> jumpInst = machine.CreateInst("jump");
        nextJumps.push_back(jumpInst.get());
        function->AddInst(std::move(jumpInst));
    }
    if (n > 0)
    {
        std::unique_ptr<Instruction> endCatchSectionInst = machine.CreateInst("endcatchsection");
        endCatchSectionInst->SetIndex(currentExceptionBlock->Id());
        function->AddInst(std::move(endCatchSectionInst));
        std::unique_ptr<Instruction> jumpInst = machine.CreateInst("jump");
        nextJumps.push_back(jumpInst.get());
        function->AddInst(std::move(jumpInst));
    }
    if (boundTryStatement.FinallyBlock())
    {
        InstIndexRequest startFinally;
        AddIndexRequest(&startFinally);
        std::unique_ptr<Instruction> beginFinallyInst = machine.CreateInst("beginfinally");
        beginFinallyInst->SetIndex(currentExceptionBlock->Id());
        function->AddInst(std::move(beginFinallyInst));
        currentExceptionBlock->SetFinallyStart(startFinally.Index());
        boundTryStatement.FinallyBlock()->Accept(*this);
        std::unique_ptr<Instruction> endFinallyInst = machine.CreateInst("endfinally");
        endFinallyInst->SetIndex(currentExceptionBlock->Id());
        function->AddInst(std::move(endFinallyInst));
        std::unique_ptr<Instruction> jumpInst = machine.CreateInst("jump");
        nextJumps.push_back(jumpInst.get());
        function->AddInst(std::move(jumpInst));
    }
    for (Instruction* nextJump : nextJumps)
    {
        nextSet->push_back(nextJump);
    }
    nextSet->push_back(next);
    currentExceptionBlock = parentExceptionBlock;
    if (parentExceptionBlock)
    {
        createPCRange = true;
        setPCRangeEnd = true;
    }
    else
    {
        createPCRange = false;
        setPCRangeEnd = false;
    }
    currentContainerScopeId = prevContainerScopeId;
}

void EmitterVisitor::Visit(BoundCatchStatement& boundCatchStatement)
{
    uint32_t prevContainerScopeId = currentContainerScopeId;
    currentContainerScopeId = boundCatchStatement.GetContainerScope()->NonDefaultId();
    SetCurrentSourceLine(boundCatchStatement.GetSpan().LineNumber());
    std::unique_ptr<CatchBlock> catchBlock(new CatchBlock(currentExceptionBlock->GetNextCatchBlockId()));
    boundCatchStatement.SetCatchBlockId(catchBlock->Id());
    CatchBlock* catchBlockPtr = catchBlock.get();
    currentExceptionBlock->AddCatchBlock(std::move(catchBlock));
    LocalVariableSymbol* exceptionVar = boundCatchStatement.ExceptionVar();
    TypeSymbol* exceptionVarType = exceptionVar->GetType();
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    std::u32string fullClassName = exceptionVarType->FullName();
    Constant exceptionVarClassTypeFullName = constantPool.GetConstant(constantPool.Install(StringPtr(fullClassName.c_str())));
    catchBlockPtr->SetExceptionVarClassTypeFullName(exceptionVarClassTypeFullName);
    catchBlockPtr->SetExceptionVarIndex(exceptionVar->Index());
    InstIndexRequest startCatch;
    AddIndexRequest(&startCatch);
    std::unique_ptr<Instruction> beginCatchInst = machine.CreateInst("begincatch");
    beginCatchInst->SetIndex(catchBlockPtr->Id());
    function->AddInst(std::move(beginCatchInst));
    function->MapPCToSourceLine(startCatch.Index(), boundCatchStatement.GetSpan().LineNumber());
    boundCatchStatement.CatchBlock()->Accept(*this);
    int32_t catchBlockStart = startCatch.Index();
    catchBlockPtr->SetCatchBlockStart(catchBlockStart);
    currentContainerScopeId = prevContainerScopeId;
}

void EmitterVisitor::Visit(BoundStaticInitStatement& boundStaticInitStatement)
{
    uint32_t prevContainerScopeId = currentContainerScopeId;
    currentContainerScopeId = boundStaticInitStatement.GetContainerScope()->NonDefaultId();
    SetCurrentSourceLine(boundStaticInitStatement.GetSpan().LineNumber());
    std::unique_ptr<Instruction> inst = machine.CreateInst("staticinit");
    StaticInitInst* staticInitInst = dynamic_cast<StaticInitInst*>(inst.get());
    Assert(staticInitInst, "static init instruction expected");
    staticInitInst->SetTypeName(boundStaticInitStatement.ClassNameConstant());
    function->AddInst(std::move(inst));
    function->SetCanThrow();
    currentContainerScopeId = prevContainerScopeId;
}

void EmitterVisitor::Visit(BoundDoneStaticInitStatement& boundDoneStaticInitStatement)
{
    uint32_t prevContainerScopeId = currentContainerScopeId;
    currentContainerScopeId = boundDoneStaticInitStatement.GetContainerScope()->NonDefaultId();
    SetCurrentSourceLine(boundDoneStaticInitStatement.GetSpan().LineNumber());
    std::unique_ptr<Instruction> inst = machine.CreateInst("donestaticinit");
    DoneStaticInitInst* doneStaticInitInst = dynamic_cast<DoneStaticInitInst*>(inst.get());
    Assert(doneStaticInitInst, "done static init instruction expected");
    doneStaticInitInst->SetTypeName(boundDoneStaticInitStatement.ClassNameConstant());
    function->AddInst(std::move(inst));
    function->SetCanThrow();
    currentContainerScopeId = prevContainerScopeId;
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

void EmitterVisitor::Visit(BoundEnumConstant& boundEnumConstant)
{
    boundEnumConstant.GenLoad(machine, *function);
    GenJumpingBoolCode();
}

void EmitterVisitor::Visit(BoundLocalVariable& boundLocalVariable)
{
    boundLocalVariable.GenLoad(machine, *function);
    GenJumpingBoolCode();
}

void EmitterVisitor::Visit(BoundMemberVariable& boundMemberVariable)
{
    boundMemberVariable.GenLoad(machine, *function);
    GenJumpingBoolCode();
}

void EmitterVisitor::Visit(BoundProperty& boundProperty)
{
    boundProperty.GenLoad(machine, *function);
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

void EmitterVisitor::Visit(BoundDelegateCall& boundDelegateCall)
{
    boundDelegateCall.GenLoad(machine, *function);
    GenJumpingBoolCode();
}

void EmitterVisitor::Visit(BoundClassDelegateCall& boundClassDelegateCall)
{
    boundClassDelegateCall.GenLoad(machine, *function);
    GenJumpingBoolCode();
}

void EmitterVisitor::Visit(BoundNewExpression& boundNewExpression)
{
    boundNewExpression.GenLoad(machine, *function);
    GenJumpingBoolCode();
}

void EmitterVisitor::Visit(BoundConjunction& boundConjunction)
{
    bool prevGenJumpingBoolCode = genJumpingBoolCode;
    if (genJumpingBoolCode)
    {
        std::vector<Instruction*>* prevTrueSet = trueSet;
        std::vector<Instruction*>* prevFalseSet = falseSet;
        std::vector<Instruction*> leftTrue;
        std::vector<Instruction*> leftFalse;
        trueSet = &leftTrue;
        falseSet = &leftFalse;
        genJumpingBoolCode = true;
        boundConjunction.Left()->Accept(*this);
        trueSet = prevTrueSet;
        falseSet = prevFalseSet;
        prevTrueSet = trueSet; 
        prevFalseSet = falseSet;
        std::vector<Instruction*> rightTrue;
        std::vector<Instruction*> rightFalse;
        trueSet = &rightTrue;
        falseSet = &rightFalse;
        genJumpingBoolCode = true;
        InstIndexRequest startRight;
        AddIndexRequest(&startRight);
        boundConjunction.Right()->Accept(*this);
        int32_t rightTarget = startRight.Index();
        Backpatch(leftTrue, rightTarget);
        trueSet = prevTrueSet;
        falseSet = prevFalseSet;
        Assert(trueSet, "true set not set");
        Assert(falseSet, "false set not set");
        Merge(rightTrue, *trueSet);
        Merge(leftFalse, *falseSet);
        Merge(rightFalse, *falseSet);
    }
    else
    {
        std::vector<Instruction*>* prevTrueSet = trueSet;
        std::vector<Instruction*>* prevFalseSet = falseSet;
        std::vector<Instruction*> leftTrue;
        std::vector<Instruction*> leftFalse;
        trueSet = &leftTrue;
        falseSet = &leftFalse;
        genJumpingBoolCode = true;
        boundConjunction.Left()->Accept(*this);
        trueSet = prevTrueSet;
        falseSet = prevFalseSet;
        prevTrueSet = trueSet;
        prevFalseSet = falseSet;
        std::vector<Instruction*> rightTrue;
        std::vector<Instruction*> rightFalse;
        trueSet = &rightTrue;
        falseSet = &rightFalse;
        genJumpingBoolCode = true;
        InstIndexRequest startRight;
        AddIndexRequest(&startRight);
        boundConjunction.Right()->Accept(*this);
        int32_t rightTarget = startRight.Index();
        Backpatch(leftTrue, rightTarget);
        trueSet = prevTrueSet;
        falseSet = prevFalseSet;
        ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
        Constant trueConstant(MakeIntegralValue<bool>(true, ValueType::boolType));
        constantPool.Install(trueConstant);
        TypeSymbol* boolType = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Boolean");
        BoundLiteral trueLit(boundCompileUnit.GetAssembly(), boolType, trueConstant);
        genJumpingBoolCode = false;
        InstIndexRequest startTrue;
        AddIndexRequest(&startTrue);
        trueLit.Accept(*this);
        int32_t trueTarget = startTrue.Index();
        Backpatch(rightTrue, trueTarget);
        std::unique_ptr<Instruction> inst = machine.CreateInst("jump");
        Assert(conDisSet, "ConDis set not set");
        conDisSet->push_back(inst.get());
        function->AddInst(std::move(inst));
        Constant falseConstant(MakeIntegralValue<bool>(false, ValueType::boolType));
        constantPool.Install(falseConstant);
        BoundLiteral falseLit(boundCompileUnit.GetAssembly(), boolType, falseConstant);
        genJumpingBoolCode = false;
        InstIndexRequest startFalse;
        AddIndexRequest(&startFalse);
        falseLit.Accept(*this);
        int32_t falseTarget = startFalse.Index();
        Backpatch(leftFalse, falseTarget);
        Backpatch(rightFalse, falseTarget);
    }
    genJumpingBoolCode = prevGenJumpingBoolCode;
}

void EmitterVisitor::Visit(BoundDisjunction& boundDisjunction)
{
    bool prevGenJumpingBoolCode = genJumpingBoolCode;
    if (genJumpingBoolCode)
    {
        std::vector<Instruction*>* prevTrueSet = trueSet;
        std::vector<Instruction*>* prevFalseSet = falseSet;
        std::vector<Instruction*> leftTrue;
        std::vector<Instruction*> leftFalse;
        trueSet = &leftTrue;
        falseSet = &leftFalse;
        genJumpingBoolCode = true;
        boundDisjunction.Left()->Accept(*this);
        trueSet = prevTrueSet;
        falseSet = prevFalseSet;
        prevTrueSet = trueSet;
        prevFalseSet = falseSet;
        std::vector<Instruction*> rightTrue;
        std::vector<Instruction*> rightFalse;
        trueSet = &rightTrue;
        falseSet = &rightFalse;
        genJumpingBoolCode = true;
        InstIndexRequest startRight;
        AddIndexRequest(&startRight);
        boundDisjunction.Right()->Accept(*this);
        int32_t rightTarget = startRight.Index();
        Backpatch(leftFalse, rightTarget);
        trueSet = prevTrueSet;
        falseSet = prevFalseSet;
        Assert(trueSet, "true set not set");
        Assert(falseSet, "false set not set");
        Merge(leftTrue, *trueSet);
        Merge(rightTrue, *trueSet);
        Merge(rightFalse, *falseSet);
    }
    else
    {
        std::vector<Instruction*>* prevTrueSet = trueSet;
        std::vector<Instruction*>* prevFalseSet = falseSet;
        std::vector<Instruction*> leftTrue;
        std::vector<Instruction*> leftFalse;
        trueSet = &leftTrue;
        falseSet = &leftFalse;
        genJumpingBoolCode = true;
        boundDisjunction.Left()->Accept(*this);
        trueSet = prevTrueSet;
        falseSet = prevFalseSet;
        prevTrueSet = trueSet;
        prevFalseSet = falseSet;
        std::vector<Instruction*> rightTrue;
        std::vector<Instruction*> rightFalse;
        trueSet = &rightTrue;
        falseSet = &rightFalse;
        genJumpingBoolCode = true;
        InstIndexRequest startRight;
        AddIndexRequest(&startRight);
        boundDisjunction.Right()->Accept(*this);
        int32_t rightTarget = startRight.Index();
        Backpatch(leftFalse, rightTarget);
        trueSet = prevTrueSet;
        falseSet = prevFalseSet;
        ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
        Constant trueConstant(MakeIntegralValue<bool>(true, ValueType::boolType));
        constantPool.Install(trueConstant);
        TypeSymbol* boolType = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Boolean");
        BoundLiteral trueLit(boundCompileUnit.GetAssembly(), boolType, trueConstant);
        genJumpingBoolCode = false;
        InstIndexRequest startTrue;
        AddIndexRequest(&startTrue);
        trueLit.Accept(*this);
        int32_t trueTarget = startTrue.Index();
        Backpatch(rightTrue, trueTarget);
        Backpatch(leftTrue, trueTarget);
        std::unique_ptr<Instruction> inst = machine.CreateInst("jump");
        Assert(conDisSet, "ConDis set not set");
        conDisSet->push_back(inst.get());
        function->AddInst(std::move(inst));
        Constant falseConstant(MakeIntegralValue<bool>(false, ValueType::boolType));
        constantPool.Install(falseConstant);
        BoundLiteral falseLit(boundCompileUnit.GetAssembly(), boolType, falseConstant);
        genJumpingBoolCode = false;
        InstIndexRequest startFalse;
        AddIndexRequest(&startFalse);
        falseLit.Accept(*this);
        int32_t falseTarget = startFalse.Index();
        Backpatch(rightFalse, falseTarget);
    }
    genJumpingBoolCode = prevGenJumpingBoolCode;
}

void EmitterVisitor::Visit(BoundIsExpression& boundIsExpression)
{
    boundIsExpression.GenLoad(machine, *function);
    GenJumpingBoolCode();
}

void EmitterVisitor::Visit(BoundAsExpression& boundAsExpression)
{
    boundAsExpression.GenLoad(machine, *function);
    GenJumpingBoolCode();
}

void EmitterVisitor::Visit(BoundLocalRefExpression& boundLocalRefExpression)
{
    boundLocalRefExpression.GenLoad(machine, *function);
    GenJumpingBoolCode();
}

void EmitterVisitor::Visit(BoundDefaultExpression& boundDefaultExpression)
{
    boundDefaultExpression.GenLoad(machine, *function);
    GenJumpingBoolCode();
}

void EmitterVisitor::Visit(GenObject& genObject)
{
    BoundExpression* expr = dynamic_cast<BoundExpression*>(&genObject);
    Assert(expr, "bound expression expected");
    expr->Accept(*this);
}

void GenerateCode(BoundCompileUnit& boundCompileUnit, Machine& machine)
{
    EmitterVisitor emitterVisitor(machine, boundCompileUnit);
    boundCompileUnit.Accept(emitterVisitor);
}

} } // namespace cminor::emitter
