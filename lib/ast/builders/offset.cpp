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
    using ExprPointer = ExprPointer<Payload, Indirection, true>;
    using BinaryExpr = BinaryExpr<Payload, Indirection, true>;
    using UnaryExpr = UnaryExpr<Payload, Indirection, true>;
    using GroupingExpr = GroupingExpr<Payload, Indirection, true>;
    using StringExpr = StringExpr<Payload, Indirection, true>;
    using NumberExpr = NumberExpr<Payload, Indirection, true>;
    using BoolExpr = BoolExpr<Payload, Indirection, true>;
    using NilExpr = NilExpr<Payload, Indirection, true>;

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
    ExprPointer operator()(Args&&... args) {
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
    std::vector<BinaryExpr>&  vector_of<BinaryExpr>() { return binary_nodes; }
    template<>
    std::vector<UnaryExpr>&  vector_of<UnaryExpr>() { return unary_nodes; }
    template<>
    std::vector<GroupingExpr>&  vector_of<GroupingExpr>() { return grouping_nodes; }
    template<>
    std::vector<StringExpr>&  vector_of<StringExpr>() { return string_nodes; }
    template<>
    std::vector<NumberExpr>&  vector_of<NumberExpr>() { return number_nodes; }
    template<>
    std::vector<BoolExpr>&  vector_of<BoolExpr>() { return bool_nodes; }
    template<>
    std::vector<NilExpr>&  vector_of<NilExpr>() { return nil_nodes; }

    private:
        std::vector<BinaryExpr> binary_nodes;
        std::vector<UnaryExpr> unary_nodes;
        std::vector<GroupingExpr> grouping_nodes;
        std::vector<StringExpr> string_nodes;
        std::vector<NumberExpr> number_nodes;
        std::vector<BoolExpr> bool_nodes;
        std::vector<NilExpr> nil_nodes;
        Builder payload_builder;
        // Interpreter<Payload, Indirection, true, Self> interpreter;

};

} // namespace loxxy