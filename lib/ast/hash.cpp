module;
#include <type_traits>
#include <utility>
#include <cstddef>
#include <iostream>
#include <cstdint>
#include <cstring>

export module ast:hash;


import :nodes;
import :token;

import utils.stupid_type_traits;
import utils.string_store;
import utils.murmurhash;

export namespace std {
    template<>
    struct hash<loxxy::empty> {
        uint64_t operator()(loxxy::empty) {
            return 0;
        }
    };
}

using utils::persistent_string;
using utils::MurmurHash64A;


template<typename Head, typename... Tail>
constexpr void aggregate_data(std::byte* buffer, Head&& head, Tail&&... tail) {
    std::memcpy(buffer, &head, sizeof(std::remove_reference_t<Head>));
    if constexpr (sizeof...(Tail) != 0)
        aggregate_data<Tail...>(buffer + sizeof(std::remove_reference_t<Head>), std::forward<Tail>(tail)...);
}

template<typename... Ts>
struct HashData {
    static constexpr size_t size = (sizeof(std::remove_reference_t<Ts>) + ... );
    alignas(uint64_t) std::byte buffer[size];

    template<typename... Args> requires((std::same_as<std::remove_reference_t<Args>, Ts> && ...))
    constexpr HashData(Args&&... args) {
        aggregate_data(buffer, std::forward<Args>(args)...);
    }

    constexpr uint64_t murmurhash(uint64_t seed) {
        return MurmurHash64A(buffer, size, seed);
    }
};

template<typename... Ts>
HashData(Ts&&...) -> HashData<std::remove_reference_t<Ts>...>;


template<typename... Args>
constexpr uint64_t murmurhash(uint64_t seed, Args&&... args) {
    HashData data{
        std::forward<Args>(args)...
    };
    //std::cout << sizeof(data) << "\n";
    return data.murmurhash(seed);
}


export namespace loxxy {

template<typename Payload = empty>
uint64_t hash_ast(bool x, const Payload& payload = Payload{}) {
    uint64_t hash = murmurhash(0xe1efeb388f25f800ul, x, std::hash<Payload>{}(payload));

    // std::cout << "builder: bool: " << hash << " " << std::hash<Payload>{}(payload) << "\n";
    return hash;
}

template<typename Payload = empty>
uint64_t hash_ast(double x, const Payload& payload = Payload{}) {
    uint64_t hash = murmurhash(0xd238d87db2da11beul, x, std::hash<Payload>{}(payload));

    // std::cout << "builder: number: " << hash << " " << std::hash<Payload>{}(payload) << "\n";
    return hash;
}

template<typename Payload = empty>
uint64_t hash_ast(const Payload& payload = Payload{}) {
    uint64_t hash = murmurhash(0xbff8fc296456f734ul, std::hash<Payload>{}(payload));

    // std::cout << "builder: nil: " << hash << " " << std::hash<Payload>{}(payload) << "\n";
    return hash;
}

template<typename Payload = empty>
uint64_t hash_ast(const persistent_string<>* string, const Payload& payload = Payload{}) {
    uint64_t hash = murmurhash(0x86f5e30c23b5a93dul, &string, std::hash<Payload>{}(payload));

    // std::cout << "builder: string: " << hash << " " << std::hash<Payload>{}(payload) << "\n";
    return hash;
}

template<typename Payload = empty>
uint64_t hash_ast(uint64_t child_hash, const Payload& payload = Payload{}) {
    uint64_t hash = murmurhash(0, child_hash, std::hash<Payload>{}(payload));

    // std::cout << "builder: grouping: " << hash << " " << std::hash<Payload>{}(payload) << "\n";
    return hash;
}

template<typename Payload = empty>
uint64_t hash_ast(uint64_t child_hash, const Token& op, const Payload& payload = Payload{}) {
    uint64_t hash = murmurhash(0, child_hash, op.getType(), std::hash<Payload>{}(payload));

    // std::cout << "builder: unary: " << hash << " " << std::hash<Payload>{}(payload) << "\n";
    return hash;
}

template<typename Payload = empty>
uint64_t hash_ast(uint64_t lhs_hash, uint64_t rhs_hash, const Token& op, const Payload& payload = Payload{}) {
    uint64_t hash = murmurhash(0, lhs_hash, rhs_hash, op.getType(), std::hash<Payload>{}(payload));

    // std::cout << "builder: binary: " << hash << " " << std::hash<Payload>{}(payload) << "\n";
    // std::cout << "   " << lhs_hash << " " << rhs_hash << " " << op.getType() << "\n";
    return hash;
}


} // namespace loxxy
