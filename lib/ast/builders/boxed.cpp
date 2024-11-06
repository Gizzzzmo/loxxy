module;
#include <memory>
#include <utility>
#include <iostream>
export module ast.boxed_node_builder;

import ast;
import ast.interpreter;
import utils.stupid_type_traits;
import utils.variant;

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

    template<typename Out, typename... Args> 
    auto operator()(marker<Out>, Args&&... args) {
        auto node = make_unique<Out>(
            payload_builder(mark<Out>, std::forward<Args>(args)...),
            std::forward<Args>(args)...
        );
        //try {
        //    Value value = utils::visit(interpreter, node);
        //    std::cout << value << std::endl;
        //} catch (const TypeError& err) {
        //    std::cerr << err.what() << std::endl;
        //}

        return node;
    }

    template<typename Out, typename... Args> 
    auto operator()(marker<Out>, const Payload& payload, Args&&... args) {
        return make_unique<ResolveNodeType<Payload, UniquePtrIndirection, true, Args...>>(
            payload,
            std::forward<Args>(args)...
        );
    }
};

static_assert(NodeBuilder<BoxedNodeBuilder<>, empty, UniquePtrIndirection, true>);

} // namespace loxxy