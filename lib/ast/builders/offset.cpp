module;
#include <vector>
#include <type_traits>
#include <utility>
#include <cstdint>
export module ast.offset_builder;

import ast;
import utils.stupid_type_traits;

export namespace loxxy {

template<
    typename offset_t,
    typename Payload = empty,
    PayloadBuilder<Payload, OffsetPointerIndirection<offset_t>, true> Builder = DefaultPayloadBuilder<Payload>
>
struct OffsetBuilder {
    using Self = OffsetBuilder<offset_t, Payload, Builder>;
    using Indirection = OffsetPointerIndirection<uint32_t>;
    using STNPointer = STNPointer<Payload, Indirection, true>;
    using BinaryNode = BinaryNode<Payload, Indirection, true>;
    using UnaryNode = UnaryNode<Payload, Indirection, true>;
    using GroupingNode = GroupingNode<Payload, Indirection, true>;
    using StringNode = StringNode<Payload, Indirection, true>;
    using NumberNode = NumberNode<Payload, Indirection, true>;
    using BoolNode = BoolNode<Payload, Indirection, true>;
    using NilNode = NilNode<Payload, Indirection, true>;

    template<typename... Args>
    OffsetBuilder(Args&&... args) : payload_builder(std::forward<Args>(args)...) {}

    template<ConcreteSTN NodeType>
    auto& operator[](const offset_pointer<NodeType, offset_t>& ptr) {
        return vector_of<NodeType>()[ptr.offset];
    }

    template<ConcreteSTN NodeType>
    const auto& operator[](const offset_pointer<NodeType, offset_t>& ptr) const {
        return vector_of<NodeType>()[ptr.offset];
    }

    template<typename... Args>
    STNPointer operator()(Args&&... args) {
        using NodeType = ResolveNodeType<Payload, Indirection, true, Args...>;
        auto& vector = vector_of<NodeType>();
        vector.emplace_back(
            payload_builder(std::forward<Args>(args)...),
            std::forward<Args>(args)...
        );

        return offset_pointer<NodeType, offset_t>(vector.size()-1);
    }

    template<ConcreteSTN NodeType>
    std::vector<NodeType>& vector_of() {
        std::vector<NodeType> vec{};
        return vec;
    }

    template<ConcreteSTN NodeType>
    const std::vector<NodeType>& vector_of() const{
        return const_cast<std::remove_const_t<decltype(*this)&>>(*this).template vector_of<NodeType>();
    }

    template<>
    std::vector<BinaryNode>&  vector_of<BinaryNode>() { return binary_nodes; }
    template<>
    std::vector<UnaryNode>&  vector_of<UnaryNode>() { return unary_nodes; }
    template<>
    std::vector<GroupingNode>&  vector_of<GroupingNode>() { return grouping_nodes; }
    template<>
    std::vector<StringNode>&  vector_of<StringNode>() { return string_nodes; }
    template<>
    std::vector<NumberNode>&  vector_of<NumberNode>() { return number_nodes; }
    template<>
    std::vector<BoolNode>&  vector_of<BoolNode>() { return bool_nodes; }
    template<>
    std::vector<NilNode>&  vector_of<NilNode>() { return nil_nodes; }

    private:
        std::vector<BinaryNode> binary_nodes;
        std::vector<UnaryNode> unary_nodes;
        std::vector<GroupingNode> grouping_nodes;
        std::vector<StringNode> string_nodes;
        std::vector<NumberNode> number_nodes;
        std::vector<BoolNode> bool_nodes;
        std::vector<NilNode> nil_nodes;
        Builder payload_builder;
        // Interpreter<Payload, Indirection, true, Self> interpreter;

};

} // namespace loxxy