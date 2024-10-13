module;
#include <memory>
#include <utility>
#include <iostream>
export module ast.boxed_node_builder;

import ast;
import ast.interpreter;
import utils.stupid_type_traits;

using std::make_unique;

export namespace loxxy {

template<
    typename Payload = empty,
    bool ptr_variant = true,
    PayloadBuilder<Payload, UniquePtrIndirection, ptr_variant> Builder = DefaultPayloadBuilder<Payload>
>
struct BoxedNodeBuilder {
    Builder payload_builder;
    Interpreter<Payload, UniquePtrIndirection, ptr_variant, void> interpreter;

    template<typename... Args>
    BoxedNodeBuilder(Args&&... args) : payload_builder(std::forward<Args>(args)...) {}

    template<typename... Args> 
        requires (ConcreteSTN<ResolveNodeType<Payload, UniquePtrIndirection, true, Args...>>)
    BoxedSTN<Payload> operator()(Args&&... args) {
        BoxedSTN<Payload> node = make_unique<ResolveNodeType<Payload, UniquePtrIndirection, true, Args...>>(
            payload_builder(std::forward<Args>(args)...),
            std::forward<Args>(args)...
        );
        try {
            Value value = visit(interpreter, node);
            std::cout << value << std::endl;
        } catch (const TypeError& err) {
            std::cerr << err.what() << std::endl;
        }

        return node;
    }
};

static_assert(NodeBuilder<BoxedNodeBuilder<>, empty, UniquePtrIndirection, true>);

} // namespace loxxy