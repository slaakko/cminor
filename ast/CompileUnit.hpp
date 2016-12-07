// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_COMPILE_UNIT_INCLUDED
#define CMINOR_AST_COMPILE_UNIT_INCLUDED
#include <cminor/ast/Namespace.hpp>

namespace cminor { namespace ast {

class CompileUnitNode : public Node
{
public:
    CompileUnitNode(const Span& span_);
    CompileUnitNode(const Span& span_, const std::string& filePath_);
    NodeType GetNodeType() const override { return NodeType::compileUnitNode; }
    NamespaceNode* GlobalNs() const { return globalNs.get(); }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    const std::string& FilePath() const { return filePath; }
private:
    std::string filePath;
    std::unique_ptr<NamespaceNode> globalNs;
};

} } // namespace cminor::ast

#endif // CMINOR_AST_COMPILE_UNIT_INCLUDED
