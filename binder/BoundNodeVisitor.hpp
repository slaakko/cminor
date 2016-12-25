// =================================
// Copyright (c) 2017 Seppo Laakko
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
class BoundSwitchStatement;
class BoundCaseStatement;
class BoundDefaultStatement;
class BoundGotoCaseStatement;
class BoundGotoDefaultStatement;
class BoundBreakStatement;
class BoundContinueStatement;
class BoundGotoStatement;
class BoundConstructionStatement;
class BoundAssignmentStatement;
class BoundExpressionStatement;
class BoundEmptyStatement;
class BoundThrowStatement;
class BoundTryStatement;
class BoundCatchStatement;
class BoundStaticInitStatement;
class BoundDoneStaticInitStatement;
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
class BoundDelegateCall;
class BoundClassDelegateCall;
class BoundNewExpression;
class BoundConjunction;
class BoundDisjunction;
class BoundIsExpression;
class BoundAsExpression;
class BoundClassDelegateClassObjectPair;
class BoundLocalRefExpression;
class BoundMemberVarRefExpression;

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
    virtual void Visit(BoundSwitchStatement& boundSwitchStatement) {}
    virtual void Visit(BoundCaseStatement& boundCaseStatement) {}
    virtual void Visit(BoundGotoCaseStatement& boundGotoCaseStatement) {}
    virtual void Visit(BoundGotoDefaultStatement& boundGotoDefaultStatement) {}
    virtual void Visit(BoundDefaultStatement& boundDefaultStatement) {}
    virtual void Visit(BoundBreakStatement& boundBreakStatement) {}
    virtual void Visit(BoundContinueStatement& boundContinueStatement) {}
    virtual void Visit(BoundGotoStatement& boundGotoStatement) {}
    virtual void Visit(BoundConstructionStatement& boundConstructionStatement) {}
    virtual void Visit(BoundAssignmentStatement& boundAssignmentStatement) {}
    virtual void Visit(BoundExpressionStatement& boundExpressionStatement) {}
    virtual void Visit(BoundEmptyStatement& boundEmptyStatement) {}
    virtual void Visit(BoundThrowStatement& boundThrowStatement) {}
    virtual void Visit(BoundTryStatement& boundTryStatement) {}
    virtual void Visit(BoundCatchStatement& boundCatchStatement) {}
    virtual void Visit(BoundStaticInitStatement& boundStaticInitStatement) {}
    virtual void Visit(BoundDoneStaticInitStatement& boundDoneStaticInitStatement) {}
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
    virtual void Visit(BoundDelegateCall& boundDelegateCall) {}
    virtual void Visit(BoundClassDelegateCall& boundClassDelegateCall) {}
    virtual void Visit(BoundNewExpression& boundNewExpression) {}
    virtual void Visit(BoundConjunction& boundConjunction) {}
    virtual void Visit(BoundDisjunction& boundDisjunction) {}
    virtual void Visit(BoundIsExpression& boundIsExpression) {}
    virtual void Visit(BoundAsExpression& boundAsExpression) {}
    virtual void Visit(BoundClassDelegateClassObjectPair& boundClassDelegateClassObjectPair) {}
    virtual void Visit(BoundLocalRefExpression& boundLocalRefExpression) {}
    virtual void Visit(BoundMemberVarRefExpression& boundMemberVarRefExpression) {}
};

} } // namespace cminor::binder

#endif // CMINOR_BINDER_BOUND_NODE_VISITOR_INCLUDED
