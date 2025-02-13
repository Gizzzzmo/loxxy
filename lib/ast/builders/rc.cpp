module;
#include "loxxy/ast.hpp"
#include <memory>
#include <utility>
export module ast.rc_node_builder;

import ast;
import utils.stupid_type_traits;

using std::make_shared;

export namespace loxxy {

template <
    typename _Payload = empty, bool _ptr_variant = true,
    PayloadBuilder<_Payload, SharedPtrIndirection, _ptr_variant> Builder = DefaultPayloadBuilder<_Payload>>
struct RCNodeBuilder {
    using Payload = _Payload;
    using Indirection = SharedPtrIndirection;
    static constexpr bool ptr_variant = _ptr_variant;
    using Resolver = void;

    USING_FAMILY(Payload, Indirection, ptr_variant);
    Builder payload_builder;

    template <typename... Args>
    RCNodeBuilder(Args&&... args) : payload_builder(std::forward<Args>(args)...) {}

    // template <typename... Args>
    //     requires(ConcreteSTN<ResolveNodeType<Payload, SharedPtrIndirection, true, Args...>>)
    // RCExpr<Payload> operator()(Args&&... args) {
    //     return make_shared<ResolveNodeType<Payload, SharedPtrIndirection, true, Args...>>(
    //         payload_builder(std::forward<Args>(args)...), std::forward<Args>(args)...
    //     );
    // }
};

} // namespace loxxy
