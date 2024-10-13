module;
#include <cstdint>
export module ast.hasher;

import ast;
import utils.stupid_type_traits;

using utils::IndirectVisitor;

export namespace loxxy {

template<typename Payload, typename Indirection, bool ptr_variant, typename Resolver>
struct Hasher : 
    IndirectVisitor<
        Hasher<Payload, Indirection, ptr_variant, Resolver>,
        Resolver,
        Indirection
    >
{
    using Self = Hasher<Payload, Indirection, ptr_variant, Resolver>;
    using Parent = IndirectVisitor<Self, Resolver, Indirection>;

    using Parent::operator();
    using Parent::Parent;

    uint64_t operator()(const BinaryNode<Payload, Indirection, ptr_variant>& node) {
        uint64_t lhs_hash = visit(*this, node.lhs);
        uint64_t rhs_hash = visit(*this, node.rhs);

        return hash_ast(lhs_hash, rhs_hash, node.op, node.payload);
    }

    uint64_t operator()(const UnaryNode<Payload, Indirection, ptr_variant>& node) {
        uint64_t child_hash = visit(*this, node.expr);

        return hash_ast(child_hash, node.op, node.payload);
    }

    uint64_t operator()(const GroupingNode<Payload, Indirection, ptr_variant>& node) {
        uint64_t child_hash = visit(*this, node.expr);

        return hash_ast(child_hash, node.payload);
    }

    uint64_t operator()(const BoolNode<Payload, Indirection, ptr_variant>& node) {
        return hash_ast(node.x, node.payload);
    }

    uint64_t operator()(const StringNode<Payload, Indirection, ptr_variant>& node) {
        return hash_ast(node.string, node.payload);
    }

    uint64_t operator()(const NumberNode<Payload, Indirection, ptr_variant>& node) {
        return hash_ast(node.x, node.payload);
    }

    uint64_t operator()(const NilNode<Payload, Indirection, ptr_variant>& node) { 
        return hash_ast(node.payload);
    }
};

} // namespace loxxy