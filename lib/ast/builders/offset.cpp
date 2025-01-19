module;
#include <cstddef>
#include <cstdint>
#include <loxxy/ast.hpp>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
export module ast.offset_builder;

import ast;
import utils.stupid_type_traits;

export namespace loxxy {

template <typename... Ts>
struct multi_vector {
    template <typename Self, typename T, typename offset_t>
    decltype(auto) operator[](this Self&& self, offset_pointer<T, offset_t>& ptr) {
        return std::get<std::vector<T>>(self.vectors)[ptr.offset];
    }
    template <typename T, typename Self>
    decltype(auto) get_vec(this Self&& self) {
        return std::get<std::vector<T>>(self.vectors);
    }

private:
    std::tuple<std::vector<Ts>...> vectors;
};

template <
    typename offset_t, typename _Payload = empty,
    PayloadBuilder<_Payload, OffsetPointerIndirection<offset_t>, true> Builder = DefaultPayloadBuilder<_Payload>>
struct OffsetBuilder {
    using Payload = _Payload;
    using Indirection = OffsetPointerIndirection<offset_t>;
    static constexpr bool ptr_variant = true;

    USING_FAMILY(Payload, OffsetPointerIndirection<offset_t>, true);

    template <typename... Args>
    OffsetBuilder(Args&&... args) : payload_builder(std::forward<Args>(args)...) {}

    template <typename NodeType, typename... Args>
    auto operator()(marker<NodeType>, Args&&... args) {
        auto& vector = nodes.template get_vec<NodeType>();
        vector.emplace_back(payload_builder(std::forward<Args>(args)...), std::forward<Args>(args)...);

        return offset_pointer<NodeType, offset_t>(vector.size() - 1);
    }

    auto&& get_resolver(this auto&& self) { return self.nodes; }

private:
    multi_vector<
        BinaryExpr, UnaryExpr, GroupingExpr, StringExpr, NumberExpr, BoolExpr, NilExpr, VarExpr, AssignExpr, CallExpr,
        PrintStmt, ExpressionStmt, VarDecl, BlockStmt, IfStmt, WhileStmt>
        nodes;

    Builder payload_builder;
};

} // namespace loxxy
