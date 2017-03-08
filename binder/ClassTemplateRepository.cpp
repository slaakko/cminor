// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/ClassTemplateRepository.hpp>
#include <cminor/binder/BoundCompileUnit.hpp>
#include <cminor/binder/TypeBinderVisitor.hpp>
#include <cminor/binder/StatementBinderVisitor.hpp>
#include <cminor/symbols/ConstantPoolInstallerVisitor.hpp>
#include <cminor/symbols/SymbolCreatorVisitor.hpp>
#include <cminor/util/Util.hpp>

namespace cminor { namespace binder {

std::unique_ptr<NamespaceNode> CreateNamespaces(const Span& span, const std::string& nsFullName, const NodeList<Node>& usingNodes, NamespaceNode*& currentNs)
{
    std::unique_ptr<NamespaceNode> globalNs(new NamespaceNode(span, new IdentifierNode(span, "")));
    currentNs = globalNs.get();
    if (!nsFullName.empty())
    {
        std::vector<std::string> components = Split(nsFullName, U'.');
        for (const std::string& component : components)
        {
            NamespaceNode* nsNode = new NamespaceNode(span, new IdentifierNode(span, component));
            currentNs->AddMember(nsNode);
            currentNs = nsNode;
        }
    }
    CloneContext cloneContext;
    int n = usingNodes.Count();
    for (int i = 0; i < n; ++i)
    {
        Node* usingNode = usingNodes[i];
        currentNs->AddMember(usingNode->Clone(cloneContext));
    }
    return globalNs;
}

ClassTemplateRepository::ClassTemplateRepository(BoundCompileUnit& boundCompileUnit_) : boundCompileUnit(boundCompileUnit_)
{
}

void ClassTemplateRepository::BindClassTemplateSpecialization(ClassTemplateSpecializationSymbol* classTemplateSpecialization, ContainerScope* containerScope)
{
    if (classTemplateSpecialization->IsBound()) return;
    classTemplateSpecializations.insert(classTemplateSpecialization);
    bool fileScopeAdded = false;
    ClassTypeSymbol* primaryClassTemplate = classTemplateSpecialization->PrimaryClassTemplate();
    if (!primaryClassTemplate->Ns()->IsGlobalNamespace())
    {
        std::unique_ptr<NamespaceImportNode> nsImport(new NamespaceImportNode(primaryClassTemplate->GetSpan(),
            new IdentifierNode(primaryClassTemplate->GetSpan(), ToUtf8(primaryClassTemplate->Ns()->FullName()))));
        std::unique_ptr<FileScope> fileScope(new FileScope());
        fileScope->InstallNamespaceImport(containerScope, nsImport.get());
        boundCompileUnit.AddNode(std::move(nsImport));
        boundCompileUnit.AddFileScope(std::move(fileScope));
        fileScopeAdded = true;
    }
    Node* node = boundCompileUnit.GetAssembly().GetSymbolTable().GetNodeNothrow(primaryClassTemplate);
    if (!node)
    {
        primaryClassTemplate->ReadClassNode(boundCompileUnit.GetAssembly());
        node = boundCompileUnit.GetAssembly().GetSymbolTable().GetNode(primaryClassTemplate);
    }
    ClassNode* classNode = dynamic_cast<ClassNode*>(node);
    Assert(classNode, "class node expected");
    NamespaceNode* currentNs = nullptr;
    std::unique_ptr<NamespaceNode> globalNs = CreateNamespaces(primaryClassTemplate->GetSpan(), ToUtf8(primaryClassTemplate->Ns()->FullName()), primaryClassTemplate->UsingNodes(), currentNs);
    CloneContext cloneContext;
    cloneContext.SetInstantiateClassNode();
    ClassNode* classInstanceNode = static_cast<ClassNode*>(classNode->Clone(cloneContext));
    IdentifierNode* classInstanceId = new IdentifierNode(classInstanceNode->GetSpan(), ToUtf8(classTemplateSpecialization->FullName()));
    classInstanceNode->SetId(classInstanceId);
    Assert(currentNs, "current namespace not set");
    currentNs->AddMember(classInstanceNode);
    int n = int(primaryClassTemplate->TypeParameters().size());
    int m = classTemplateSpecialization->NumTypeArguments();
    if (n < m)
    {
        throw Exception("too many template arguments", classTemplateSpecialization->GetSpan());
    }
    else if (n > m)
    {
        throw Exception("too few template arguments", classTemplateSpecialization->GetSpan());
    }
    for (int i = 0; i < n; ++i)
    {
        TypeParameterSymbol* typeParameterSymbol = primaryClassTemplate->TypeParameters()[i];
        BoundTypeParameterSymbol* boundTypeParameterSymbol = new BoundTypeParameterSymbol(typeParameterSymbol->GetSpan(), typeParameterSymbol->NameConstant());
        boundTypeParameterSymbol->SetAssembly(classTemplateSpecialization->GetAssembly());
        boundTypeParameterSymbol->GetAssembly()->GetConstantPool().Install(boundTypeParameterSymbol->NameConstant());
        boundTypeParameterSymbol->SetType(classTemplateSpecialization->TypeArgument(i));
        classTemplateSpecialization->AddSymbol(std::unique_ptr<Symbol>(boundTypeParameterSymbol));
    }
    SymbolCreatorVisitor symbolCreatorVisitor(boundCompileUnit.GetAssembly());
    symbolCreatorVisitor.SetClassInstanceNode(classInstanceNode);
    symbolCreatorVisitor.SetClassTemplateSpecialization(classTemplateSpecialization);
    globalNs->Accept(symbolCreatorVisitor);
    TypeBinderVisitor typeBinderVisitor(boundCompileUnit);
    globalNs->Accept(typeBinderVisitor);
    StatementBinderVisitor statementBinderVisitor(boundCompileUnit);
    statementBinderVisitor.SetDoNotInstantiate();
    globalNs->Accept(statementBinderVisitor);
    ConstantPoolInstallerVisitor constantPoolInstallerVisitor(boundCompileUnit.GetAssembly().GetConstantPool());
    globalNs->Accept(constantPoolInstallerVisitor);
    classTemplateSpecialization->SetGlobalNs(std::move(globalNs));
    if (fileScopeAdded)
    {
        boundCompileUnit.RemoveLastFileScope();
    }
}

void ClassTemplateRepository::Add(ClassTemplateSpecializationSymbol* classTemplateSpecialization)
{
    classTemplateSpecializations.insert(classTemplateSpecialization);
}

void ClassTemplateRepository::Clear()
{
    classTemplateSpecializations.clear();
}

} } // namespace cminor::binder
