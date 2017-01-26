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
    void GenJumpingBoolCode();
    void Backpatch(std::vector<Instruction*>& set, int32_t target);
    void Merge(std::vector<Instruction*>& fromSet, std::vector<Instruction*>& toSet);
    void ExitBlocks(BoundCompoundStatement* targetBlock);
    void AddNextInst(std::unique_ptr<Instruction>&& nextInst);
};

EmitterVisitor::EmitterVisitor(Machine& machine_, BoundCompileUnit& boundCompileUnit_) : 
    machine(machine_), boundCompileUnit(boundCompileUnit_), function(nullptr), nextSet(nullptr), trueSet(nullptr), falseSet(nullptr), breakSet(nullptr), continueSet(nullptr), conDisSet(nullptr),
    breakTarget(nullptr), continueTarget(nullptr), caseTargets(nullptr), gotoCaseJumps(nullptr), gotoDefaultJumps(nullptr), genJumpingBoolCode(false), 
    createPCRange(false), setPCRangeEnd(false), currentExceptionBlock(nullptr)
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
        std::unique_ptr<Instruction> exitBlockInst = machine.CreateInst("exitblock");
        int32_t exceptionBlockId = block->ExceptionBlockId();
        bool add = !GetGlobalFlag(GlobalFlags::release);
        if (exceptionBlockId != -1)
        {
            ExitBlockInst* exit = dynamic_cast<ExitBlockInst*>(exitBlockInst.get());
            Assert(exit, "exit block instruction expected");
            exit->SetExceptionBlockId(exceptionBlockId);
            add = true;
        }
        if (add)
        {
            function->AddInst(std::move(exitBlockInst));
        }
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
    boundFunction.GetFunctionSymbol()->CreateMachineFunction();
    Function* prevFunction = function;
    function = boundFunction.GetFunctionSymbol()->MachineFunction();
    Emitter* prevEmitter = function->GetEmitter();
    function->SetEmitter(this);
    if (boundFunction.Body())
    {
        boundFunction.Body()->Accept(*this);
    }
    function->SetEmitter(prevEmitter);
    function = prevFunction;
}

void EmitterVisitor::Visit(BoundCompoundStatement& boundCompoundStatement)
{
    SetCurrentSourceLine(boundCompoundStatement.GetSpan().LineNumber());
    blockStack.push_back(&boundCompoundStatement);
    bool setFirstInstIndex = false;
    if (!GetGlobalFlag(GlobalFlags::release))
    {
        std::unique_ptr<Instruction> enterBlockInst = machine.CreateInst("enterblock");
        InstIndexRequest enterBlock;
        AddIndexRequest(&enterBlock);
        function->AddInst(std::move(enterBlockInst));
        boundCompoundStatement.SetFirstInstIndex(enterBlock.Index());
        function->MapPCToSourceLine(enterBlock.Index(), boundCompoundStatement.BeginBraceSpan().LineNumber());
    }
    else
    {
        setFirstInstIndex = true;
    }
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
        if (setFirstInstIndex)
        {
            setFirstInstIndex = false;
            boundCompoundStatement.SetFirstInstIndex(startStatement.Index());
            function->MapPCToSourceLine(startStatement.Index(), boundCompoundStatement.BeginBraceSpan().LineNumber());
        }
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
    if (!GetGlobalFlag(GlobalFlags::release))
    {
        InstIndexRequest exitBlock;
        AddIndexRequest(&exitBlock);
        std::unique_ptr<Instruction> exitBlockInst = machine.CreateInst("exitblock");
        function->AddInst(std::move(exitBlockInst));
        function->MapPCToSourceLine(exitBlock.Index(), boundCompoundStatement.EndBraceSpan().LineNumber());
        int32_t exitBlockTarget = exitBlock.Index();
        Backpatch(prevNext, exitBlockTarget);
    }
    blockStack.pop_back();
}

void EmitterVisitor::Visit(BoundReturnStatement& boundReturnStatement)
{
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
}

void EmitterVisitor::Visit(BoundIfStatement& boundIfStatement)
{
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
}

void EmitterVisitor::Visit(BoundWhileStatement& boundWhileStatement)
{
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
    boundWhileStatement.Statement()->Accept(*this);
    function->MapPCToSourceLine(startStatement.Index(), boundWhileStatement.Statement()->GetSpan().LineNumber());
    boundWhileStatement.Statement()->SetFirstInstIndex(startStatement.Index());
    Backpatch(true_, startStatement.Index());
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
}

void EmitterVisitor::Visit(BoundDoStatement& boundDoStatement)
{
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
    InstIndexRequest startStatement;
    AddIndexRequest(&startStatement);
    boundDoStatement.Statement()->Accept(*this);
    function->MapPCToSourceLine(startStatement.Index(), boundDoStatement.GetSpan().LineNumber());
    int32_t statementTarget = startStatement.Index();
    boundDoStatement.Statement()->SetFirstInstIndex(statementTarget);
    trueSet = &true_;
    falseSet = &false_;
    bool prevGenJumpingBoolCode = genJumpingBoolCode;
    genJumpingBoolCode = true;
    InstIndexRequest startCond;
    AddIndexRequest(&startCond);
    boundDoStatement.Condition()->Accept(*this);
    int32_t conditionTarget = startCond.Index();
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
}

void EmitterVisitor::Visit(BoundForStatement& boundForStatement)
{
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
    InstIndexRequest startAction;
    AddIndexRequest(&startAction);
    boundForStatement.ActionS()->Accept(*this);
    function->MapPCToSourceLine(startAction.Index(), boundForStatement.ActionS()->GetSpan().LineNumber());
    int32_t actionTarget = startAction.Index();
    boundForStatement.ActionS()->SetFirstInstIndex(actionTarget);
    Backpatch(true_, actionTarget);
    InstIndexRequest startLoop;
    AddIndexRequest(&startLoop);
    boundForStatement.LoopS()->Accept(*this);
    int32_t loopTarget = startLoop.Index();
    boundForStatement.LoopS()->SetFirstInstIndex(loopTarget);
    Backpatch(continue_, loopTarget);
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
}

void EmitterVisitor::Visit(BoundSwitchStatement& boundSwitchStatement)
{
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
}

void EmitterVisitor::Visit(BoundCaseStatement& boundCaseStatement)
{
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
}

void EmitterVisitor::Visit(BoundDefaultStatement& boundDefaultStatement)
{
    SetCurrentSourceLine(boundDefaultStatement.GetSpan().LineNumber());
    InstIndexRequest startDefault;
    AddIndexRequest(&startDefault);
    boundDefaultStatement.CompoundStatement()->Accept(*this);
    function->MapPCToSourceLine(startDefault.Index(), boundDefaultStatement.GetSpan().LineNumber());
    boundDefaultStatement.SetFirstInstIndex(startDefault.Index());
}

void EmitterVisitor::Visit(BoundGotoCaseStatement& boundGotoCaseStatement)
{
    SetCurrentSourceLine(boundGotoCaseStatement.GetSpan().LineNumber());
    ExitBlocks(breakTarget->Block());
    std::unique_ptr<Instruction> jump = machine.CreateInst("jump");
    Instruction* gotoCaseJump = jump.get();
    function->AddInst(std::move(jump));
    gotoCaseJumps->insert(std::make_pair(boundGotoCaseStatement.CaseValue(), gotoCaseJump));
}

void EmitterVisitor::Visit(BoundGotoDefaultStatement& boundGotoDefaultStatement)
{
    SetCurrentSourceLine(boundGotoDefaultStatement.GetSpan().LineNumber());
    ExitBlocks(breakTarget->Block());
    std::unique_ptr<Instruction> jump = machine.CreateInst("jump");
    Instruction* gotoDefaultJump = jump.get();
    function->AddInst(std::move(jump));
    gotoDefaultJumps->push_back(gotoDefaultJump);
}

void EmitterVisitor::Visit(BoundBreakStatement& boundBreakStatement)
{
    SetCurrentSourceLine(boundBreakStatement.GetSpan().LineNumber());
    Assert(breakSet, "break set not set");
    BoundCompoundStatement* targetBlock = breakTarget->Parent()->Block();
    ExitBlocks(targetBlock);
    std::unique_ptr<Instruction> jump = machine.CreateInst("jump");
    breakSet->push_back(jump.get());
    function->AddInst(std::move(jump));
}

void EmitterVisitor::Visit(BoundContinueStatement& boundContinueStatement)
{
    SetCurrentSourceLine(boundContinueStatement.GetSpan().LineNumber());
    Assert(continueSet, "continue set not set");
    BoundCompoundStatement* targetBlock = continueTarget->Parent()->Block();
    ExitBlocks(targetBlock);
    std::unique_ptr<Instruction> jump = machine.CreateInst("jump");
    continueSet->push_back(jump.get());
    function->AddInst(std::move(jump));
}

void EmitterVisitor::Visit(BoundGotoStatement& boundGotoStatement)
{
    SetCurrentSourceLine(boundGotoStatement.GetSpan().LineNumber());
    BoundCompoundStatement* targetBlock = boundGotoStatement.TargetBlock();
    ExitBlocks(targetBlock);
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
}

void EmitterVisitor::Visit(BoundConstructionStatement& boundConstructionStatement)
{
    SetCurrentSourceLine(boundConstructionStatement.GetSpan().LineNumber());
    std::vector<Instruction*>* prevConDisSet = conDisSet;
    std::vector<Instruction*> conDis;
    conDisSet = &conDis;
    boundConstructionStatement.ConstructorCall()->Accept(*this);
    conDisSet = prevConDisSet;
}

void EmitterVisitor::Visit(BoundAssignmentStatement& boundAssignmentStatement)
{
    SetCurrentSourceLine(boundAssignmentStatement.GetSpan().LineNumber());
    std::vector<Instruction*>* prevConDisSet = conDisSet;
    std::vector<Instruction*> conDis;
    conDisSet = &conDis;
    boundAssignmentStatement.AssignmentCall()->Accept(*this);
    conDisSet = prevConDisSet;
}

void EmitterVisitor::Visit(BoundExpressionStatement& boundExpressionStatement)
{
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
}

void EmitterVisitor::Visit(BoundEmptyStatement& boundEmptyStatement)
{
    SetCurrentSourceLine(boundEmptyStatement.GetSpan().LineNumber());
    std::unique_ptr<Instruction> nop = machine.CreateInst("nop");
    function->AddInst(std::move(nop));
}

void EmitterVisitor::Visit(BoundThrowStatement& boundThrowStatement)
{
    SetCurrentSourceLine(boundThrowStatement.GetSpan().LineNumber());
    if (boundThrowStatement.Expression())
    {
        boundThrowStatement.Expression()->Accept(*this);
        std::unique_ptr<Instruction> throwInst = machine.CreateInst("throw");
        InstIndexRequest startThrow;
        AddIndexRequest(&startThrow);
        function->AddInst(std::move(throwInst));
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
}

void EmitterVisitor::Visit(BoundTryStatement& boundTryStatement)
{
    SetCurrentSourceLine(boundTryStatement.GetSpan().LineNumber());
    InstIndexRequest startTry;
    AddIndexRequest(&startTry);
    std::unique_ptr<Instruction> beginTryInst = machine.CreateInst("begintry");
    function->AddInst(std::move(beginTryInst));
    function->MapPCToSourceLine(startTry.Index(), CurrentSourceLine());
    bool prevCreatePCRange = createPCRange;
    bool prevSetPCRangeEnd = setPCRangeEnd;
    createPCRange = true;
    setPCRangeEnd = true;
    std::unique_ptr<ExceptionBlock> exceptionBlock(new ExceptionBlock(function->GetNextExeptionBlockId()));
    ExceptionBlock* parentExceptionBlock = currentExceptionBlock;
    if (parentExceptionBlock)
    {
        exceptionBlock->SetParentId(parentExceptionBlock->Id());
    }
    currentExceptionBlock = exceptionBlock.get();
    function->AddExceptionBlock(std::move(exceptionBlock));
    boundTryStatement.TryBlock()->SetExceptionBlockId(currentExceptionBlock->Id());
    boundTryStatement.TryBlock()->Accept(*this);
    if (boundTryStatement.FinallyBlock())
    {
        InstIndexRequest startFinally;
        AddIndexRequest(&startFinally);
        boundTryStatement.FinallyBlock()->Accept(*this);
        function->MapPCToSourceLine(startFinally.Index(), boundTryStatement.FinallyBlock()->GetSpan().LineNumber());
        int32_t finallyBlockStart = startFinally.Index();
        currentExceptionBlock->SetFinallyStart(finallyBlockStart);
        std::unique_ptr<Instruction> inst = machine.CreateInst("endfinally");
        function->AddInst(std::move(inst));
    }
    std::unique_ptr<Instruction> inst = machine.CreateInst("jump");
    nextSet->push_back(inst.get());
    function->AddInst(std::move(inst));
    std::unique_ptr<Instruction> nextInst = machine.CreateInst("next");
    NextInst* nxtInst = dynamic_cast<NextInst*>(nextInst.get());
    nxtInst->SetExceptionBlock(currentExceptionBlock);
    nextSet->push_back(nextInst.get());
    AddNextInst(std::move(nextInst));
    setPCRangeEnd = prevSetPCRangeEnd;
    createPCRange = prevCreatePCRange;
    int n = int(boundTryStatement.Catches().size());
    for (int i = 0; i < n; ++i)
    {
        const std::unique_ptr<BoundCatchStatement>& catchStatement = boundTryStatement.Catches()[i];
        catchStatement->Accept(*this);
    }
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
    std::unique_ptr<Instruction> endTryInst = machine.CreateInst("endtry");
    function->AddInst(std::move(endTryInst));
}

void EmitterVisitor::Visit(BoundCatchStatement& boundCatchStatement)
{
    SetCurrentSourceLine(boundCatchStatement.GetSpan().LineNumber());
    std::unique_ptr<CatchBlock> catchBlock(new CatchBlock());
    LocalVariableSymbol* exceptionVar = boundCatchStatement.ExceptionVar();
    TypeSymbol* exceptionVarType = exceptionVar->GetType();
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    utf32_string fullClassName = exceptionVarType->FullName();
    Constant exceptionVarClassTypeFullName = constantPool.GetConstant(constantPool.Install(StringPtr(fullClassName.c_str())));
    catchBlock->SetExceptionVarClassTypeFullName(exceptionVarClassTypeFullName);
    catchBlock->SetExceptionVarIndex(exceptionVar->Index());
    InstIndexRequest startCatch;
    AddIndexRequest(&startCatch);
    boundCatchStatement.CatchBlock()->Accept(*this);
    function->MapPCToSourceLine(startCatch.Index(), boundCatchStatement.GetSpan().LineNumber());
    int32_t catchBlockStart = startCatch.Index();
    catchBlock->SetCatchBlockStart(catchBlockStart);
    std::unique_ptr<Instruction> inst = machine.CreateInst("endcatch");
    function->AddInst(std::move(inst));
    currentExceptionBlock->AddCatchBlock(std::move(catchBlock));
}

void EmitterVisitor::Visit(BoundStaticInitStatement& boundStaticInitStatement)
{
    SetCurrentSourceLine(boundStaticInitStatement.GetSpan().LineNumber());
    std::unique_ptr<Instruction> inst = machine.CreateInst("staticinit");
    StaticInitInst* staticInitInst = dynamic_cast<StaticInitInst*>(inst.get());
    Assert(staticInitInst, "static init instruction expected");
    staticInitInst->SetTypeName(boundStaticInitStatement.ClassNameConstant());
    function->AddInst(std::move(inst));
}

void EmitterVisitor::Visit(BoundDoneStaticInitStatement& boundDoneStaticInitStatement)
{
    SetCurrentSourceLine(boundDoneStaticInitStatement.GetSpan().LineNumber());
    std::unique_ptr<Instruction> inst = machine.CreateInst("donestaticinit");
    DoneStaticInitInst* doneStaticInitInst = dynamic_cast<DoneStaticInitInst*>(inst.get());
    Assert(doneStaticInitInst, "done static init instruction expected");
    doneStaticInitInst->SetTypeName(boundDoneStaticInitStatement.ClassNameConstant());
    function->AddInst(std::move(inst));
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
        Constant trueConstant(IntegralValue(uint64_t(true), ValueType::boolType));
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
        Constant falseConstant(IntegralValue(uint64_t(false), ValueType::boolType));
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
        Constant trueConstant(IntegralValue(uint64_t(true), ValueType::boolType));
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
        Constant falseConstant(IntegralValue(uint64_t(false), ValueType::boolType));
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
