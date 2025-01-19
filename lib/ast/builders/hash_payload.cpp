module;
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
    NodeHash() {}
    uint64_t hash;
};

static_assert(ConcreteSTN<NilExpr<NodeHash>>);

template <typename Indirection, bool ptr_variant, typename Resolver = void>
struct HashPayloadBuilder {
    using ExprPointer = ExprPointer<NodeHash, Indirection, ptr_variant>;
    NodeHash payload(const ExprPointer& node) { return utils::visit(extractor, node); }

    template <typename... Args>
    HashPayloadBuilder(Args&&... args) : extractor(std::forward<Args>(args)...) {}

    template <LiteralSTN NodeType, typename... Args>
    NodeHash operator()(marker<NodeType>, Args&&... args) {
        uint64_t hash = hash_ast(std::forward<Args>(args)...);
        return hash;
    }

    NodeHash operator()(auto, const ExprPointer& child, const Token& token) {
        uint64_t hash = hash_ast(payload(child).hash, token);
        return hash;
    }

    NodeHash operator()(auto, const ExprPointer& child) {
        uint64_t hash = hash_ast(payload(child).hash);
        return hash;
    }

    NodeHash operator()(auto, const ExprPointer& lhs, const ExprPointer& rhs, const Token& token) {
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
    uint64_t operator()(loxxy::NodeHash) { return 0; }
};

} // namespace std