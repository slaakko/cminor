// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_NODE_LIST_INCLUDED
#define CMINOR_AST_NODE_LIST_INCLUDED
#include <cminor/ast/AstWriter.hpp>
#include <cminor/ast/AstReader.hpp>

namespace cminor { namespace ast {

template<typename T>
class NodeList
{
public:
    int Count() const
    {
        return static_cast<int>(nodes.size());
    }
    T* operator[](int index) const
    {
        return nodes[index].get();
    }
    void Add(T* node)
    {
        nodes.push_back(std::unique_ptr<T>(node));
    }
    void SetParent(Node* parent)
    {
        for (const std::unique_ptr<T>& node : nodes)
        {
            node->SetParent(parent);
        }
    }
    void Write(AstWriter& writer)
    {
        int32_t n = static_cast<int32_t>(nodes.size());
        writer.AsMachineWriter().Put(n);
        for (int i = 0; i < n; ++i)
        {
            writer.Put(nodes[i].get());
        }
    }
    void Read(AstReader& reader)
    {
        int32_t n = reader.GetInt();
        for (int i = 0; i < n; ++i)
        {
            Node* node = reader.GetNode();
            T* asTNode = dynamic_cast<T*>(node);
            Assert(asTNode, "wrong node type");
            nodes.push_back(std::unique_ptr<T>(asTNode));
        }
    }
private:
    std::vector<std::unique_ptr<T>> nodes;
};

} } // namespace cminor::ast

#endif // CMINOR_AST_NODE_LIST_INCLUDED
