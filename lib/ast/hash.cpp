module;
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <type_traits>
#include <utility>
#include <vector>

export module ast:hash;

import :nodes;
import :token;
import :type_traits;

import utils.stupid_type_traits;
import utils.string_store;
import utils.murmurhash;

export namespace std {
template <>
struct hash<loxxy::empty> {
    uint64_t operator()(loxxy::empty) { return 0; }
};
} // namespace std

using utils::MurmurHash64A;
using utils::persistent_string;

constexpr void aggregate_data(std::byte* buffer) {}

template <typename Head, typename... Tail>
constexpr void aggregate_data(std::byte* buffer, Head&& head, Tail&&... tail) {
    std::memcpy(buffer, &head, sizeof(std::remove_reference_t<Head>));
    if constexpr (sizeof...(Tail) != 0)
        aggregate_data<Tail...>(buffer + sizeof(std::remove_reference_t<Head>), std::forward<Tail>(tail)...);
}

template <typename... Ts>
struct HashData {
    static constexpr size_t size = (sizeof(std::remove_reference_t<Ts>) + ... + 0);
    alignas(uint64_t) std::array<std::byte, size> buffer;

    template <typename... Args>
        requires((std::same_as<std::remove_reference_t<Args>, Ts> && ...))
    constexpr HashData(Args&&... args) {
        aggregate_data(buffer.data(), std::forward<Args>(args)...);
    }

    constexpr uint64_t murmurhash(uint64_t seed) { return MurmurHash64A(buffer.data(), size, seed); }
};

template <typename... Ts>
HashData(Ts&&...) -> HashData<std::remove_reference_t<Ts>...>;

template <typename... Args>
constexpr uint64_t murmurhash(uint64_t seed, Args&&... args) {
    HashData data{std::forward<Args>(args)...};
    // std::cout << sizeof(data) << "\n";
    return data.murmurhash(seed);
}

using namespace loxxy;

template <ConcreteSTN NodeType>
struct HashSeedImpl;

template <uint64_t x>
using constant = std::integral_constant<uint64_t, x>;

template <typename Payload, typename Indirection, bool ptr_variant>
struct HashSeedImpl<BinaryExpr<Payload, Indirection, ptr_variant>> : constant<0xdf19dc173a8e41a2> {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct HashSeedImpl<UnaryExpr<Payload, Indirection, ptr_variant>> : constant<0x80cb5a2bf8b9530d> {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct HashSeedImpl<GroupingExpr<Payload, Indirection, ptr_variant>> : constant<0x7defd7fffbea69b0> {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct HashSeedImpl<StringExpr<Payload, Indirection, ptr_variant>> : constant<0xca5701cbb09a8b67> {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct HashSeedImpl<NumberExpr<Payload, Indirection, ptr_variant>> : constant<0x966b69e4958b14b4> {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct HashSeedImpl<BoolExpr<Payload, Indirection, ptr_variant>> : constant<0xdf19dc173a8e41a2> {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct HashSeedImpl<NilExpr<Payload, Indirection, ptr_variant>> : constant<0xce507b0627041fb7> {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct HashSeedImpl<VarExpr<Payload, Indirection, ptr_variant>> : constant<0x133642d7af86ffa1> {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct HashSeedImpl<AssignExpr<Payload, Indirection, ptr_variant>> : constant<0x5338a440b01371e1> {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct HashSeedImpl<CallExpr<Payload, Indirection, ptr_variant>> : constant<0x219321066ff025b2> {};

template <typename Payload, typename Indirection, bool ptr_variant>
struct HashSeedImpl<PrintStmt<Payload, Indirection, ptr_variant>> : constant<0x71e036f691c86a45> {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct HashSeedImpl<ExpressionStmt<Payload, Indirection, ptr_variant>> : constant<0x42393e687862b5db> {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct HashSeedImpl<VarDecl<Payload, Indirection, ptr_variant>> : constant<0xab68e3a58d3d1488> {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct HashSeedImpl<FunDecl<Payload, Indirection, ptr_variant>> : constant<0xbff016c652ab530f> {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct HashSeedImpl<BlockStmt<Payload, Indirection, ptr_variant>> : constant<0x67e55544689b663b> {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct HashSeedImpl<IfStmt<Payload, Indirection, ptr_variant>> : constant<0x82408d589ed26d2c> {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct HashSeedImpl<WhileStmt<Payload, Indirection, ptr_variant>> : constant<0x7d493bf5efc2588b> {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct HashSeedImpl<ReturnStmt<Payload, Indirection, ptr_variant>> : constant<0x8fcc111160a8cd3a> {};

// template <typename Payload, typename Indirection, bool ptr_variant>
// struct HashSeedImpl<NilExpr<Payload, Indirection, ptr_variant>> : constant<0x80b638fc880e8b96> {};
// template <typename Payload, typename Indirection, bool ptr_variant>
// struct HashSeedImpl<NilExpr<Payload, Indirection, ptr_variant>> : constant<0xc3b2175aab3a5a98> {};
// template <typename Payload, typename Indirection, bool ptr_variant>
// struct HashSeedImpl<NilExpr<Payload, Indirection, ptr_variant>> : constant<0x1bdbde632013da01> {};
// template <typename Payload, typename Indirection, bool ptr_variant>
// struct HashSeedImpl<NilExpr<Payload, Indirection, ptr_variant>> : constant<0x8c37d9c4f7e0eb43> {};
// template <typename Payload, typename Indirection, bool ptr_variant>
// struct HashSeedImpl<NilExpr<Payload, Indirection, ptr_variant>> : constant<0x8dfcce11e9558c5a> {};
// template <typename Payload, typename Indirection, bool ptr_variant>
// struct HashSeedImpl<NilExpr<Payload, Indirection, ptr_variant>> : constant<0x0a2e19adc429edfe> {};

export namespace loxxy {

template <ConcreteSTN NodeType>
constexpr uint64_t hash_seed = HashSeedImpl<NodeType>::value;

template <typename T>
auto pre_hash_vector(const T& t) -> const T& {
    return t;
}

template <typename T>
auto pre_hash_vector(const std::vector<T>& vec) -> uint64_t {
    return MurmurHash64A(vec.data(), vec.size() * sizeof(T), 0);
}

template <ConcreteSTN NodeType, typename... Args>
auto hash_ast(Args&&... args) -> uint64_t {
    uint64_t hash = murmurhash(hash_seed<NodeType>, pre_hash_vector(std::forward<Args>(args))...);

    return hash;
}

} // namespace loxxy
