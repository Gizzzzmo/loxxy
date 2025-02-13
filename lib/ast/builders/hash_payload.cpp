module;
#include "loxxy/ast.hpp"
#include <bits/functional_hash.h>
#include <cstdint>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>
export module ast.hash_payload_builder;
import ast;
import ast.extractor;
import utils.stupid_type_traits;
import utils.variant;

export namespace loxxy {

// struct NodeHash {
//     NodeHash(uint64_t hash) : hash(hash) {}
//     NodeHash() = default;
//     uint64_t hash;
// };

using NodeHash = uint64_t;

template <typename Indirection, bool ptr_variant, typename Resolver = void>
struct HashPayloadBuilder {
    using Payload = NodeHash;
    USING_FAMILY(Payload, Indirection, ptr_variant);

    HashPayloadBuilder(Resolver& res)
        requires(!std::same_as<Resolver, void>)
        : extractor(res) {}

    template <typename... Args>
    HashPayloadBuilder(Args&&... args) : extractor(std::forward<Args>(args)...) {}

    template <LeafSTN NodeType, typename... Args>
    auto operator()(marker<NodeType>, const Args&... args) -> NodeHash {
        NodeHash hash = hash_ast<NodeType>(args...);
        return hash;
    }

    template <InnerSTN NodeType, typename... Args>
    auto operator()(marker<NodeType>, const Args&... args) -> NodeHash {
        NodeHash hash = hash_ast<NodeType>(extract_hash(args)...);
        return hash;
    }

private:
    template <typename T>
    auto extract_hash(const T& t) -> const T& {
        return t;
    }

    template <STNPointer PointerType>
    auto extract_hash(const PointerType& pointer) -> NodeHash {
        return utils::visit(extractor, pointer);
    }

    template <STNPointer PointerType>
    auto extract_hash(const std::optional<PointerType>& maybe_pointer) -> NodeHash {
        if (maybe_pointer.has_value())
            return utils::visit(extractor, maybe_pointer.value());
        else
            return 0;
    }

    template <STNPointer PointerType>
    auto extract_hash(const std::vector<PointerType>& pointer_vector) -> std::vector<NodeHash> {
        std::vector<NodeHash> hash_vector;
        hash_vector.reserve(pointer_vector.size());
        for (const PointerType& pointer : pointer_vector) {
            hash_vector.push_back(utils::visit(extractor, pointer));
        }

        return hash_vector;
    }

    Extractor<NodeHash, Indirection, ptr_variant, std::add_lvalue_reference_t<Resolver&>> extractor;
};

} // namespace loxxy

export namespace std {

// template <>
// struct hash<loxxy::NodeHash> {
//     auto operator()(loxxy::NodeHash) const -> uint64_t { return 0; }
// };

} // namespace std
