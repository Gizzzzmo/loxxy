module;
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <loxxy/ast.hpp>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
export module ast.offset_builder;

import ast;
import utils.stupid_type_traits;
import utils.multi_vector;

using utils::multi_vector;

export namespace loxxy {

template <
    typename offset_t, typename _Payload = empty, bool _ptr_variant = true,
    PayloadBuilder<_Payload, OffsetPointerIndirection<offset_t>, _ptr_variant> Builder =
        DefaultPayloadBuilder<_Payload>>
struct OffsetBuilder {
    using Payload = _Payload;
    using Indirection = OffsetPointerIndirection<offset_t>;
    static constexpr bool ptr_variant = true;
    USING_FAMILY(Payload, OffsetPointerIndirection<offset_t>, true);

    using Resolver = multi_vector<
        BinaryExpr, UnaryExpr, GroupingExpr, StringExpr, NumberExpr, BoolExpr, NilExpr, VarExpr, AssignExpr, CallExpr,
        PrintStmt, ExpressionStmt, VarDecl, FunDecl, BlockStmt, IfStmt, WhileStmt, ReturnStmt>;

    template <typename... Args>
    OffsetBuilder(Args&&... args) : payload_builder(std::forward<Args>(args)...) {}

    template <typename NodeType, typename... Args>
    auto operator()(marker<NodeType>, Args&&... args) {
        auto& vector = nodes.template get_vec<NodeType>();
        vector.emplace_back(payload_builder(std::forward<Args>(args)...), std::forward<Args>(args)...);

        return offset_pointer<NodeType, offset_t>(vector.size() - 1);
    }

    auto get_resolver(this auto&& self) -> auto&& { return self.nodes; }

private:
    Resolver nodes;

    Builder payload_builder;
};

template <
    typename offset_t, typename _Payload, PayloadBuilder<_Payload, OffsetPointerIndirection<offset_t>, false> Builder>
struct OffsetBuilder<offset_t, _Payload, false, Builder> {
    using Payload = _Payload;
    using Indirection = OffsetPointerIndirection<offset_t>;
    static constexpr bool ptr_variant = false;
    USING_FAMILY(Payload, OffsetPointerIndirection<offset_t>, false);

    using Resolver = multi_vector<Expression, Statement>;

    template <typename... Args>
    OffsetBuilder(Args&&... args) : payload_builder(std::forward<Args>(args)...) {}

    ~OffsetBuilder() {
        // std::cout << nodes.template get_vec<Expression>().size() << "\n"
        //           << nodes.template get_vec<Statement>().size() << "\n";
    }

    template <ExpressionSTN NodeType, typename... Args>
    auto operator()(marker<NodeType>, Args&&... args) {
        auto& vector = nodes.template get_vec<Expression>();
        Expression expr{NodeType(payload_builder(std::forward<Args>(args)...), std::forward<Args>(args)...)};
        vector.emplace_back(expr);

        return offset_pointer<Expression, offset_t>(vector.size() - 1);
    }

    template <StatementSTN NodeType, typename... Args>
    auto operator()(marker<NodeType>, Args&&... args) {
        auto& vector = nodes.template get_vec<Statement>();
        Statement stmt{NodeType(payload_builder(std::forward<Args>(args)...), std::forward<Args>(args)...)};
        vector.emplace_back(stmt);

        return offset_pointer<Statement, offset_t>(vector.size() - 1);
    }

    auto get_resolver(this auto&& self) -> auto&& { return self.nodes; }

private:
    Resolver nodes;

    Builder payload_builder;
};

} // namespace loxxy
