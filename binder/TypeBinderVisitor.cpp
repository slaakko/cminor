// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/TypeBinderVisitor.hpp>
#include <cminor/binder/BoundCompileUnit.hpp>
#include <cminor/binder/TypeResolver.hpp>
#include <cminor/binder/Evaluator.hpp>
#include <cminor/ast/CompileUnit.hpp>
#include <cminor/symbols/FunctionSymbol.hpp>
#include <cminor/symbols/ConstantSymbol.hpp>
#include <cminor/symbols/DelegateSymbol.hpp>
#include <cminor/symbols/VariableSymbol.hpp>
#include <cminor/symbols/PropertySymbol.hpp>
#include <cminor/symbols/IndexerSymbol.hpp>
#include <cminor/symbols/EnumTypeFun.hpp>
#include <cminor/symbols/ObjectFun.hpp>
#include <cminor/symbols/SymbolCreatorVisitor.hpp>
#include <cminor/symbols/Warning.hpp>
#include <cminor/machine/Class.hpp>
#include <iostream>

namespace cminor { namespace binder {

TypeBinderVisitor::TypeBinderVisitor(BoundCompileUnit& boundCompileUnit_) : boundCompileUnit(boundCompileUnit_), containerScope(nullptr), instantiateRequested(false), enumType(nullptr),
    sourceFilePathConstant()
{
    if (boundCompileUnit.GetCompileUnitNode())
    {
        sourceFilePathConstant = boundCompileUnit.GetAssembly().RegisterSourceFilePath(boundCompileUnit.GetCompileUnitNode()->FilePath());
    }
}

void TypeBinderVisitor::Visit(CompileUnitNode& compileUnitNode)
{
    boundCompileUnit.AddFileScope(std::unique_ptr<FileScope>(new FileScope()));
    compileUnitNode.GlobalNs()->Accept(*this);
}

void TypeBinderVisitor::Visit(NamespaceNode& namespaceNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(namespaceNode);
    containerScope = symbol->GetContainerScope();
    int n = namespaceNode.Members().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* member = namespaceNode.Members()[i];
        member->Accept(*this);
    }
    containerScope = prevContainerScope;
}

void TypeBinderVisitor::Visit(NamespaceImportNode& namespaceImportNode)
{
    boundCompileUnit.FirstFileScope()->InstallNamespaceImport(containerScope, &namespaceImportNode);
    usingNodes.push_back(&namespaceImportNode);
}

void TypeBinderVisitor::Visit(AliasNode& aliasNode)
{
    boundCompileUnit.FirstFileScope()->InstallAlias(containerScope, &aliasNode);
    usingNodes.push_back(&aliasNode);
}

void TypeBinderVisitor::Visit(FunctionNode& functionNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(functionNode);
    FunctionSymbol* functionSymbol = dynamic_cast<FunctionSymbol*>(symbol);
    Assert(functionSymbol, "function symbol expected");
    containerScope = functionSymbol->GetContainerScope();
    Specifiers specifiers = functionNode.GetSpecifiers();
    functionSymbol->SetSpecifiers(specifiers);
    if (functionSymbol->IsExternal())
    {
        if (functionNode.HasBody())
        {
            throw Exception("external function cannot have a body", functionNode.GetSpan());
        }
        utf32_string vmf = ToUtf32(functionNode.Attributes().GetAttribute("vmf"));
        if (vmf.empty())
        {
            throw Exception("virtual machine function name attribute ('vmf') not set for external function", functionNode.GetSpan());
        }
        functionSymbol->SetVmf(StringPtr(vmf.c_str()));
    }
    int n = functionNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = functionNode.Parameters()[i];
        TypeSymbol* parameterType = ResolveType(boundCompileUnit, containerScope, parameterNode->TypeExpr(), TypeResolverFlags::parameterType);
        Symbol* ps = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(*parameterNode);
        ParameterSymbol* parameterSymbol = dynamic_cast<ParameterSymbol*>(ps);
        Assert(parameterSymbol, "parameter symbol expected");
        parameterSymbol->SetType(parameterType);
    }
    TypeSymbol* returnType = ResolveType(boundCompileUnit, containerScope, functionNode.ReturnTypeExpr());
    functionSymbol->SetReturnType(returnType);
    functionSymbol->ComputeName();
    if (functionNode.HasBody())
    {
        functionNode.Body()->Accept(*this);
    }
    else
    {
        if (!functionSymbol->IsExternal())
        {
            throw Exception("function has no body", functionSymbol->GetSpan());
        }
    }
    containerScope = prevContainerScope;
}

void TypeBinderVisitor::Visit(ConstantNode& constantNode)
{
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(constantNode);
    ConstantSymbol* constantSymbol = dynamic_cast<ConstantSymbol*>(symbol);
    Assert(constantSymbol, "constant symbol expected");
    if (constantSymbol->IsBound()) return;
    constantSymbol->SetBound();
    constantSymbol->SetSpecifiers(constantNode.GetSpecifiers());
    TypeSymbol* type = ResolveType(boundCompileUnit, containerScope, constantNode.TypeExpr());
    constantSymbol->SetType(type);
    Value* value = Evaluate(GetValueKindFor(type->GetSymbolType(), constantNode.GetSpan()), false, containerScope, boundCompileUnit, constantNode.Value());
    constantSymbol->SetValue(value);
}

void TypeBinderVisitor::Visit(EnumTypeNode& enumTypeNode)
{
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(enumTypeNode);
    EnumTypeSymbol* enumTypeSymbol = dynamic_cast<EnumTypeSymbol*>(symbol);
    Assert(enumTypeSymbol, "enum type symbol expected");
    if (enumTypeSymbol->IsBound()) return;
    enumTypeSymbol->SetBound();
    EnumTypeSymbol* prevEnumType = enumType;
    enumType = enumTypeSymbol;
    enumTypeSymbol->SetSpecifiers(enumTypeNode.GetSpecifiers());
    TypeSymbol* underlyingType = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Int32");
    if (enumTypeNode.GetUnderlyingType())
    {
        underlyingType = ResolveType(boundCompileUnit, containerScope, enumTypeNode.GetUnderlyingType());
    }
    enumTypeSymbol->SetUnderlyingType(underlyingType);
    ContainerScope* prevContainerScope = containerScope;
    containerScope = enumTypeSymbol->GetContainerScope();
    int n = enumTypeNode.Constants().Count();
    for (int i = 0; i < n; ++i)
    {
        EnumConstantNode* enumConstantNode = enumTypeNode.Constants()[i];
        enumConstantNode->Accept(*this);
    }
    CreateEnumFun(boundCompileUnit.GetAssembly(), enumTypeSymbol);
    containerScope = prevContainerScope;
    enumType = prevEnumType;
}

void TypeBinderVisitor::Visit(EnumConstantNode& enumConstantNode)
{
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(enumConstantNode);
    EnumConstantSymbol* enumConstantSymbol = dynamic_cast<EnumConstantSymbol*>(symbol);
    Assert(enumConstantSymbol, "enum constant expected");
    enumConstantSymbol->SetEvaluating();
    Value* value = Evaluate(GetValueKindFor(enumType->GetUnderlyingType()->GetSymbolType(), enumConstantNode.GetSpan()), false, containerScope, boundCompileUnit, enumConstantNode.GetValue());
    enumConstantSymbol->SetValue(value);
    enumConstantSymbol->ResetEvaluating();
}

void TypeBinderVisitor::BindClass(ClassTypeSymbol* classTypeSymbol, ClassNode& classNode)
{
    if (classTypeSymbol->IsBound()) return;
    classTypeSymbol->SetBound();
    if (sourceFilePathConstant.Value().AsStringLiteral())
    {
        classTypeSymbol->SetSourceFilePathConstant(sourceFilePathConstant);
    }
    if (instantiateRequested)
    {
        ContainerScope* prevContainerScope = containerScope;
        containerScope = classTypeSymbol->GetContainerScope();
        int nm = classNode.Members().Count();
        for (int i = 0; i < nm; ++i)
        {
            Node* member = classNode.Members()[i];
            member->Accept(*this);
        }
        containerScope = prevContainerScope;
        return;
    }
    if (classTypeSymbol->IsClassTemplate())
    {
        classTypeSymbol->CloneUsingNodes(usingNodes);
        return;
    }
    classTypeSymbol->SetSpecifiers(classNode.GetSpecifiers());
    utf32_string fullName = classTypeSymbol->FullName();
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    Constant fullNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(fullName.c_str())));
    classTypeSymbol->GetObjectType()->SetNameConstant(fullNameConstant);
    ContainerScope* prevContainerScope = containerScope;
    containerScope = classTypeSymbol->GetContainerScope();
    int nb = classNode.BaseClassOrInterfaces().Count();
    for (int i = 0; i < nb; ++i)
    {
        Node* baseOrInterfaceNode = classNode.BaseClassOrInterfaces()[i];
        TypeSymbol* baseOrInterfaceSymbol = ResolveType(boundCompileUnit, containerScope, baseOrInterfaceNode);
        if (ClassTypeSymbol* baseClassSymbol = dynamic_cast<ClassTypeSymbol*>(baseOrInterfaceSymbol))
        {
            if (baseClassSymbol->IsProject())
            {
                Node* node = boundCompileUnit.GetAssembly().GetSymbolTable().GetNode(baseClassSymbol);
                ClassNode* baseClassNode = dynamic_cast<ClassNode*>(node);
                Assert(baseClassNode, "class node expected");
                BindClass(baseClassSymbol, *baseClassNode);
            }
            else
            {
                constantPool.Install(baseClassSymbol->Name());
            }
            if (classTypeSymbol->BaseClass())
            {
                throw Exception("class type can have at most one base class", classTypeSymbol->GetSpan(), baseClassSymbol->GetSpan());
            }
            else if (baseClassSymbol == classTypeSymbol)
            {
                throw Exception("class cannot derive from itself", classTypeSymbol->GetSpan());
            }
            else
            {
                classTypeSymbol->SetBaseClass(baseClassSymbol);
            }
        }
        else if (InterfaceTypeSymbol* interfaceTypeSymbol = dynamic_cast<InterfaceTypeSymbol*>(baseOrInterfaceSymbol))
        {
            if (interfaceTypeSymbol->IsProject())
            {
                Node* node = boundCompileUnit.GetAssembly().GetSymbolTable().GetNode(interfaceTypeSymbol);
                InterfaceNode* intfNode = dynamic_cast<InterfaceNode*>(node);
                Assert(intfNode, "interface node expected");
                BindInterface(interfaceTypeSymbol, *intfNode);
            }
            classTypeSymbol->AddImplementedInterface(interfaceTypeSymbol);
        }
        else
        {
            throw Exception("symbol '" + ToUtf8(baseOrInterfaceSymbol->FullName()) + "' is not a class or interface type symbol", baseOrInterfaceNode->GetSpan(), baseOrInterfaceSymbol->GetSpan());
        }
    }
    int nm = classNode.Members().Count();
    for (int i = 0; i < nm; ++i)
    {
        Node* member = classNode.Members()[i];
        member->Accept(*this);
    }
    if (!classTypeSymbol->BaseClass())
    {
        TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Object");
        ClassTypeSymbol* objectType = dynamic_cast<ClassTypeSymbol*>(type);
        Assert(objectType, "object expected");
        if (objectType != classTypeSymbol)
        {
            Assert(objectType, "class type symbol expected");
            if (objectType->IsProject())
            {
                Node* node = boundCompileUnit.GetAssembly().GetSymbolTable().GetNode(objectType);
                ClassNode* objectTypeNode = dynamic_cast<ClassNode*>(node);
                BindClass(objectType, *objectTypeNode);
            }
            classTypeSymbol->SetBaseClass(objectType);
        }
        else
        {
            objectType->GetObjectType()->AddField(ValueType::classDataPtr);
        }
    }
    if (classTypeSymbol->BaseClass())
    {
        classTypeSymbol->GetContainerScope()->SetBase(classTypeSymbol->BaseClass()->GetContainerScope());
        ObjectType* baseClassObjectType = classTypeSymbol->BaseClass()->GetObjectType();
        classTypeSymbol->GetObjectType()->AddFields(baseClassObjectType->Fields());
    }
    if (!classTypeSymbol->DefaultConstructorSymbol() && !classTypeSymbol->DontCreateDefaultConstructor())
    {
        CreateDefaultConstructor(boundCompileUnit.GetAssembly(), classTypeSymbol);
    }
    CreateBasicTypeObjectFun(boundCompileUnit.GetAssembly(), classTypeSymbol);
    classTypeSymbol->InitVmt();
    classTypeSymbol->InitImts();
    int nmv = int(classTypeSymbol->MemberVariables().size());
    for (int i = 0; i < nmv; ++i)
    {
        MemberVariableSymbol* memberVariableSymbol = classTypeSymbol->MemberVariables()[i];
        memberVariableSymbol->SetIndex(classTypeSymbol->GetObjectType()->FieldCount());
        TypeSymbol* memberVariableType = memberVariableSymbol->GetType();
        Assert(memberVariableType, "got no type");
        ValueType memberVariableValueType = memberVariableType->GetValueType();
        classTypeSymbol->GetObjectType()->AddField(memberVariableValueType);
    }
    int nsmv = int(classTypeSymbol->StaticMemberVariables().size());
    if (nsmv > 0)
    {
        ClassData* classData = classTypeSymbol->GetClassData();
        if (!classData->GetStaticClassData())
        {
            classData->CreateStaticClassData();
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        Layout& staticLayout = staticClassData->StaticLayout();
        for (int i = 0; i < nsmv; ++i)
        {
            MemberVariableSymbol* staticMemberVariableSymbol = classTypeSymbol->StaticMemberVariables()[i];
            staticMemberVariableSymbol->SetIndex(staticLayout.FieldCount());
            TypeSymbol* staticMemberVariableType = staticMemberVariableSymbol->GetType();
            Assert(staticMemberVariableType, "got no type");
            ValueType memberVariableValueType = staticMemberVariableType->GetValueType();
            staticLayout.AddField(memberVariableValueType);
        }
    }
    if (classTypeSymbol->FullName() == U"System.String")
    {
        classTypeSymbol->GetObjectType()->AddField(ValueType::allocationHandle);
    }
    containerScope = prevContainerScope;
}

void TypeBinderVisitor::Visit(ClassNode& classNode)
{
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(classNode);
    ClassTypeSymbol* classTypeSymbol = dynamic_cast<ClassTypeSymbol*>(symbol);
    Assert(classTypeSymbol, "class type symbol expected");
    BindClass(classTypeSymbol, classNode);
}

void TypeBinderVisitor::BindInterface(InterfaceTypeSymbol* interfaceTypeSymbol, InterfaceNode& interfaceNode)
{
    if (interfaceTypeSymbol->IsBound()) return;
    interfaceTypeSymbol->SetBound();
    interfaceTypeSymbol->SetSpecifiers(interfaceNode.GetSpecifiers());
    ContainerScope* prevContainerScope = containerScope;
    containerScope = interfaceTypeSymbol->GetContainerScope();
    ObjectType* objectType = interfaceTypeSymbol->GetObjectType();
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    utf32_string fullName = interfaceTypeSymbol->FullName();
    Constant fullNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(fullName.c_str())));
    objectType->SetNameConstant(fullNameConstant);
    objectType->AddField(ValueType::objectReference);
    objectType->AddField(ValueType::intType);
    int nm = interfaceNode.Members().Count();
    for (int i = 0; i < nm; ++i)
    {
        Node* member = interfaceNode.Members()[i];
        member->Accept(*this);
    }
    CreateBasicTypeObjectFun(boundCompileUnit.GetAssembly(), interfaceTypeSymbol);
    containerScope = prevContainerScope;
}

void TypeBinderVisitor::Visit(InterfaceNode& interfaceNode)
{
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(interfaceNode);
    InterfaceTypeSymbol* interfaceTypeSymbol = dynamic_cast<InterfaceTypeSymbol*>(symbol);
    Assert(interfaceTypeSymbol, "interface type expected");
    BindInterface(interfaceTypeSymbol, interfaceNode);
}

void TypeBinderVisitor::Visit(StaticConstructorNode& staticConstructorNode)
{
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(staticConstructorNode);
    StaticConstructorSymbol* staticConstructorSymbol = dynamic_cast<StaticConstructorSymbol*>(symbol);
    Assert(staticConstructorSymbol, "static constructor function symbol expected");
    ContainerScope* prevContainerScope = containerScope;
    containerScope = staticConstructorSymbol->GetContainerScope();
    if (instantiateRequested)
    {
        if (!staticConstructorSymbol->IsInstantiationRequested())
        {
            return;
        }
    }
    else
    {
        staticConstructorSymbol->SetSpecifiers(staticConstructorNode.GetSpecifiers());
        if (staticConstructorSymbol->IsExternal())
        {
            if (staticConstructorNode.HasBody())
            {
                throw Exception("external function cannot have a body", staticConstructorNode.GetSpan());
            }
            utf32_string vmf = ToUtf32(staticConstructorNode.Attributes().GetAttribute("vmf"));
            if (vmf.empty())
            {
                throw Exception("virtual machine function name attribute ('vmf') not set for external function", staticConstructorNode.GetSpan());
            }
            staticConstructorSymbol->SetVmf(StringPtr(vmf.c_str()));
        }
        staticConstructorSymbol->ComputeName();
    }
    Symbol* parent = staticConstructorSymbol->Parent();
    ClassTypeSymbol* classTypeSymbol = dynamic_cast<ClassTypeSymbol*>(parent);
    Assert(classTypeSymbol, "class type symbol expected");
    ClassData* classData = classTypeSymbol->GetClassData();
    if (!classData->GetStaticClassData())
    {
        classData->CreateStaticClassData();
    }
    StaticClassData* staticClassData = classData->GetStaticClassData();
    utf32_string fullName = staticConstructorSymbol->FullName();
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    Constant fullNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(fullName.c_str())));
    staticConstructorSymbol->SetFullNameConstant(fullNameConstant);
    staticClassData->SetStaticConstructorName(fullNameConstant);
    if (staticConstructorNode.HasBody())
    {
        staticConstructorNode.Body()->Accept(*this);
    }
    else
    {
        if (!staticConstructorSymbol->IsExternal() && dynamic_cast<ClassTemplateSpecializationSymbol*>(staticConstructorSymbol->ContainingClass()) == nullptr)
        {
            throw Exception("static constructor has no body", staticConstructorSymbol->GetSpan());
        }
    }
    containerScope = prevContainerScope;
    if (staticConstructorSymbol->IsExternal() || staticConstructorNode.HasBody())
    {
        staticConstructorSymbol->SetTypesResolved();
    }
}

void TypeBinderVisitor::Visit(ConstructorNode& constructorNode)
{
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(constructorNode);
    ConstructorSymbol* constructorSymbol = dynamic_cast<ConstructorSymbol*>(symbol);
    Assert(constructorSymbol, "constructor function symbol expected");
    ContainerScope* prevContainerScope = containerScope;
    containerScope = constructorSymbol->GetContainerScope();
    if (instantiateRequested)
    {
        if (!constructorSymbol->IsInstantiationRequested())
        {
            return;
        }
    }
    else
    {
        constructorSymbol->SetSpecifiers(constructorNode.GetSpecifiers());
        if (constructorSymbol->IsExternal())
        {
            if (constructorNode.HasBody())
            {
                throw Exception("external function cannot have a body", constructorNode.GetSpan());
            }
            utf32_string vmf = ToUtf32(constructorNode.Attributes().GetAttribute("vmf"));
            if (vmf.empty())
            {
                throw Exception("virtual machine function name attribute ('vmf') not set for external function", constructorNode.GetSpan());
            }
            constructorSymbol->SetVmf(StringPtr(vmf.c_str()));
        }
        int n = constructorNode.Parameters().Count();
        for (int i = 0; i < n; ++i)
        {
            ParameterNode* parameterNode = constructorNode.Parameters()[i];
            TypeSymbol* parameterType = ResolveType(boundCompileUnit, containerScope, parameterNode->TypeExpr(), TypeResolverFlags::parameterType);
            Symbol* ps = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(*parameterNode);
            ParameterSymbol* parameterSymbol = dynamic_cast<ParameterSymbol*>(ps);
            Assert(parameterSymbol, "parameter symbol expected");
            parameterSymbol->SetType(parameterType);
        }
        constructorSymbol->ComputeName();
        if (constructorSymbol->IsDefaultConstructorSymbol())
        {
            constructorSymbol->SetBound();
        }
    }
    Symbol* parent = constructorSymbol->Parent();
    if (parent->IsStatic())
    {
        throw Exception("static class cannot contain instance constructors", constructorSymbol->GetSpan(), parent->GetSpan());
    }
    if (constructorNode.HasBody())
    {
        constructorNode.Body()->Accept(*this);
    }
    else
    {
        if (!constructorSymbol->IsExternal() && dynamic_cast<ClassTemplateSpecializationSymbol*>(constructorSymbol->ContainingClass()) == nullptr)
        {
            throw Exception("constructor has no body", constructorSymbol->GetSpan());
        }
    }
    containerScope = prevContainerScope;
    if (constructorSymbol->IsExternal() || constructorNode.HasBody())
    {
        constructorSymbol->SetTypesResolved();
    }
}

void TypeBinderVisitor::Visit(MemberFunctionNode& memberFunctionNode)
{
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(memberFunctionNode);
    MemberFunctionSymbol* memberFunctionSymbol = dynamic_cast<MemberFunctionSymbol*>(symbol);
    Assert(memberFunctionSymbol, "member function symbol expected");
    ContainerScope* prevContainerScope = containerScope;
    containerScope = memberFunctionSymbol->GetContainerScope();
    if (instantiateRequested)
    {
        if (!memberFunctionSymbol->IsInstantiationRequested())
        {
            return;
        }
    }
    else
    {
        memberFunctionSymbol->SetSpecifiers(memberFunctionNode.GetSpecifiers());
        if (memberFunctionSymbol->IsExternal())
        {
            if (memberFunctionNode.HasBody())
            {
                throw Exception("external function cannot have a body", memberFunctionNode.GetSpan());
            }
            utf32_string vmf = ToUtf32(memberFunctionNode.Attributes().GetAttribute("vmf"));
            if (vmf.empty())
            {
                throw Exception("virtual machine function name attribute ('vmf') not set for external function", memberFunctionNode.GetSpan());
            }
            memberFunctionSymbol->SetVmf(StringPtr(vmf.c_str()));
        }
        if (!memberFunctionSymbol->IsVirtualAbstractOrOverride() && !memberFunctionSymbol->IsNew())
        {
            ClassTypeSymbol* ownerClass = dynamic_cast<ClassTypeSymbol*>(memberFunctionSymbol->Parent());
            if (ownerClass)
            {
                if (ownerClass->BaseClass())
                {
                    ClassTypeSymbol* baseClass = ownerClass->BaseClass();
                    for (MemberFunctionSymbol* f : baseClass->MemberFunctions())
                    {
                        if (f->IsVirtualAbstractOrOverride())
                        {
                            if (Overrides(memberFunctionSymbol, f))
                            {
                                std::string warningMessage = "member function '" + ToUtf8(memberFunctionSymbol->FullNameWithSpecifiers()) + "' hides base class virtual function '" + ToUtf8(f->FullNameWithSpecifiers()) + "'. " +
                                    "To get rid of this warning declare the function either 'override' or 'new'.";
                                Warning warning(CompileWarningCollection::Instance().GetCurrentProjectName(), warningMessage);
                                warning.SetDefined(memberFunctionSymbol->GetSpan());
                                warning.SetReferenced(f->GetSpan());
                                CompileWarningCollection::Instance().AddWarning(warning);
                            }
                        }
                    }
                }
            }
        }
        int n = memberFunctionNode.Parameters().Count();
        for (int i = 0; i < n; ++i)
        {
            ParameterNode* parameterNode = memberFunctionNode.Parameters()[i];
            TypeSymbol* parameterType = ResolveType(boundCompileUnit, containerScope, parameterNode->TypeExpr(), TypeResolverFlags::parameterType);
            Symbol* ps = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(*parameterNode);
            ParameterSymbol* parameterSymbol = dynamic_cast<ParameterSymbol*>(ps);
            Assert(parameterSymbol, "parameter symbol expected");
            parameterSymbol->SetType(parameterType);
        }
        TypeSymbol* returnType = ResolveType(boundCompileUnit, containerScope, memberFunctionNode.ReturnTypeExpr());
        memberFunctionSymbol->SetReturnType(returnType);
        memberFunctionSymbol->ComputeName();
    }
    Symbol* parent = memberFunctionSymbol->Parent();
    if (parent->IsStatic() && !memberFunctionSymbol->IsStatic())
    {
        throw Exception("static class cannot contain nonstatic member functions", memberFunctionSymbol->GetSpan(), parent->GetSpan());
    }
    if (memberFunctionNode.HasBody())
    {
        memberFunctionNode.Body()->Accept(*this);
    }
    else
    {
        if (!memberFunctionSymbol->ContainingInterface() && !memberFunctionSymbol->IsAbstract() && !memberFunctionSymbol->IsExternal() && 
            dynamic_cast<ClassTemplateSpecializationSymbol*>(memberFunctionSymbol->ContainingClass()) == nullptr)
        {
            throw Exception("member function has no body", memberFunctionSymbol->GetSpan());
        }
    }
    containerScope = prevContainerScope;
    if (memberFunctionSymbol->ContainingInterface() || memberFunctionSymbol->IsAbstract() || memberFunctionSymbol->IsExternal() || memberFunctionNode.HasBody())
    {
        memberFunctionSymbol->SetTypesResolved();
    }
}

void TypeBinderVisitor::Visit(MemberVariableNode& memberVariableNode)
{
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(memberVariableNode);
    MemberVariableSymbol* memberVariableSymbol = dynamic_cast<MemberVariableSymbol*>(symbol);
    Assert(memberVariableSymbol, "member variable symbol expected");
    memberVariableSymbol->SetSpecifiers(memberVariableNode.GetSpecifiers());
    Symbol* parent = memberVariableSymbol->Parent();
    if (parent->IsStatic() && !memberVariableSymbol->IsStatic())
    {
        throw Exception("static class cannot contain instance variables", memberVariableSymbol->GetSpan(), parent->GetSpan());
    }
    TypeSymbol* memberVariableType = ResolveType(boundCompileUnit, containerScope, memberVariableNode.TypeExpr());
    memberVariableSymbol->SetType(memberVariableType);
}

void TypeBinderVisitor::Visit(PropertyNode& propertyNode)
{
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(propertyNode);
    PropertySymbol* propertySymbol = dynamic_cast<PropertySymbol*>(symbol);
    Assert(propertySymbol, "property symbol expected");
    propertySymbol->SetSpecifiers(propertyNode.GetSpecifiers());
    Symbol* parent = propertySymbol->Parent();
    if (parent->IsStatic() && !propertySymbol->IsStatic())
    {
        throw Exception("static class cannot contain instance properties", propertySymbol->GetSpan(), parent->GetSpan());
    }
    ContainerScope* prevContainerScope = containerScope;
    containerScope = propertySymbol->GetContainerScope();
    TypeSymbol* propertyType = ResolveType(boundCompileUnit, containerScope, propertyNode.TypeExpr());
    propertySymbol->SetType(propertyType);
    PropertyGetterFunctionSymbol* getter = propertySymbol->Getter();
    PropertySetterFunctionSymbol* setter = propertySymbol->Setter();
    if (!getter && !setter)
    {
        throw Exception("property '" + ToUtf8(propertySymbol->FullName()) + "' must have either getter or setter or both", propertySymbol->GetSpan());
    }
    bool instantiateGetter = true;
    if (instantiateRequested)
    {
        if (getter)
        {
            if (getter->IsInstantiated() || !getter->IsInstantiationRequested())
            {
                instantiateGetter = false;
            }
        }
    }
    if (getter && instantiateGetter)
    {
        getter->SetReturnType(propertyType);
        prevContainerScope = containerScope;
        containerScope = getter->GetContainerScope();
        propertyNode.Getter()->Accept(*this);
        containerScope = prevContainerScope;
        getter->SetTypesResolved();
    }
    bool instantiateSetter = true;
    if (instantiateRequested)
    {
        if (setter)
        {
            if (setter->IsInstantiated() || !setter->IsInstantiationRequested())
            {
                instantiateSetter = false;
            }
        }
    }
    if (setter && instantiateSetter)
    {
        prevContainerScope = containerScope;
        containerScope = setter->GetContainerScope();
        Symbol* valueSymbol = containerScope->Lookup(StringPtr(U"value"));
        ParameterSymbol* value = dynamic_cast<ParameterSymbol*>(valueSymbol);
        Assert(value, "parameter symbol expected");
        value->SetType(propertyType);
        propertyNode.Setter()->Accept(*this);
        containerScope = prevContainerScope;
        setter->SetTypesResolved();
    }
    containerScope = prevContainerScope;
}

void TypeBinderVisitor::Visit(IndexerNode& indexerNode)
{
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(indexerNode);
    IndexerSymbol* indexerSymbol = dynamic_cast<IndexerSymbol*>(symbol);
    Assert(indexerSymbol, "indexer symbol expected");
    ContainerScope* prevContainerScope = containerScope;
    containerScope = indexerSymbol->GetContainerScope();
    indexerSymbol->SetSpecifiers(indexerNode.GetSpecifiers());
    Symbol* parent = indexerSymbol->Parent();
    if (parent->IsStatic() && !indexerSymbol->IsStatic())
    {
        throw Exception("static class cannot contain instance indexers", indexerSymbol->GetSpan(), parent->GetSpan());
    }
    TypeSymbol* indexerValueType = ResolveType(boundCompileUnit, containerScope, indexerNode.ValueTypeExpr());
    indexerSymbol->SetValueType(indexerValueType);
    TypeSymbol* indexerIndexType = ResolveType(boundCompileUnit, containerScope, indexerNode.IndexTypeExpr());
    indexerSymbol->SetIndexType(indexerIndexType);
    IndexerGetterFunctionSymbol* getter = indexerSymbol->Getter();
    IndexerSetterFunctionSymbol* setter = indexerSymbol->Setter();
    if (!getter && !setter)
    {
        throw Exception("indexer '" + ToUtf8(indexerSymbol->FullName()) + "' must have either getter or setter or both", indexerSymbol->GetSpan());
    }
    bool instantiateGetter = true;
    if (instantiateRequested)
    {
        if (getter)
        {
            if (getter->IsInstantiated() || !getter->IsInstantiationRequested())
            {
                instantiateGetter = false;
            }
        }
    }
    if (getter && instantiateGetter)
    {
        getter->SetReturnType(indexerValueType);
        prevContainerScope = containerScope;
        containerScope = getter->GetContainerScope();
        if (!indexerSymbol->IsStatic())
        {
            Assert(getter->Parameters().size() > 1, "more than one parameter expected");
            ParameterSymbol* indexParam = getter->Parameters()[1];
            indexParam->SetType(indexerIndexType);
        }
        else
        {
            Assert(getter->Parameters().size() > 0, "more than zero parameters expected");
            ParameterSymbol* indexParam = getter->Parameters()[0];
            indexParam->SetType(indexerIndexType);
        }
        indexerNode.Getter()->Accept(*this);
        containerScope = prevContainerScope;
        getter->SetTypesResolved();
    }
    bool instantiateSetter = true;
    if (instantiateRequested)
    {
        if (setter)
        {
            if (setter->IsInstantiated() || !setter->IsInstantiationRequested())
            {
                instantiateSetter = false;
            }
        }
    }
    if (setter && instantiateSetter)
    {
        prevContainerScope = containerScope;
        containerScope = setter->GetContainerScope();
        Symbol* valueSymbol = containerScope->Lookup(StringPtr(U"value"));
        ParameterSymbol* value = dynamic_cast<ParameterSymbol*>(valueSymbol);
        Assert(value, "parameter symbol expected");
        value->SetType(indexerValueType);
        if (!indexerSymbol->IsStatic())
        {
            Assert(setter->Parameters().size() > 1, "more than one parameter expected");
            ParameterSymbol* indexParam = setter->Parameters()[1];
            indexParam->SetType(indexerIndexType);
        }
        else
        {
            Assert(setter->Parameters().size() > 0, "more than zero parameters expected");
            ParameterSymbol* indexParam = setter->Parameters()[0];
            indexParam->SetType(indexerIndexType);
        }
        indexerNode.Setter()->Accept(*this);
        containerScope = prevContainerScope;
        setter->SetTypesResolved();
    }
    containerScope = prevContainerScope;
}

void TypeBinderVisitor::Visit(CompoundStatementNode& compoundStatementNode)
{
    ContainerScope* prevContainerScope = containerScope;
    containerScope = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(compoundStatementNode)->GetContainerScope();
    int n = compoundStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statementNode = compoundStatementNode.Statements()[i];
        statementNode->Accept(*this);
    }
    containerScope = prevContainerScope;
}

void TypeBinderVisitor::Visit(IfStatementNode& ifStatementNode)
{
    ifStatementNode.ThenS()->Accept(*this);
    if (ifStatementNode.ElseS())
    {
        ifStatementNode.ElseS()->Accept(*this);
    }
}

void TypeBinderVisitor::Visit(WhileStatementNode& whileStatementNode)
{
    whileStatementNode.Statement()->Accept(*this);
}

void TypeBinderVisitor::Visit(DoStatementNode& doStatementNode)
{
    doStatementNode.Statement()->Accept(*this);
}

void TypeBinderVisitor::Visit(ForStatementNode& forStatementNode)
{
    ContainerScope* prevContainerScope = containerScope;
    containerScope = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(forStatementNode)->GetContainerScope();
    forStatementNode.InitS()->Accept(*this);
    forStatementNode.ActionS()->Accept(*this);
    containerScope = prevContainerScope;
}

void TypeBinderVisitor::Visit(SwitchStatementNode& switchStatementNode)
{
    int n = switchStatementNode.Cases().Count();
    for (int i = 0; i < n; ++i)
    {
        CaseStatementNode* caseStatement = switchStatementNode.Cases()[i];
        caseStatement->Accept(*this);
    }
    if (switchStatementNode.Default())
    {
        switchStatementNode.Default()->Accept(*this);
    }
}

void TypeBinderVisitor::Visit(CaseStatementNode& caseStatementNode)
{
    int n = caseStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statementNode = caseStatementNode.Statements()[i];
        statementNode->Accept(*this);
    }
}

void TypeBinderVisitor::Visit(DefaultStatementNode& defaultStatementNode)
{
    int n = defaultStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statementNode = defaultStatementNode.Statements()[i];
        statementNode->Accept(*this);
    }
}

void TypeBinderVisitor::Visit(ConstructionStatementNode& constructionStatementNode)
{
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(constructionStatementNode);
    LocalVariableSymbol* localVariableSymbol = dynamic_cast<LocalVariableSymbol*>(symbol);
    Assert(localVariableSymbol, "local variable symbol expected");
    TypeSymbol* type = ResolveType(boundCompileUnit, containerScope, constructionStatementNode.TypeExpr());
    localVariableSymbol->SetType(type);
}

void TypeBinderVisitor::Visit(TryStatementNode& tryStatementNode)
{
    tryStatementNode.TryBlock()->Accept(*this);
    int n = tryStatementNode.Catches().Count();
    for (int i = 0; i < n; ++i)
    {
        CatchNode* catchNode = tryStatementNode.Catches()[i];
        catchNode->Accept(*this);
    }
    if (tryStatementNode.FinallyBlock())
    {
        tryStatementNode.FinallyBlock()->Accept(*this);
    }
}

void TypeBinderVisitor::Visit(CatchNode& catchNode)
{
    ContainerScope* prevContainerScope = containerScope;
    containerScope = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(catchNode)->GetContainerScope();
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(*catchNode.Id());
    LocalVariableSymbol* exceptionVar = dynamic_cast<LocalVariableSymbol*>(symbol);
    Assert(exceptionVar, "local variable symbol expected");
    TypeSymbol* exceptionVarType = ResolveType(boundCompileUnit, containerScope, catchNode.TypeExpr());
    if (ClassTypeSymbol* exceptionVarClassType = dynamic_cast<ClassTypeSymbol*>(exceptionVarType))
    {
        TypeSymbol* systemExceptionType = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Exception");
        ClassTypeSymbol* systemExceptionClassType = dynamic_cast<ClassTypeSymbol*>(systemExceptionType);
        Assert(systemExceptionClassType, "class type symbol expected");
        if (exceptionVarClassType->IsProject())
        {
            Node* exceptionVarNode = boundCompileUnit.GetAssembly().GetSymbolTable().GetNode(exceptionVarClassType);
            ClassNode* exceptionVarClassNode = dynamic_cast<ClassNode*>(exceptionVarNode);
            Assert(exceptionVarClassNode, "class node expected");
            BindClass(exceptionVarClassType, *exceptionVarClassNode);
        }
        if (exceptionVarClassType == systemExceptionClassType || exceptionVarClassType->HasBaseClass(systemExceptionClassType))
        {
            exceptionVar->SetType(exceptionVarClassType);
        }
        else
        {
            throw Exception("exception variable must be of class type equal to System.Exception class or derive from it", catchNode.TypeExpr()->GetSpan());
        }
    }
    else
    {
        throw Exception("exception variable must of be class type equal to System.Exception class or derive from it", catchNode.TypeExpr()->GetSpan());
    }
    catchNode.CatchBlock()->Accept(*this);
    containerScope = prevContainerScope;
}

void TypeBinderVisitor::Visit(DelegateNode& delegateNode)
{
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(delegateNode);
    DelegateTypeSymbol* delegateTypeSymbol = dynamic_cast<DelegateTypeSymbol*>(symbol);
    Assert(delegateTypeSymbol, "delegate type symbol expected");
    delegateTypeSymbol->SetSpecifiers(delegateNode.GetSpecifiers());
    TypeSymbol* returnType = ResolveType(boundCompileUnit, containerScope, delegateNode.ReturnTypeExpr());
    delegateTypeSymbol->SetReturnType(returnType);
    int n = delegateNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = delegateNode.Parameters()[i];
        TypeSymbol* parameterType = ResolveType(boundCompileUnit, containerScope, parameterNode->TypeExpr(), TypeResolverFlags::parameterType);
        Symbol* ps = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(*parameterNode);
        ParameterSymbol* parameterSymbol = dynamic_cast<ParameterSymbol*>(ps);
        Assert(parameterSymbol, "parameter symbol expected");
        parameterSymbol->SetType(parameterType);
    }
    CreateDelegateFun(boundCompileUnit.GetAssembly(), delegateTypeSymbol);
}

void TypeBinderVisitor::Visit(ClassDelegateNode& classDelegateNode)
{
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(classDelegateNode);
    ClassDelegateTypeSymbol* classDelegateTypeSymbol = dynamic_cast<ClassDelegateTypeSymbol*>(symbol);
    Assert(classDelegateTypeSymbol, "class delegate type symbol expected");
    classDelegateTypeSymbol->SetSpecifiers(classDelegateNode.GetSpecifiers());
    TypeSymbol* returnType = ResolveType(boundCompileUnit, containerScope, classDelegateNode.ReturnTypeExpr());
    classDelegateTypeSymbol->SetReturnType(returnType);
    int n = classDelegateNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = classDelegateNode.Parameters()[i];
        TypeSymbol* parameterType = ResolveType(boundCompileUnit, containerScope, parameterNode->TypeExpr(), TypeResolverFlags::parameterType);
        Symbol* ps = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(*parameterNode);
        ParameterSymbol* parameterSymbol = dynamic_cast<ParameterSymbol*>(ps);
        Assert(parameterSymbol, "parameter symbol expected");
        parameterSymbol->SetType(parameterType);
    }
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    Constant objName = constantPool.GetConstant(constantPool.Install(StringPtr(U"@obj")));
    MemberVariableSymbol* objMemVar = new MemberVariableSymbol(classDelegateNode.GetSpan(), objName);
    objMemVar->SetAssembly(&boundCompileUnit.GetAssembly());
    objMemVar->SetPublic();
    objMemVar->SetType(boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Object"));
    classDelegateTypeSymbol->AddSymbol(std::unique_ptr<Symbol>(objMemVar));
    Constant dlgName = constantPool.GetConstant(constantPool.Install(StringPtr(U"@dlg")));
    MemberVariableSymbol* dlgMemVar = new MemberVariableSymbol(classDelegateNode.GetSpan(), dlgName);
    dlgMemVar->SetAssembly(&boundCompileUnit.GetAssembly());
    dlgMemVar->SetPublic();
    CloneContext cloneContext;
    DelegateNode delegateNode(classDelegateNode.GetSpan(), classDelegateNode.GetSpecifiers(), classDelegateNode.ReturnTypeExpr()->Clone(cloneContext),
        new IdentifierNode(classDelegateNode.GetSpan(), "@" + classDelegateNode.Id()->Str() + "_delegate_type"));
    delegateNode.AddParameter(new ParameterNode(classDelegateNode.GetSpan(), new IdentifierNode(classDelegateNode.GetSpan(), "System.Object"), new IdentifierNode(classDelegateNode.GetSpan(), "@obj")));
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = static_cast<ParameterNode*>(classDelegateNode.Parameters()[i]->Clone(cloneContext));
        delegateNode.AddParameter(parameterNode);
    }
    boundCompileUnit.GetAssembly().GetSymbolTable().BeginContainer(containerScope->Container());
    SymbolCreatorVisitor symbolCreatorVisitor(boundCompileUnit.GetAssembly());
    delegateNode.Accept(symbolCreatorVisitor);
    boundCompileUnit.GetAssembly().GetSymbolTable().EndContainer();
    delegateNode.Accept(*this);
    Symbol* dlgSym = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(delegateNode);
    DelegateTypeSymbol* delegateTypeSymbol = dynamic_cast<DelegateTypeSymbol*>(dlgSym);
    Assert(delegateTypeSymbol, "delegate type symbol expected");
    dlgMemVar->SetType(delegateTypeSymbol);
    classDelegateTypeSymbol->AddSymbol(std::unique_ptr<Symbol>(dlgMemVar));
    ClassNode* classNode = new ClassNode(classDelegateNode.GetSpan(), classDelegateNode.GetSpecifiers(), static_cast<IdentifierNode*>(classDelegateNode.Id()->Clone(cloneContext)));
    BindClass(classDelegateTypeSymbol, *classNode);
    boundCompileUnit.GetAssembly().GetSymbolTable().MapNode(*classNode, classDelegateTypeSymbol);
    boundCompileUnit.AddClassNode(classNode);
}

} } // namespace cminor::binder
