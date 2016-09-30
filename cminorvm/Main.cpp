// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <iostream>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Function.hpp>
#include <cminor/machine/Unicode.hpp>

using namespace cminor::machine;

int main()
{
    try
    {
        Assembly assembly("assembly");
        Function mainFun("main");
        mainFun.SetAssembly(&assembly);
        mainFun.SetNumLocals(1);
        Machine machine;
        ConstantId cid = assembly.GetConstantPool().Install(Constant(IntegralValue(1, ValueType::boolType)));
        LoadConstantInst* loadConstant = new LoadConstantInst();
        loadConstant->SetIndex(cid.Value());
        mainFun.AddInst(std::unique_ptr<Instruction>(loadConstant));
        StoreFieldInst* storeField = new StoreFieldInst();
        storeField->SetIndex(0);
        mainFun.AddInst(std::unique_ptr<Instruction>(storeField));
        std::unique_ptr<Thread> mainThread(new Thread(machine, mainFun));
        machine.Threads().push_back(std::move(mainThread));
        Type* type = assembly.GetTypeRepository().CreateType("foo");
        type->AddField(ValueType::boolType);
        ObjectReference reference = machine.GetObjectPool().CreateObject(machine.MainThread(), type);
        Frame& frame = machine.MainThread().Frames().back();
        frame.OpStack().Push(reference);
        frame.OpStack().Push(reference);
        StoreLocalInst* storeLocal = new StoreLocalInst();
        storeLocal->SetIndex(0);
        mainFun.AddInst(std::unique_ptr<Instruction>(storeLocal));
        machine.MainThread().Run();
        int x = 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}