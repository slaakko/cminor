// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/BoundCompileUnit.hpp>
#include <cminor/binder/BoundNodeVisitor.hpp>
#include <cminor/binder/BoundExpression.hpp>
#include <cminor/symbols/DelegateSymbol.hpp>

namespace cminor { namespace binder {

BoundCompileUnit::BoundCompileUnit(Assembly& assembly_, CompileUnitNode* compileUnitNode_) : 
    BoundNode(assembly_), assembly(assembly_), compileUnitNode(compileUnitNode_), conversionTable(assembly_), classTemplateRepository(*this), hasGotos(false)
{
    conversionTable.SetConversionMap(assembly.GetSymbolTable().GetConversionTable().ConversionMap());
}

void BoundCompileUnit::AddFileScope(std::unique_ptr<FileScope>&& fileScope)
{
    fileScopes.push_back(std::move(fileScope));
}

void BoundCompileUnit::RemoveLastFileScope()
{
    fileScopes.pop_back();
}

void BoundCompileUnit::AddBoundNode(std::unique_ptr<BoundNode>&& boundNode)
{
    boundNodes.push_back(std::move(boundNode));
}

void BoundCompileUnit::AddNode(std::unique_ptr<Node>&& node_)
{
    nodes.push_back(std::move(node_));
}

void BoundCompileUnit::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

FunctionSymbol* BoundCompileUnit::GetConversion(TypeSymbol* sourceType, TypeSymbol* targetType) 
{
    FunctionSymbol* conversionFun = conversionTable.GetConversion(sourceType, targetType);
    if (conversionFun)
    {
        return conversionFun;
    }
    if (targetType->IsClassDelegateType())
    {
        ClassDelegateTypeSymbol* targetClassDelegateType = static_cast<ClassDelegateTypeSymbol*>(targetType);
        FunctionGroupSymbol* sourceFunctionGroup = nullptr;
        if (sourceType->IsMemberExpressionTypeSymbol())
        {
            MemberExpressionTypeSymbol* memberExpressionType = static_cast<MemberExpressionTypeSymbol*>(sourceType);
            BoundMemberExpression* boundMemberExpr = static_cast<BoundMemberExpression*>(memberExpressionType->BoundMemberExpression());
            BoundFunctionGroupExpression* boundFunctionGroup = dynamic_cast<BoundFunctionGroupExpression*>(boundMemberExpr->Member());
            if (!boundFunctionGroup)
            {
                Assert(false, "bound function group expected");
            }
            sourceFunctionGroup = boundFunctionGroup->FunctionGroup();
        }
        else if (sourceType->IsFunctionGroupTypeSymbol())
        {
            FunctionGroupTypeSymbol* functionGroupType = static_cast<FunctionGroupTypeSymbol*>(sourceType);
            sourceFunctionGroup = functionGroupType->FunctionGroup();
        }
        else
        {
            return nullptr;
        }
        int arity = targetClassDelegateType->Arity();
        std::unordered_set<FunctionSymbol*> viableFunctions;
        sourceFunctionGroup->CollectViableFunctions(arity + 1, viableFunctions);
        for (FunctionSymbol* viableFunction : viableFunctions)
        {
            bool functionFound = true;
            for (int i = 1; i < arity + 1; ++i)
            {
                ParameterSymbol* sourceParam = viableFunction->Parameters()[i];
                ParameterSymbol* targetParam = targetClassDelegateType->Parameters()[i - 1];
                if (sourceParam->GetType() != targetParam->GetType())
                {
                    functionFound = false;
                    break;
                }
            }
            if (viableFunction->ReturnType() != targetClassDelegateType->GetReturnType())
            {
                functionFound = false;
            }
            if (functionFound)
            {
                ConstantPool& constantPool = assembly.GetConstantPool();
                utf32_string fullFunctionName = viableFunction->FullName();
                Constant functionNameContant = constantPool.GetConstant(constantPool.Install(StringPtr(fullFunctionName.c_str())));
                Constant groupName = constantPool.GetConstant(constantPool.Install(StringPtr(U"@conversion")));
                utf32_string conversionName = sourceType->FullName() + U"2" + targetType->FullName();
                Constant conversionNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(conversionName.c_str())));
                std::unique_ptr<MemFunToClassDelegateConversion> memfun2ClassDlgConversion(new MemFunToClassDelegateConversion(Span(), conversionNameConstant));
                memfun2ClassDlgConversion->SetAssembly(&assembly);
                memfun2ClassDlgConversion->SetGroupNameConstant(groupName);
                memfun2ClassDlgConversion->SetSourceType(sourceType);
                memfun2ClassDlgConversion->SetTargetType(targetType);
                memfun2ClassDlgConversion->SetFunctionName(functionNameContant);
                FunctionSymbol* conversionFun = memfun2ClassDlgConversion.get();
                classDelegateConversions.push_back(std::move(memfun2ClassDlgConversion));
                return conversionFun;
            }
        }
    }
    return nullptr;
}

void BoundCompileUnit::AddClassNode(ClassNode* classNode)
{
    classNodes.push_back(std::unique_ptr<ClassNode>(classNode));
}

} } // namespace cminor::binder
