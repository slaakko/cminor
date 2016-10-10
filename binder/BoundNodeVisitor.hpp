// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_BOUND_NODE_VISITOR_INCLUDED
#define CMINOR_BINDER_BOUND_NODE_VISITOR_INCLUDED

namespace cminor { namespace binder {

class BoundCompileUnit;
class BoundFunction;
class BoundCompoundStatement;
class BoundConstructionStatement;
class BoundAssignmentStatement;
class BoundLiteral;
class BoundConstant;
class BoundLocalVariable;
class BoundMemberVariable;
class BoundParameter;
class BoundConversion;
class BoundFunctionCall;

class BoundNodeVisitor
{
public:
    virtual ~BoundNodeVisitor();
    virtual void Visit(BoundCompileUnit& boundCompileUnit) {}
    virtual void Visit(BoundFunction& boundFunction) {}
    virtual void Visit(BoundCompoundStatement& boundCompoundStatement) {}
    virtual void Visit(BoundConstructionStatement& boundConstructionStatement) {}
    virtual void Visit(BoundAssignmentStatement& boundAssignmentStatement) {}
    virtual void Visit(BoundLiteral& boundLiteral) {}
    virtual void Visit(BoundConstant& boundConstant) {}
    virtual void Visit(BoundLocalVariable& boundLocalVariable) {}
    virtual void Visit(BoundMemberVariable& boundMemberVariable) {}
    virtual void Visit(BoundParameter& boundParameter) {}
    virtual void Visit(BoundConversion& boundConversion) {}
    virtual void Visit(BoundFunctionCall& boundFunctionCall) {}
};

} } // namespace cminor::binder

#endif // CMINOR_BINDER_BOUND_NODE_VISITOR_INCLUDED
