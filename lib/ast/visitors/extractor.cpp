module;
#include <utility>
export module ast.extractor;
import ast;
import utils.stupid_type_traits;

using utils::IndirectVisitor;
using std::same_as;

export namespace loxxy {

template<typename Payload, typename Indirection, bool ptr_variant, typename Resolver = void>
struct Extractor : 
    public IndirectVisitor<
        Extractor<Payload, Indirection, ptr_variant, Resolver>,
        Resolver,
        Indirection
    >
{
    using Self = Extractor<Payload, Indirection, ptr_variant, Resolver>;
    using Parent = IndirectVisitor<Self, Resolver, Indirection>;
    
    using Parent::operator();
    using Parent::Parent;

    template<template<typename, typename, bool> class NodeTempl>
        requires(ConcreteSTN<NodeTempl<Payload, Indirection, ptr_variant>>)
    Payload& operator()(NodeTempl<Payload, Indirection, ptr_variant>& node) {
        return node.payload;
    }
};

template<typename Payload, typename Indirection, bool ptr_variant>
auto& payload_of(STNPointer<Payload, Indirection, ptr_variant>& node) {
    return visit(Extractor<Payload, Indirection, ptr_variant, void>{}, node);
}

template<typename Payload, typename Indirection, bool ptr_variant, typename Resolver>
auto& payload_of(STNPointer<Payload, Indirection, ptr_variant>& node, Resolver&& resolver) {
    return visit(
        Extractor<Payload, Indirection, ptr_variant, Resolver>{
            std::forward<Resolver>(resolver)
        }, 
        node
    );
}

template<typename Payload, typename Indirection, bool ptr_variant>
const auto& payload_of(const STNPointer<Payload, Indirection, ptr_variant>& node) {
    using NodeType = STNPointer<Payload, Indirection, ptr_variant>;
    return const_cast<const Payload&>(payload_of(const_cast<NodeType&>(node)));
}

template<typename Payload, typename Indirection, bool ptr_variant, typename Resolver>
const Payload& payload_of(const STNPointer<Payload, Indirection, ptr_variant>& node, Resolver&& resolver) {
    return const_cast<const Payload&>(payload_of(
        const_cast<STNPointer<Payload, Indirection, ptr_variant>&>(node),
        std::forward<Resolver>(resolver)
    ));
}


} // namespace loxxy
