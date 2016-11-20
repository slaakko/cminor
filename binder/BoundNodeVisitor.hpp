// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_BOUND_NODE_VISITOR_INCLUDED
#define CMINOR_BINDER_BOUND_NODE_VISITOR_INCLUDED

namespace cminor { namespace binder {

class BoundCompileUnit;
class BoundClass;
class BoundFunction;
class BoundCompoundStatement;
class BoundReturnStatement;
class BoundIfStatement;
class BoundWhileStatement;
class BoundDoStatement;
class BoundForStatement;
class BoundBreakStatement;
class BoundContinueStatement;
class BoundConstructionStatement;
class BoundAssignmentStatement;
class BoundExpressionStatement;
class BoundEmptyStatement;
class BoundLiteral;
class BoundConstant;
class BoundEnumConstant;
class BoundLocalVariable;
class BoundMemberVariable;
class BoundProperty;
class BoundIndexer;
class BoundParameter;
class BoundArrayElement;
class BoundStringChar;
class BoundConversion;
class BoundFunctionCall;
class BoundNewExpression;
class BoundConjunction;
class BoundDisjunction;

class BoundNodeVisitor
{
public:
    virtual ~BoundNodeVisitor();
    virtual void Visit(BoundCompileUnit& boundCompileUnit) {}
    virtual void Visit(BoundClass& boundClass) {}
    virtual void Visit(BoundFunction& boundFunction) {}
    virtual void Visit(BoundCompoundStatement& boundCompoundStatement) {}
    virtual void Visit(BoundReturnStatement& boundReturnStatement) {}
    virtual void Visit(BoundIfStatement& boundIfStatement) {}
    virtual void Visit(BoundWhileStatement& boundWhileStatement) {}
    virtual void Visit(BoundDoStatement& boundDoStatement) {}
    virtual void Visit(BoundForStatement& boundForStatement) {}
    virtual void Visit(BoundBreakStatement& boundBreakStatement) {}
    virtual void Visit(BoundContinueStatement& boundContinueStatement) {}
    virtual void Visit(BoundConstructionStatement& boundConstructionStatement) {}
    virtual void Visit(BoundAssignmentStatement& boundAssignmentStatement) {}
    virtual void Visit(BoundExpressionStatement& boundExpressionStatement) {}
    virtual void Visit(BoundEmptyStatement& boundEmptyStatement) {}
    virtual void Visit(BoundLiteral& boundLiteral) {}
    virtual void Visit(BoundConstant& boundConstant) {}
    virtual void Visit(BoundEnumConstant& boundEnumConstant) {}
    virtual void Visit(BoundLocalVariable& boundLocalVariable) {}
    virtual void Visit(BoundMemberVariable& boundMemberVariable) {}
    virtual void Visit(BoundProperty& boundProperty) {}
    virtual void Visit(BoundIndexer& boundIndexer) {}
    virtual void Visit(BoundParameter& boundParameter) {}
    virtual void Visit(BoundArrayElement& boundArrayElement) {}
    virtual void Visit(BoundStringChar& boundStringChar) {}
    virtual void Visit(BoundConversion& boundConversion) {}
    virtual void Visit(BoundFunctionCall& boundFunctionCall) {}
    virtual void Visit(BoundNewExpression& boundNewExpression) {}
    virtual void Visit(BoundConjunction& boundConjunction) {}
    virtual void Visit(BoundDisjunction& boundDisjunction) {}
};

} } // namespace cminor::binder

#endif // CMINOR_BINDER_BOUND_NODE_VISITOR_INCLUDED
