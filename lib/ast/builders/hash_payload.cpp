module;
#include "loxxy/ast.hpp"
#include <bits/functional_hash.h>
#include <cstdint>
#include <utility>
export module ast.hash_payload_builder;
import ast;
import ast.extractor;
import utils.stupid_type_traits;
import utils.variant;

export namespace loxxy {

struct NodeHash {
    NodeHash(uint64_t hash) : hash(hash) {}
    NodeHash() = default;
    uint64_t hash;
};

template <typename Indirection, bool ptr_variant, typename Resolver = void>
struct HashPayloadBuilder {
    using Payload = NodeHash;
    USING_FAMILY(Payload, Indirection, ptr_variant);

    auto payload(const ExprPointer& node) -> NodeHash { return utils::visit(extractor, node); }

    template <typename... Args>
    HashPayloadBuilder(Args&&... args) : extractor(std::forward<Args>(args)...) {}

    template <LiteralSTN NodeType, typename... Args>
    auto operator()(marker<NodeType>, Args&&... args) -> NodeHash {
        uint64_t hash = hash_ast(std::forward<Args>(args)...);
        return hash;
    }

    auto operator()(auto, const ExprPointer& child, const Token& token) -> NodeHash {
        uint64_t hash = hash_ast(payload(child).hash, token);
        return hash;
    }

    auto operator()(auto, const ExprPointer& child) -> NodeHash {
        uint64_t hash = hash_ast(payload(child).hash);
        return hash;
    }

    auto operator()(auto, const ExprPointer& lhs, const ExprPointer& rhs, const Token& token) -> NodeHash {
        uint64_t hash = hash_ast(payload(lhs).hash, payload(rhs).hash, token);
        return hash;
    }

private:
    Extractor<NodeHash, Indirection, ptr_variant, Resolver> extractor;
};

} // namespace loxxy

export namespace std {

template <>
struct hash<loxxy::NodeHash> {
    auto operator()(loxxy::NodeHash) -> uint64_t { return 0; }
};

} // namespace std
