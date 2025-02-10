module;
#include "loxxy/ast.hpp"
#include <iostream>
#include <memory>
#include <utility>
export module ast.boxed_node_builder;

import ast;
import ast.interpreter;
import utils.stupid_type_traits;
import utils.variant;

using std::make_unique;

export namespace loxxy {

template <
    typename _Payload = empty, bool _ptr_variant = true,
    PayloadBuilder<_Payload, UniquePtrIndirection, _ptr_variant> Builder = DefaultPayloadBuilder<_Payload>>
struct BoxedNodeBuilder {
    using Payload = _Payload;
    using Indirection = UniquePtrIndirection;
    static constexpr bool ptr_variant = _ptr_variant;
    using Resolver = void;

    USING_FAMILY(Payload, Indirection, ptr_variant);

    Builder payload_builder;

    template <typename... Args>
    BoxedNodeBuilder(Args&&... args) : payload_builder(std::forward<Args>(args)...) {}

    template <StatementSTN Out, typename... Args>
        requires(!ptr_variant)
    auto operator()(marker<Out>, Args&&... args) {
        auto node = make_unique<Statement>(
            Out{payload_builder(mark<Out>, std::forward<Args>(args)...), std::forward<Args>(args)...}
        );

        return StmtPointer(std::move(node));
    }

    template <ExpressionSTN Out, typename... Args>
        requires(!ptr_variant)
    auto operator()(marker<Out>, Args&&... args) {
        auto node = make_unique<Expression>(
            Out{payload_builder(mark<Out>, std::forward<Args>(args)...), std::forward<Args>(args)...}
        );

        return ExprPointer(std::move(node));
    }

    template <typename Out, typename... Args>
        requires(ptr_variant)
    auto operator()(marker<Out>, Args&&... args) {
        auto node =
            make_unique<Out>(payload_builder(mark<Out>, std::forward<Args>(args)...), std::forward<Args>(args)...);

        return node;
    }
};

static_assert(NodeBuilder<BoxedNodeBuilder<>, empty, UniquePtrIndirection, true>);

} // namespace loxxy
