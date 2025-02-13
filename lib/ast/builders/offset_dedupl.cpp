module;
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cxxabi.h>
#include <iostream>
#include <loxxy/ast.hpp>
#include <memory>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
export module ast.offset_dedupl_builder;

import ast;
import ast.hash_payload_builder;
import utils.stupid_type_traits;
import utils.multi_vector;

using utils::multi_vector;

auto demangle(const char* name) -> std::string {

    int status = -4; // some arbitrary value to eliminate the compiler warning

    // enable c++11 by passing the flag -std=c++11 to g++
    std::unique_ptr<char, void (*)(void*)> res{abi::__cxa_demangle(name, nullptr, nullptr, &status), std::free};

    return (status == 0) ? res.get() : name;
}

template <typename... Ts>
void for_types(auto&& f) {
    (f.template operator()<Ts>(), ...);
}

export namespace loxxy {

template <typename offset_t, bool _ptr_variant = true>
struct OffsetDeduplBuilder {
    using Payload = NodeHash;
    using Indirection = OffsetPointerIndirection<offset_t>;
    static constexpr bool ptr_variant = true;
    USING_FAMILY(Payload, OffsetPointerIndirection<offset_t>, true);

    using Resolver = multi_vector<
        BinaryExpr, UnaryExpr, GroupingExpr, StringExpr, NumberExpr, BoolExpr, NilExpr, VarExpr, AssignExpr, CallExpr,
        PrintStmt, ExpressionStmt, VarDecl, FunDecl, BlockStmt, IfStmt, WhileStmt, ReturnStmt>;

    using Builder = HashPayloadBuilder<Indirection, ptr_variant, Resolver>;

    template <typename NodeType, typename... Args>
    auto operator()(marker<NodeType>, Args&&... args) {
        // for_types<
        //     BinaryExpr, UnaryExpr, GroupingExpr, StringExpr, NumberExpr, BoolExpr, NilExpr, VarExpr, AssignExpr,
        //     CallExpr, PrintStmt, ExpressionStmt, VarDecl, FunDecl, BlockStmt, IfStmt, WhileStmt, ReturnStmt>(
        //     [&nodes = this->nodes]<typename T>() {
        //         auto& vector = nodes.template get_vec<T>();
        //         std::cout << "  " << &vector << " " << sizeof(decltype(vector)) << " "
        //                   << demangle(typeid(decltype(vector)).name()) << std::endl;
        //     }
        // );
        auto& vector = nodes.template get_vec<NodeType>();

        Payload hash = payload_builder(mark<NodeType>, args...);

        const auto& it = hash_map.find(hash);
        if (it != hash_map.end())
            return offset_pointer<NodeType, offset_t>(it->second);

        vector.emplace_back(hash, std::forward<Args>(args)...);
        hash_map[hash] = vector.size() - 1;

        return offset_pointer<NodeType, offset_t>(vector.size() - 1);
    }

    auto get_resolver(this auto&& self) -> auto&& { return self.nodes; }

private:
    Resolver nodes;

    Builder payload_builder{nodes};
    std::unordered_map<Payload, offset_t> hash_map;
};

// template <
//     typename offset_t, typename _Payload, PayloadBuilder<_Payload, OffsetPointerIndirection<offset_t>, false>
//     Builder>
// struct OffsetBuilder<offset_t, _Payload, false, Builder> {
//     using Payload = _Payload;
//     using Indirection = OffsetPointerIndirection<offset_t>;
//     static constexpr bool ptr_variant = false;
//     USING_FAMILY(Payload, OffsetPointerIndirection<offset_t>, false);
//
//     using Resolver = multi_vector<Expression, Statement>;
//
//     template <typename... Args>
//     OffsetBuilder(Args&&... args) : payload_builder(std::forward<Args>(args)...) {}
//
//     ~OffsetBuilder() {
//         // std::cout << nodes.template get_vec<Expression>().size() << "\n"
//         //           << nodes.template get_vec<Statement>().size() << "\n";
//     }
//
//     template <ExpressionSTN NodeType, typename... Args>
//     auto operator()(marker<NodeType>, Args&&... args) {
//         auto& vector = nodes.template get_vec<Expression>();
//         Expression expr{NodeType(payload_builder(std::forward<Args>(args)...), std::forward<Args>(args)...)};
//         vector.emplace_back(expr);
//
//         return offset_pointer<Expression, offset_t>(vector.size() - 1);
//     }
//
//     template <StatementSTN NodeType, typename... Args>
//     auto operator()(marker<NodeType>, Args&&... args) {
//         auto& vector = nodes.template get_vec<Statement>();
//         Statement stmt{NodeType(payload_builder(std::forward<Args>(args)...), std::forward<Args>(args)...)};
//         vector.emplace_back(stmt);
//
//         return offset_pointer<Statement, offset_t>(vector.size() - 1);
//     }
//
//     auto get_resolver(this auto&& self) -> auto&& { return self.nodes; }
//
// private:
//     Resolver nodes;
//
//     Builder payload_builder;
// };

} // namespace loxxy
