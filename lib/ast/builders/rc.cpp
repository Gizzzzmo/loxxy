module;
#include <memory>
#include <utility>
export module ast.rc_node_builder;

import ast;
import utils.stupid_type_traits;

using std::make_shared;


export namespace loxxy {

template<
    typename Payload = empty,
    bool ptr_variant = true,
    PayloadBuilder<Payload, SharedPtrIndirection, ptr_variant> Builder = DefaultPayloadBuilder<Payload>
>
struct RCNodeBuilder {
    Builder payload_builder;

    template<typename... Args>
    RCNodeBuilder(Args&&... args) : payload_builder(std::forward<Args>(args)...) {}

    template<typename... Args> 
        requires (ConcreteSTN<ResolveNodeType<Payload, SharedPtrIndirection, true, Args...>>)
    RCExpr<Payload> operator()(Args&&... args) {
        return make_shared<ResolveNodeType<Payload, SharedPtrIndirection, true, Args...>>(
            payload_builder(std::forward<Args>(args)...),
            std::forward<Args>(args)...
        );
    }
};

} // namespace loxxy