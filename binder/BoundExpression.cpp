// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/BoundExpression.hpp>
#include <cminor/binder/BoundNodeVisitor.hpp>
#include <cminor/symbols/FunctionSymbol.hpp>
#include <cminor/symbols/EnumSymbol.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/machine/Function.hpp>
#include <cminor/machine/Machine.hpp>

namespace cminor { namespace binder {

BoundExpression::BoundExpression(Assembly& assembly_, TypeSymbol* type_) : BoundNode(assembly_), type(type_), flags(BoundExpressionFlags::none)
{
}

BoundLiteral::BoundLiteral(Assembly& assembly_, TypeSymbol* type_, Constant value_) : BoundExpression(assembly_, type_), value(value_)
{
}

void BoundLiteral::GenLoad(Machine& machine, Function& function)
{
    std::unique_ptr<Instruction> loadConstantInst; 
    ConstantId constantId = GetAssembly().GetConstantPool().GetIdFor(value);
    if (constantId != noConstantId)
    {
        if (constantId.Value() < 256)
        {
            loadConstantInst = std::move(machine.CreateInst("loadconstant.b"));
            loadConstantInst->SetIndex(static_cast<uint8_t>(constantId.Value()));
        }
        else if (constantId.Value() < 65536)
        {
            loadConstantInst = std::move(machine.CreateInst("loadconstant.s"));
            loadConstantInst->SetIndex(static_cast<uint16_t>(constantId.Value()));
        }
        else
        {
            loadConstantInst = std::move(machine.CreateInst("loadconstant"));
            loadConstantInst->SetIndex(int32_t(constantId.Value()));
        }
        function.AddInst(std::move(loadConstantInst));
    }
    else
    {
        throw std::runtime_error("id for constant not found");
    }
}

void BoundLiteral::GenStore(Machine& machine, Function& function) 
{
    throw std::runtime_error("cannot store to literal");
}

void BoundLiteral::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundConstant::BoundConstant(Assembly& assembly_, TypeSymbol* type_, ConstantSymbol* constantSymbol_) : BoundExpression(assembly_, type_), constantSymbol(constantSymbol_)
{
    GetAssembly().GetConstantPool().Install(constantSymbol->GetConstant());
}

void BoundConstant::GenLoad(Machine& machine, Function& function)
{
    std::unique_ptr<Instruction> loadConstantInst;
    ConstantId constantId = GetAssembly().GetConstantPool().GetIdFor(constantSymbol->GetConstant());
    if (constantId != noConstantId)
    {
        if (constantId.Value() < 256)
        {
            loadConstantInst = std::move(machine.CreateInst("loadconstant.b"));
            loadConstantInst->SetIndex(static_cast<uint8_t>(constantId.Value()));
        }
        else if (constantId.Value() < 65536)
        {
            loadConstantInst = std::move(machine.CreateInst("loadconstant.s"));
            loadConstantInst->SetIndex(static_cast<uint16_t>(constantId.Value()));
        }
        else
        {
            loadConstantInst = std::move(machine.CreateInst("loadconstant"));
            loadConstantInst->SetIndex(int32_t(constantId.Value()));
        }
        function.AddInst(std::move(loadConstantInst));
    }
    else
    {
        throw std::runtime_error("id for constant not found");
    }
}

void BoundConstant::GenStore(Machine& machine, Function& function) 
{
    throw std::runtime_error("cannot store to constant");
}

void BoundConstant::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundEnumConstant::BoundEnumConstant(Assembly& assembly_, TypeSymbol* type_, EnumConstantSymbol* enumConstantSymbol_) : BoundExpression(assembly_, type_), enumConstantSymbol(enumConstantSymbol_)
{
    GetAssembly().GetConstantPool().Install(enumConstantSymbol->GetConstant());
}

void BoundEnumConstant::GenLoad(Machine& machine, Function& function)
{
    std::unique_ptr<Instruction> loadConstantInst;
    ConstantId constantId = GetAssembly().GetConstantPool().GetIdFor(enumConstantSymbol->GetConstant());
    if (constantId != noConstantId)
    {
        if (constantId.Value() < 256)
        {
            loadConstantInst = std::move(machine.CreateInst("loadconstant.b"));
            loadConstantInst->SetIndex(static_cast<uint8_t>(constantId.Value()));
        }
        else if (constantId.Value() < 65536)
        {
            loadConstantInst = std::move(machine.CreateInst("loadconstant.s"));
            loadConstantInst->SetIndex(static_cast<uint16_t>(constantId.Value()));
        }
        else
        {
            loadConstantInst = std::move(machine.CreateInst("loadconstant"));
            loadConstantInst->SetIndex(int32_t(constantId.Value()));
        }
        function.AddInst(std::move(loadConstantInst));
    }
    else
    {
        throw std::runtime_error("id for constant not found");
    }
}

void BoundEnumConstant::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to enum constant");
}

void BoundEnumConstant::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundLocalVariable::BoundLocalVariable(Assembly& assembly_, TypeSymbol* type_, LocalVariableSymbol* localVariableSymbol_) : BoundExpression(assembly_, type_), localVariableSymbol(localVariableSymbol_)
{
}

void BoundLocalVariable::GenLoad(Machine& machine, Function& function)
{
    int32_t index = localVariableSymbol->Index();
    std::unique_ptr<Instruction> loadLocalInst;
    if (index != -1)
    {
        switch (index)
        {
            case 0:  loadLocalInst = std::move(machine.CreateInst("loadlocal.0")); break;
            case 1:  loadLocalInst = std::move(machine.CreateInst("loadlocal.1")); break;
            case 2:  loadLocalInst = std::move(machine.CreateInst("loadlocal.2")); break;
            case 3:  loadLocalInst = std::move(machine.CreateInst("loadlocal.3")); break;
            default:
            {
                if (index < 256)
                {
                    loadLocalInst = std::move(machine.CreateInst("loadlocal.b")); 
                    loadLocalInst->SetIndex(static_cast<uint8_t>(index));
                }
                else if (index < 65536)
                {
                    loadLocalInst = std::move(machine.CreateInst("loadlocal.s")); 
                    loadLocalInst->SetIndex(static_cast<uint16_t>(index));
                }
                else
                {
                    loadLocalInst = std::move(machine.CreateInst("loadlocal"));
                    loadLocalInst->SetIndex(index);
                }
                break;
            }
        }
        function.AddInst(std::move(loadLocalInst));
    }
    else
    {
        throw std::runtime_error("invalid local variable index");
    }
}

void BoundLocalVariable::GenStore(Machine& machine, Function& function)
{
    std::unique_ptr<Instruction> storeLocalInst;
    int32_t index = localVariableSymbol->Index();
    if (index != -1)
    {
        switch (index)
        {
            case 0:  storeLocalInst = std::move(machine.CreateInst("storelocal.0")); break;
            case 1:  storeLocalInst = std::move(machine.CreateInst("storelocal.1")); break;
            case 2:  storeLocalInst = std::move(machine.CreateInst("storelocal.2")); break;
            case 3:  storeLocalInst = std::move(machine.CreateInst("storelocal.3")); break;
            default:
            {
                if (index < 256)
                {
                    storeLocalInst = std::move(machine.CreateInst("storelocal.b")); 
                    storeLocalInst->SetIndex(static_cast<uint8_t>(index));
                }
                else if (index < 65536)
                {
                    storeLocalInst = std::move(machine.CreateInst("storelocal.s")); 
                    storeLocalInst->SetIndex(static_cast<uint16_t>(index));
                }
                else
                {
                    storeLocalInst = std::move(machine.CreateInst("storelocal"));
                    storeLocalInst->SetIndex(index);
                }
                break;
            }
        }
        function.AddInst(std::move(storeLocalInst));
    }
    else
    {
        throw std::runtime_error("invalid local variable index");
    }
}

void BoundLocalVariable::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundMemberVariable::BoundMemberVariable(Assembly& assembly_, TypeSymbol* type_, MemberVariableSymbol* memberVariableSymbol_) : 
    BoundExpression(assembly_, type_), memberVariableSymbol(memberVariableSymbol_), staticInitNeeded(false)
{
}

void BoundMemberVariable::SetClassObject(std::unique_ptr<BoundExpression>&& classObject_)
{
    classObject = std::move(classObject_);
}

void BoundMemberVariable::GenLoad(Machine& machine, Function& function)
{
    std::unique_ptr<Instruction> loadFieldInst;
    int32_t index = memberVariableSymbol->Index();
    if (index != -1)
    {
        if (memberVariableSymbol->IsStatic())
        {
            ConstantPool& constantPool = GetAssembly().GetConstantPool();
            Symbol* parent = memberVariableSymbol->Parent();
            ClassTypeSymbol* classTypeSymbol = dynamic_cast<ClassTypeSymbol*>(parent);
            Assert(classTypeSymbol, "class type symbol expected");
            utf32_string classTypeFullName = classTypeSymbol->FullName();
            Constant fullClassNameConstant = constantPool.GetConstant(constantPool.GetIdFor(classTypeFullName));
            if (staticInitNeeded)
            {
                std::unique_ptr<Instruction> inst = machine.CreateInst("staticinit");
                StaticInitInst* staticInitInst = dynamic_cast<StaticInitInst*>(inst.get());
                staticInitInst->SetTypeName(fullClassNameConstant);
                function.AddInst(std::move(inst));
            }
            loadFieldInst = std::move(machine.CreateInst("loadstaticfield"));
            loadFieldInst->SetIndex(index);
            LoadStaticFieldInst* loadStaticField = dynamic_cast<LoadStaticFieldInst*>(loadFieldInst.get());
            loadStaticField->SetTypeName(fullClassNameConstant);
        }
        else
        {
            classObject->GenLoad(machine, function);
            switch (index)
            {
                case 0:  loadFieldInst = std::move(machine.CreateInst("loadfield.0")); break;
                case 1:  loadFieldInst = std::move(machine.CreateInst("loadfield.1")); break;
                case 2:  loadFieldInst = std::move(machine.CreateInst("loadfield.2")); break;
                case 3:  loadFieldInst = std::move(machine.CreateInst("loadfield.3")); break;
                default:
                {
                    if (index < 256)
                    {
                        loadFieldInst = std::move(machine.CreateInst("loadfield.b"));
                        loadFieldInst->SetIndex(static_cast<uint8_t>(index));
                    }
                    else if (index < 65536)
                    {
                        loadFieldInst = std::move(machine.CreateInst("loadfield.s"));
                        loadFieldInst->SetIndex(static_cast<uint16_t>(index));
                    }
                    else
                    {
                        loadFieldInst = std::move(machine.CreateInst("loadfield"));
                        loadFieldInst->SetIndex(index);
                    }
                    break;
                }
            }
        }
        function.AddInst(std::move(loadFieldInst));
    }
    else
    {
        throw std::runtime_error("invalid member variable index");
    }
}

void BoundMemberVariable::GenStore(Machine& machine, Function& function)
{
    int32_t index = memberVariableSymbol->Index();
    std::unique_ptr<Instruction> storeFieldlInst;
    if (index != -1)
    {
        if (memberVariableSymbol->IsStatic())
        {
            ConstantPool& constantPool = GetAssembly().GetConstantPool();
            Symbol* parent = memberVariableSymbol->Parent();
            ClassTypeSymbol* classTypeSymbol = dynamic_cast<ClassTypeSymbol*>(parent);
            Assert(classTypeSymbol, "class type symbol expected");
            utf32_string classTypeFullName = classTypeSymbol->FullName();
            Constant fullClassNameConstant = constantPool.GetConstant(constantPool.GetIdFor(classTypeFullName));
            if (staticInitNeeded)
            {
                std::unique_ptr<Instruction> inst = machine.CreateInst("staticinit");
                StaticInitInst* staticInitInst = dynamic_cast<StaticInitInst*>(inst.get());
                staticInitInst->SetTypeName(fullClassNameConstant);
                function.AddInst(std::move(inst));
            }
            storeFieldlInst = std::move(machine.CreateInst("storestaticfield"));
            storeFieldlInst->SetIndex(index);
            StoreStaticFieldInst* storeStaticField = dynamic_cast<StoreStaticFieldInst*>(storeFieldlInst.get());
            storeStaticField->SetTypeName(fullClassNameConstant);
        }
        else
        {
            classObject->GenLoad(machine, function);
            switch (index)
            {
                case 0:  storeFieldlInst = std::move(machine.CreateInst("storefield.0")); break;
                case 1:  storeFieldlInst = std::move(machine.CreateInst("storefield.1")); break;
                case 2:  storeFieldlInst = std::move(machine.CreateInst("storefield.2")); break;
                case 3:  storeFieldlInst = std::move(machine.CreateInst("storefield.3")); break;
                default:
                {
                    if (index < 256)
                    {
                        storeFieldlInst = std::move(machine.CreateInst("storefield.b"));
                        storeFieldlInst->SetIndex(static_cast<uint8_t>(index));
                    }
                    else if (index < 65536)
                    {
                        storeFieldlInst = std::move(machine.CreateInst("storefield.s"));
                        storeFieldlInst->SetIndex(static_cast<uint16_t>(index));
                    }
                    else
                    {
                        storeFieldlInst = std::move(machine.CreateInst("storefield"));
                        storeFieldlInst->SetIndex(index);
                    }
                    break;
                }
            }
        }
        function.AddInst(std::move(storeFieldlInst));
    }
    else
    {
        throw std::runtime_error("invalid member variable index");
    }
}

void BoundMemberVariable::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundProperty::BoundProperty(Assembly& assembly_, TypeSymbol* type_, PropertySymbol* propertySymbol_) : BoundExpression(assembly_, type_), propertySymbol(propertySymbol_), staticInitNeeded(false)
{
}

void BoundProperty::SetClassObject(std::unique_ptr<BoundExpression>&& classObject_)
{
    classObject = std::move(classObject_);
}

void BoundProperty::GenLoad(Machine& machine, Function& function)
{
    if (classObject)
    {
        classObject->GenLoad(machine, function);
    }
    if (propertySymbol->IsStatic())
    {
        if (staticInitNeeded)
        {
            ConstantPool& constantPool = GetAssembly().GetConstantPool();
            Symbol* parent = propertySymbol->Parent();
            ClassTypeSymbol* classTypeSymbol = dynamic_cast<ClassTypeSymbol*>(parent);
            Assert(classTypeSymbol, "class type symbol expected");
            utf32_string classTypeFullName = classTypeSymbol->FullName();
            Constant fullClassNameConstant = constantPool.GetConstant(constantPool.GetIdFor(classTypeFullName));
            std::unique_ptr<Instruction> inst = machine.CreateInst("staticinit");
            StaticInitInst* staticInitInst = dynamic_cast<StaticInitInst*>(inst.get());
            staticInitInst->SetTypeName(fullClassNameConstant);
            function.AddInst(std::move(inst));
        }
    }
    std::vector<GenObject*> emptyObjects;
    Assert(propertySymbol->Getter(), "property has no getter");
    propertySymbol->Getter()->GenerateCall(machine, GetAssembly(), function, emptyObjects, 0);
}

void BoundProperty::GenStore(Machine& machine, Function& function)
{
    if (classObject)
    {
        classObject->GenLoad(machine, function);
    }
    if (propertySymbol->IsStatic())
    {
        if (staticInitNeeded)
        {
            ConstantPool& constantPool = GetAssembly().GetConstantPool();
            Symbol* parent = propertySymbol->Parent();
            ClassTypeSymbol* classTypeSymbol = dynamic_cast<ClassTypeSymbol*>(parent);
            Assert(classTypeSymbol, "class type symbol expected");
            utf32_string classTypeFullName = classTypeSymbol->FullName();
            Constant fullClassNameConstant = constantPool.GetConstant(constantPool.GetIdFor(classTypeFullName));
            std::unique_ptr<Instruction> inst = machine.CreateInst("staticinit");
            StaticInitInst* staticInitInst = dynamic_cast<StaticInitInst*>(inst.get());
            staticInitInst->SetTypeName(fullClassNameConstant);
            function.AddInst(std::move(inst));
        }
    }
    std::unique_ptr<Instruction> swapInst = machine.CreateInst("swap");
    function.AddInst(std::move(swapInst));
    std::vector<GenObject*> emptyObjects;
    Assert(propertySymbol->Setter(), "property has no setter");
    propertySymbol->Setter()->GenerateCall(machine, GetAssembly(), function, emptyObjects, 0);
}

void BoundProperty::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundIndexer::BoundIndexer(Assembly& assembly_, TypeSymbol* type_, IndexerSymbol* indexerSymbol_, std::unique_ptr<BoundExpression>&& classObject_, std::unique_ptr<BoundExpression>&& index_) : 
    BoundExpression(assembly_, type_), indexerSymbol(indexerSymbol_), classObject(std::move(classObject_)), index(std::move(index_)), staticInitNeeded(false)
{
}

void BoundIndexer::GenLoad(Machine& machine, Function& function)
{
    if (!indexerSymbol->IsStatic())
    {
        classObject->GenLoad(machine, function);
    }
    else
    {
        if (staticInitNeeded)
        {
            ConstantPool& constantPool = GetAssembly().GetConstantPool();
            Symbol* parent = indexerSymbol->Parent();
            ClassTypeSymbol* classTypeSymbol = dynamic_cast<ClassTypeSymbol*>(parent);
            Assert(classTypeSymbol, "class type symbol expected");
            utf32_string classTypeFullName = classTypeSymbol->FullName();
            Constant fullClassNameConstant = constantPool.GetConstant(constantPool.GetIdFor(classTypeFullName));
            std::unique_ptr<Instruction> inst = machine.CreateInst("staticinit");
            StaticInitInst* staticInitInst = dynamic_cast<StaticInitInst*>(inst.get());
            staticInitInst->SetTypeName(fullClassNameConstant);
            function.AddInst(std::move(inst));
        }
    }
    index->GenLoad(machine, function);
    std::vector<GenObject*> emptyObjects;
    Assert(indexerSymbol->Getter(), "indexer has no getter");
    indexerSymbol->Getter()->GenerateCall(machine, GetAssembly(), function, emptyObjects, 0);
}

void BoundIndexer::GenStore(Machine& machine, Function& function)
{
    if (!indexerSymbol->IsStatic())
    {
        classObject->GenLoad(machine, function);
    }
    else 
    {
        if (staticInitNeeded)
        {
            ConstantPool& constantPool = GetAssembly().GetConstantPool();
            Symbol* parent = indexerSymbol->Parent();
            ClassTypeSymbol* classTypeSymbol = dynamic_cast<ClassTypeSymbol*>(parent);
            Assert(classTypeSymbol, "class type symbol expected");
            utf32_string classTypeFullName = classTypeSymbol->FullName();
            Constant fullClassNameConstant = constantPool.GetConstant(constantPool.GetIdFor(classTypeFullName));
            std::unique_ptr<Instruction> inst = machine.CreateInst("staticinit");
            StaticInitInst* staticInitInst = dynamic_cast<StaticInitInst*>(inst.get());
            staticInitInst->SetTypeName(fullClassNameConstant);
            function.AddInst(std::move(inst));
        }
    }
    index->GenLoad(machine, function);
    std::unique_ptr<Instruction> rotateInst = machine.CreateInst("rotate");
    function.AddInst(std::move(rotateInst));
    std::vector<GenObject*> emptyObjects;
    Assert(indexerSymbol->Setter(), "indexer has no setter");
    indexerSymbol->Setter()->GenerateCall(machine, GetAssembly(), function, emptyObjects, 0);
}

void BoundIndexer::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundParameter::BoundParameter(Assembly& assembly_, TypeSymbol* type_, ParameterSymbol* parameterSymbol_) : BoundExpression(assembly_, type_), parameterSymbol(parameterSymbol_)
{
}

void BoundParameter::GenLoad(Machine& machine, Function& function)
{
    int32_t index = parameterSymbol->Index();
    std::unique_ptr<Instruction> loadLocalInst;
    if (index != -1)
    {
        if (parameterSymbol->GetType()->IsRefType())
        {
            loadLocalInst = std::move(machine.CreateInst("loadref"));
            loadLocalInst->SetIndex(index);
        }
        else
        {
            switch (index)
            {
                case 0:  loadLocalInst = std::move(machine.CreateInst("loadlocal.0")); break;
                case 1:  loadLocalInst = std::move(machine.CreateInst("loadlocal.1")); break;
                case 2:  loadLocalInst = std::move(machine.CreateInst("loadlocal.2")); break;
                case 3:  loadLocalInst = std::move(machine.CreateInst("loadlocal.3")); break;
                default:
                {
                    if (index < 256)
                    {
                        loadLocalInst = std::move(machine.CreateInst("loadlocal.b"));
                        loadLocalInst->SetIndex(static_cast<uint8_t>(index));
                    }
                    else if (index < 65536)
                    {
                        loadLocalInst = std::move(machine.CreateInst("loadlocal.s"));
                        loadLocalInst->SetIndex(static_cast<uint16_t>(index));
                    }
                    else
                    {
                        loadLocalInst = std::move(machine.CreateInst("loadlocal"));
                        loadLocalInst->SetIndex(index);
                    }
                    break;
                }
            }
        }
        function.AddInst(std::move(loadLocalInst));
    }
    else
    {
        throw std::runtime_error("invalid parameter index");
    }
}

void BoundParameter::GenStore(Machine& machine, Function& function)
{
    std::unique_ptr<Instruction> storeLocalInst;
    int32_t index = parameterSymbol->Index();
    if (index != -1)
    {
        if (parameterSymbol->GetType()->IsRefType())
        {
            storeLocalInst = std::move(machine.CreateInst("storeref"));
            storeLocalInst->SetIndex(index);
        }
        else
        {
            switch (index)
            {
                case 0:  storeLocalInst = std::move(machine.CreateInst("storelocal.0")); break;
                case 1:  storeLocalInst = std::move(machine.CreateInst("storelocal.1")); break;
                case 2:  storeLocalInst = std::move(machine.CreateInst("storelocal.2")); break;
                case 3:  storeLocalInst = std::move(machine.CreateInst("storelocal.3")); break;
                default:
                {
                    if (index < 256)
                    {
                        storeLocalInst = std::move(machine.CreateInst("storelocal.b"));
                        storeLocalInst->SetIndex(static_cast<uint8_t>(index));
                    }
                    else if (index < 65536)
                    {
                        storeLocalInst = std::move(machine.CreateInst("storelocal.s"));
                        storeLocalInst->SetIndex(static_cast<uint16_t>(index));
                    }
                    else
                    {
                        storeLocalInst = std::move(machine.CreateInst("storelocal"));
                        storeLocalInst->SetIndex(index);
                    }
                    break;
                }
            }
        }
        function.AddInst(std::move(storeLocalInst));
    }
    else
    {
        throw std::runtime_error("invalid parameter index");
    }
}

void BoundParameter::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundArrayElement::BoundArrayElement(Assembly& assembly_, ArrayTypeSymbol* arrayTypeSymbol_, std::unique_ptr<BoundExpression>&& arr_, std::unique_ptr<BoundExpression>&& index_) :
    BoundExpression(assembly_, arrayTypeSymbol_->ElementType()), arr(std::move(arr_)), index(std::move(index_))
{
}

void BoundArrayElement::GenLoad(Machine& machine, Function& function)
{
    arr->GenLoad(machine, function);
    index->GenLoad(machine, function);
    std::unique_ptr<Instruction> loadElemInst = machine.CreateInst("loadarrayelem");
    function.AddInst(std::move(loadElemInst));
}

void BoundArrayElement::GenStore(Machine& machine, Function& function)
{
    arr->GenLoad(machine, function);
    index->GenLoad(machine, function);
    std::unique_ptr<Instruction> storeElemInst = machine.CreateInst("storearrayelem");
    function.AddInst(std::move(storeElemInst));
}

void BoundArrayElement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundStringChar::BoundStringChar(Assembly& assembly_, TypeSymbol* charType_, std::unique_ptr<BoundExpression>&& str_, std::unique_ptr<BoundExpression>&& index_) :
    BoundExpression(assembly_, charType_), str(std::move(str_)), index(std::move(index_))
{
}

void BoundStringChar::GenLoad(Machine& machine, Function& function)
{
    str->GenLoad(machine, function);
    index->GenLoad(machine, function);
    std::unique_ptr<Instruction> loadStringCharInst = machine.CreateInst("loadstringchar");
    function.AddInst(std::move(loadStringCharInst));
}

void BoundStringChar::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to bound string char");
}

void BoundStringChar::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundConversion::BoundConversion(Assembly& assembly_, std::unique_ptr<BoundExpression>&& sourceExpr_, FunctionSymbol* conversionFun_) :
    BoundExpression(assembly_, conversionFun_->ConversionTargetType()), sourceExpr(std::move(sourceExpr_)), conversionFun(conversionFun_)
{
}

void BoundConversion::GenLoad(Machine& machine, Function& function)
{
    sourceExpr->GenLoad(machine, function);
    std::vector<GenObject*> emptyObjects;
    conversionFun->GenerateCall(machine, GetAssembly(), function, emptyObjects, 0);
}

void BoundConversion::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to conversion");
}

void BoundConversion::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundNamespaceExpression::BoundNamespaceExpression(Assembly& assembly_, NamespaceSymbol* ns_) : BoundExpression(assembly_, nullptr), ns(ns_)
{
}

void BoundNamespaceExpression::GenLoad(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot load from namespace");
}

void BoundNamespaceExpression::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to namespace");
}

void BoundNamespaceExpression::Accept(BoundNodeVisitor& visitor)
{
    throw std::runtime_error("cannot visit bound namespace");
}

BoundTypeExpression::BoundTypeExpression(Assembly& assembly_, TypeSymbol* type_) : BoundExpression(assembly_, type_)
{
}

void BoundTypeExpression::GenLoad(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot load from type");
}

void BoundTypeExpression::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to type");
}

void BoundTypeExpression::Accept(BoundNodeVisitor& visitor)
{
    throw std::runtime_error("cannot visit bound type expression");
}

BoundFunctionGroupExpression::BoundFunctionGroupExpression(Assembly& assembly_, FunctionGroupSymbol* functionGroupSymbol_) : 
    BoundExpression(assembly_, new FunctionGroupTypeSymbol(functionGroupSymbol_)), functionGroupSymbol(functionGroupSymbol_)
{
    functionGroupType.reset(GetType());
}

void BoundFunctionGroupExpression::GenLoad(Machine& machine, Function& function)
{
    // Fun2Dlg conversion does not need source value, so this implementation is intentionally left empty.
}

void BoundFunctionGroupExpression::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to function group");
}

void BoundFunctionGroupExpression::Accept(BoundNodeVisitor& visitor)
{
    throw std::runtime_error("cannot visit bound function group");
}

void BoundFunctionGroupExpression::SetClassObject(std::unique_ptr<BoundExpression>&& classObject_)
{
    classObject = std::move(classObject_);
}

BoundMemberExpression::BoundMemberExpression(Assembly& assembly_, std::unique_ptr<BoundExpression>&& classObject_, std::unique_ptr<BoundExpression>&& member_) :
    BoundExpression(assembly_, new MemberExpressionTypeSymbol(member_->GetType()->NameConstant(), this)), classObject(std::move(classObject_)), member(std::move(member_))
{
    memberExpressionType.reset(GetType());
}

void BoundMemberExpression::GenLoad(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot load from bound member expression");
}

void BoundMemberExpression::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to from bound member expression");
}

void BoundMemberExpression::Accept(BoundNodeVisitor& visitor)
{
    throw std::runtime_error("cannot visit member expression");
}

BoundClassDelegateClassObjectPair::BoundClassDelegateClassObjectPair(Assembly& assembly_, std::unique_ptr<BoundExpression>&& newClassDelegateObjectExpr_,
    std::unique_ptr<BoundExpression>&& classObject_) : BoundExpression(assembly_, newClassDelegateObjectExpr_->GetType()),
    newClassDelegateObjectExpr(std::move(newClassDelegateObjectExpr_)), classObject(std::move(classObject_))
{
}

void BoundClassDelegateClassObjectPair::GenLoad(Machine& machine, Function& function)
{
    newClassDelegateObjectExpr->GenLoad(machine, function);
    classObject->GenLoad(machine, function);
}

void BoundClassDelegateClassObjectPair::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to from bound class delegate class object pair");
}

void BoundClassDelegateClassObjectPair::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundFunctionCall::BoundFunctionCall(Assembly& assembly_, FunctionSymbol* functionSymbol_) : 
    BoundExpression(assembly_, functionSymbol_->ReturnType()), functionSymbol(functionSymbol_), callType(FunctionCallType::regularCall)
{
}

void BoundFunctionCall::AddArgument(std::unique_ptr<BoundExpression>&& argument)
{
    arguments.push_back(std::move(argument));
}

void BoundFunctionCall::SetArguments(std::vector<std::unique_ptr<BoundExpression>>&& arguments_)
{
    arguments = std::move(arguments_);
}

void BoundFunctionCall::GenLoad(Machine& machine, Function& function)
{
    std::vector<GenObject*> objects;
    for (const std::unique_ptr<BoundExpression>& argument : arguments)
    {
        objects.push_back(argument.get());
    }
    switch (callType)
    {
        case FunctionCallType::regularCall:
        {
            functionSymbol->GenerateCall(machine, GetAssembly(), function, objects, 0);
            break;
        }
        case FunctionCallType::virtualCall:
        {
            MemberFunctionSymbol* memberFunctionSymbol = dynamic_cast<MemberFunctionSymbol*>(functionSymbol);
            Assert(memberFunctionSymbol, "member function symbol expected");
            memberFunctionSymbol->GenerateVirtualCall(machine, GetAssembly(), function, objects);
            break;
        }
        case FunctionCallType::interfaceCall:
        {
            MemberFunctionSymbol* memberFunctionSymbol = dynamic_cast<MemberFunctionSymbol*>(functionSymbol);
            Assert(memberFunctionSymbol, "member function symbol expected");
            memberFunctionSymbol->GenerateInterfaceCall(machine, GetAssembly(), function, objects);
            break;
        }
        default:
        {
            Assert(false, "unknown call type");
            break;
        }
    }
}

void BoundFunctionCall::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to function call");
}

void BoundFunctionCall::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundDelegateCall::BoundDelegateCall(Assembly& assembly_, DelegateTypeSymbol* delegateTypeSymbol_, std::vector<std::unique_ptr<BoundExpression>>&& arguments_) :
    BoundExpression(assembly_, delegateTypeSymbol_->GetReturnType()), delegateTypeSymbol(delegateTypeSymbol_), arguments(std::move(arguments_))
{
}

void BoundDelegateCall::GenLoad(Machine& machine, Function& function)
{
    int n = int(arguments.size());
    for (int i = 0; i < n; ++i)
    {
        GenObject* genObject = arguments[i].get();
        genObject->GenLoad(machine, function);
    }
    InstIndexRequest startDelegateCall;
    function.GetEmitter()->AddIndexRequest(&startDelegateCall);
    std::unique_ptr<Instruction> inst = machine.CreateInst("calld");
    function.AddInst(std::move(inst));
    function.GetEmitter()->BackpatchConDisSet(startDelegateCall.Index());
    function.MapPCToSourceLine(startDelegateCall.Index(), function.GetEmitter()->CurrentSourceLine());
}

void BoundDelegateCall::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to delegate call");
}

void BoundDelegateCall::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundClassDelegateCall::BoundClassDelegateCall(Assembly& assembly_, ClassDelegateTypeSymbol* classDelegateTypeSymbol_, std::vector<std::unique_ptr<BoundExpression>>&& arguments_) :
    BoundExpression(assembly_, classDelegateTypeSymbol_->GetReturnType()), classDelegateTypeSymbol(classDelegateTypeSymbol_), arguments(std::move(arguments_))
{
}

void BoundClassDelegateCall::GenLoad(Machine& machine, Function& function)
{
    int n = int(arguments.size());
    for (int i = 0; i < n; ++i)
    {
        GenObject* genObject = arguments[i].get();
        genObject->GenLoad(machine, function);
    }
    InstIndexRequest startClassDelegateCall;
    function.GetEmitter()->AddIndexRequest(&startClassDelegateCall);
    std::unique_ptr<Instruction> inst = machine.CreateInst("callcd");
    function.AddInst(std::move(inst));
    function.GetEmitter()->BackpatchConDisSet(startClassDelegateCall.Index());
    function.MapPCToSourceLine(startClassDelegateCall.Index(), function.GetEmitter()->CurrentSourceLine());
}

void BoundClassDelegateCall::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to class delegate call");
}

void BoundClassDelegateCall::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundNewExpression::BoundNewExpression(BoundFunctionCall* boundFunctionCall_, TypeSymbol* type_) : BoundExpression(boundFunctionCall_->GetAssembly(), type_),
    functionSymbol(boundFunctionCall_->GetFunctionSymbol()), arguments(std::move(boundFunctionCall_->ReleaseArguments()))
{
}

void BoundNewExpression::GenLoad(Machine& machine, Function& function) 
{
    std::unique_ptr<Instruction> createInst;
    createInst = machine.CreateInst("createo");
    ConstantPool& constantPool = GetAssembly().GetConstantPool();
    ClassTypeSymbol* containingClass = functionSymbol->ContainingClass();
    Assert(containingClass, "containing class not found");
    utf32_string fullClassName = containingClass->FullName();
    Constant fullClassNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(fullClassName.c_str())));
    CreateObjectInst* createObjectInst = dynamic_cast<CreateObjectInst*>(createInst.get());
    Assert(createObjectInst, "CreateObject instruction expected");
    createObjectInst->SetTypeName(fullClassNameConstant);
    function.AddInst(std::move(createInst));
    std::unique_ptr<Instruction> dupInst = machine.CreateInst("dup");
    function.AddInst(std::move(dupInst));
    std::vector<GenObject*> objects;
    for (const std::unique_ptr<BoundExpression>& argument : arguments)
    {
        objects.push_back(argument.get());
    }
    functionSymbol->GenerateCall(machine, GetAssembly(), function, objects, 1);
}

void BoundNewExpression::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to new expression");
}

void BoundNewExpression::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundBooleanBinaryExpression::BoundBooleanBinaryExpression(Assembly& assembly_, BoundExpression* left_, BoundExpression* right_) : BoundExpression(assembly_, left_->GetType()), left(left_), right(right_)
{
}

BoundConjunction::BoundConjunction(Assembly& assembly_, BoundExpression* left_, BoundExpression* right_) : BoundBooleanBinaryExpression(assembly_, left_, right_)
{
}

void BoundConjunction::GenLoad(Machine& machine, Function& function)
{
    function.GetEmitter()->Visit(*this);
}

void BoundConjunction::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to bound conjunction");
}

void BoundConjunction::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundDisjunction::BoundDisjunction(Assembly& assembly_, BoundExpression* left_, BoundExpression* right_) : BoundBooleanBinaryExpression(assembly_, left_, right_)
{
}

void BoundDisjunction::GenLoad(Machine& machine, Function& function)
{
    function.GetEmitter()->Visit(*this);
}

void BoundDisjunction::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to bound disjunction");
}

void BoundDisjunction::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundIsExpression::BoundIsExpression(Assembly& assembly_, std::unique_ptr<BoundExpression>&& expr_, TypeSymbol* classType_) :
    BoundExpression(assembly_, assembly_.GetSymbolTable().GetType(U"System.Boolean")), expr(std::move(expr_)), classType(classType_)
{
    ConstantPool& constantPool = GetAssembly().GetConstantPool();
    utf32_string classTypeFullName = classType->FullName();
    constantPool.Install(StringPtr(classTypeFullName.c_str()));
}

void BoundIsExpression::GenLoad(Machine& machine, Function& function)
{
    expr->GenLoad(machine, function);
    std::unique_ptr<Instruction> inst = machine.CreateInst("is");
    IsInst* isInst = dynamic_cast<IsInst*>(inst.get());
    Assert(isInst, "is instruction expected");
    ConstantPool& constantPool = GetAssembly().GetConstantPool();
    utf32_string classTypeFullName = classType->FullName();
    ConstantId classTypeId = constantPool.GetIdFor(classTypeFullName);
    Constant classTypeNameConstant = constantPool.GetConstant(classTypeId);
    isInst->SetTypeName(classTypeNameConstant);
    function.AddInst(std::move(inst));
}

void BoundIsExpression::GenStore(Machine& machine, Function& function) 
{
    throw std::runtime_error("cannot store to bound is expression");
}

void BoundIsExpression::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundAsExpression::BoundAsExpression(Assembly& assembly_, std::unique_ptr<BoundExpression>&& expr_, TypeSymbol* classType_) :
    BoundExpression(assembly_, classType_), expr(std::move(expr_)), classType(classType_)
{
    ConstantPool& constantPool = GetAssembly().GetConstantPool();
    utf32_string classTypeFullName = classType->FullName();
    constantPool.Install(StringPtr(classTypeFullName.c_str()));
}

void BoundAsExpression::GenLoad(Machine& machine, Function& function)
{
    expr->GenLoad(machine, function);
    std::unique_ptr<Instruction> inst = machine.CreateInst("as");
    AsInst* asInst = dynamic_cast<AsInst*>(inst.get());
    Assert(asInst, "as instruction expected");
    ConstantPool& constantPool = GetAssembly().GetConstantPool();
    utf32_string classTypeFullName = classType->FullName();
    ConstantId classTypeId = constantPool.GetIdFor(classTypeFullName);
    Constant classTypeNameConstant = constantPool.GetConstant(classTypeId);
    asInst->SetTypeName(classTypeNameConstant);
    function.AddInst(std::move(inst));
}

void BoundAsExpression::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to bound as expression");
}

void BoundAsExpression::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundLocalRefExpression::BoundLocalRefExpression(Assembly& assembly_, int32_t localIndex_, TypeSymbol* refType_) : BoundExpression(assembly_, refType_), localIndex(localIndex_)
{
}

void BoundLocalRefExpression::GenLoad(Machine& machine, Function& function)
{
    std::unique_ptr<Instruction> inst = machine.CreateInst("createlocalref");
    CreateLocalVariableReferenceInst* createLocalVarRefInst = dynamic_cast<CreateLocalVariableReferenceInst*>(inst.get());
    createLocalVarRefInst->SetLocalIndex(localIndex);
    function.AddInst(std::move(inst));
}

void BoundLocalRefExpression::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to bound local ref expression");
}

void BoundLocalRefExpression::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundMemberVarRefExpression::BoundMemberVarRefExpression(Assembly& assembly_, std::unique_ptr<BoundExpression>&& classObject_, int32_t memberVarIndex_, TypeSymbol* refType_) :
    BoundExpression(assembly_, refType_), classObject(std::move(classObject_)), memberVarIndex(memberVarIndex_)
{
}

void BoundMemberVarRefExpression::GenLoad(Machine& machine, Function& function)
{
    classObject->GenLoad(machine, function);
    std::unique_ptr<Instruction> inst = machine.CreateInst("createfieldref");
    CreateMemberVariableReferenceInst* createMemberVarRefInst = dynamic_cast<CreateMemberVariableReferenceInst*>(inst.get());
    createMemberVarRefInst->SetMemberVarIndex(memberVarIndex);
    function.AddInst(std::move(inst));
}

void BoundMemberVarRefExpression::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to bound member var ref expression");
}

void BoundMemberVarRefExpression::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::binder
