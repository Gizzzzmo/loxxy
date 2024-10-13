module;
#include <utility>
#include <cstdint>
#include <bits/functional_hash.h>
export module ast.hash_payload_builder;
import ast;
import ast.extractor;
import utils.stupid_type_traits;

using utils::IndirectVisitor;

export namespace loxxy {

struct NodeHash {
    NodeHash(uint64_t hash) : hash(hash) {}
    NodeHash() {}
    uint64_t hash;
};

static_assert(ConcreteSTN<NilNode<NodeHash>>);

template<typename Indirection, bool ptr_variant, typename Resolver = void>
struct HashPayloadBuilder {
    using STNPointer = STNPointer<NodeHash, Indirection, ptr_variant>;
    NodeHash payload(const STNPointer& node) {
        return visit(extractor, node);
    }

    template<typename... Args>
    HashPayloadBuilder(Args&&... args) : extractor(std::forward<Args>(args)...) {}

    template<typename... Args>
        requires (LiteralSTN<ResolveNodeType<NodeHash, Indirection, ptr_variant, Args...>>)
    NodeHash operator()(Args&&... args) {
        uint64_t hash = hash_ast(std::forward<Args>(args)...);
        return hash;
    }

    NodeHash operator()(const STNPointer& child, const Token& token) {
        uint64_t hash = hash_ast(payload(child).hash, token);
        return hash;
    }

    NodeHash operator()(const STNPointer& child) {
        uint64_t hash = hash_ast(payload(child).hash);
        return hash;
    }

    NodeHash operator()(const STNPointer& lhs, const STNPointer& rhs, const Token& token) {
        uint64_t hash = hash_ast(payload(lhs).hash, payload(rhs).hash, token);
        return hash;
    }
    private:
        Extractor<NodeHash, Indirection, ptr_variant, Resolver> extractor;
};

} // namespace loxxy

export namespace std {

template<>
struct hash<loxxy::NodeHash> {
    uint64_t operator()(loxxy::NodeHash) {
        return 0;
    }
};

} // namespace std