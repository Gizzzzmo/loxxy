module;
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

    Builder payload_builder;
    Interpreter<Payload, UniquePtrIndirection, ptr_variant, void> interpreter;

    template <typename... Args>
    BoxedNodeBuilder(Args&&... args) : payload_builder(std::forward<Args>(args)...) {}

    template <typename Out, typename... Args>
    auto operator()(marker<Out>, Args&&... args) {
        auto node =
            make_unique<Out>(payload_builder(mark<Out>, std::forward<Args>(args)...), std::forward<Args>(args)...);

        return node;
    }

    template <typename Out, typename... Args>
    auto operator()(marker<Out>, const Payload& payload, Args&&... args) {
        return make_unique<ResolveNodeType<Payload, UniquePtrIndirection, true, Args...>>(
            payload, std::forward<Args>(args)...
        );
    }
};

static_assert(NodeBuilder<BoxedNodeBuilder<>, empty, UniquePtrIndirection, true>);

} // namespace loxxy
