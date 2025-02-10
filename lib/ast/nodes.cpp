module;

#include "loxxy/ast.hpp"
#include <concepts>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

export module ast:nodes;

import :token;
import :value;
import utils.string_store;
import utils.stupid_type_traits;
import utils.variant;

using std::false_type;
using std::same_as;
using std::true_type;
using utils::MapTypes;
using utils::PointerIndirection;
using utils::PropConstIndirection;
using utils::SharedPtrIndirection;
using utils::UniquePtrIndirection;
using utils::variant;

export namespace loxxy {

struct empty {};

template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct BinaryExpr;
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct GroupingExpr;
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct UnaryExpr;
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct NumberExpr;
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct StringExpr;
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct BoolExpr;
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct NilExpr;
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct VarExpr;
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct AssignExpr;
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct CallExpr;

template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct BlockStmt;
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct FunDecl;
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct IfStmt;
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct WhileStmt;
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct ExpressionStmt;
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct PrintStmt;
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct VarDecl;
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct ReturnStmt;

template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
using Expression = variant<
    BinaryExpr<Payload, Indirection, ptr_variant>, UnaryExpr<Payload, Indirection, ptr_variant>,
    GroupingExpr<Payload, Indirection, ptr_variant>, NumberExpr<Payload, Indirection, ptr_variant>,
    StringExpr<Payload, Indirection, ptr_variant>, BoolExpr<Payload, Indirection, ptr_variant>,
    NilExpr<Payload, Indirection, ptr_variant>, VarExpr<Payload, Indirection, ptr_variant>,
    AssignExpr<Payload, Indirection, ptr_variant>, CallExpr<Payload, Indirection, ptr_variant>>;

template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
using Statement = variant<
    ExpressionStmt<Payload, Indirection, ptr_variant>, PrintStmt<Payload, Indirection, ptr_variant>,
    VarDecl<Payload, Indirection, ptr_variant>, BlockStmt<Payload, Indirection, ptr_variant>,
    IfStmt<Payload, Indirection, ptr_variant>, WhileStmt<Payload, Indirection, ptr_variant>,
    FunDecl<Payload, Indirection, ptr_variant>, ReturnStmt<Payload, Indirection, ptr_variant>>;

template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
class ExprPointer : public utils::WrappedVar<MapTypes<Expression<Payload, Indirection, ptr_variant>, Indirection>> {
    using Self = ExprPointer<Payload, Indirection, ptr_variant>;
    using Var = MapTypes<Expression<Payload, Indirection, ptr_variant>, Indirection>;
    using Parent = utils::WrappedVar<Var>;
    using Parent::Parent;
};

template <typename Payload, typename Indirection>
class ExprPointer<Payload, Indirection, false>
    : public utils::WrappedVar<Expression<Payload, Indirection, false>, Indirection> {
    using Self = ExprPointer<Payload, Indirection, false>;
    using Parent = utils::WrappedVar<Expression<Payload, Indirection, false>, Indirection>;
    using Parent::Parent;
};

template <typename Payload = empty, bool ptr_variant = true>
using BoxedExpr = ExprPointer<Payload, UniquePtrIndirection, ptr_variant>;
template <typename Payload = empty, bool ptr_variant = true>
using RCExpr = ExprPointer<Payload, SharedPtrIndirection, ptr_variant>;

template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
class StmtPointer : public utils::WrappedVar<MapTypes<Statement<Payload, Indirection, ptr_variant>, Indirection>> {
    using Self = StmtPointer<Payload, Indirection, ptr_variant>;
    using Var = MapTypes<Statement<Payload, Indirection, ptr_variant>, Indirection>;
    using Parent = utils::WrappedVar<Var>;
    using Parent::Parent;
};

template <typename Payload, typename Indirection>
class StmtPointer<Payload, Indirection, false>
    : public utils::WrappedVar<Statement<Payload, Indirection, false>, Indirection> {
    using Self = StmtPointer<Payload, Indirection, false>;
    using Parent = utils::WrappedVar<Statement<Payload, Indirection, false>, Indirection>;
    using Parent::Parent;
};

template <typename Payload = empty, bool ptr_variant = true>
using BoxedStmt = StmtPointer<Payload, UniquePtrIndirection, ptr_variant>;
template <typename Payload = empty, bool ptr_variant = true>
using RCStmt = StmtPointer<Payload, SharedPtrIndirection, ptr_variant>;

template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct BinaryExpr {
    Payload payload;
    ExprPointer<Payload, Indirection, ptr_variant> lhs;
    ExprPointer<Payload, Indirection, ptr_variant> rhs;
    Token op;
};
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct GroupingExpr {
    Payload payload;
    ExprPointer<Payload, Indirection, ptr_variant> expr;
};
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct UnaryExpr {
    Payload payload;
    ExprPointer<Payload, Indirection, ptr_variant> expr;
    Token op;
};
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct NumberExpr {
    Payload payload;
    double x;
};
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct StringExpr {
    Payload payload;
    const persistent_string<>* string;
};
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct BoolExpr {
    Payload payload;
    bool x;
};
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct NilExpr {
    Payload payload;
};
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct VarExpr {
    Payload payload;
    const persistent_string<>* identifier;
};
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct AssignExpr {
    Payload payload;
    const persistent_string<>* identifier;
    ExprPointer<Payload, Indirection, ptr_variant> expr;
};
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct CallExpr {
    Payload payload;
    ExprPointer<Payload, Indirection, ptr_variant> callee;
    std::vector<ExprPointer<Payload, Indirection, ptr_variant>> arguments;
};
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct ExpressionStmt {
    Payload payload;
    ExprPointer<Payload, Indirection, ptr_variant> expr;
};

template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct PrintStmt {
    Payload payload;
    ExprPointer<Payload, Indirection, ptr_variant> expr;
};
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct VarDecl {
    Payload payload;
    const persistent_string<>* identifier;
    std::optional<ExprPointer<Payload, Indirection, ptr_variant>> expr;
};
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct BlockStmt {
    Payload payload;
    std::vector<StmtPointer<Payload, Indirection, ptr_variant>> statements;
};
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct FunDecl {
    Payload payload;
    const persistent_string<>* identifier;
    std::vector<const persistent_string<>*> args;
    std::vector<StmtPointer<Payload, Indirection, ptr_variant>> body;
};
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct IfStmt {
    Payload payload;
    ExprPointer<Payload, Indirection, ptr_variant> condition;
    StmtPointer<Payload, Indirection, ptr_variant> then_branch;
    std::optional<StmtPointer<Payload, Indirection, ptr_variant>> else_branch;
};
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct WhileStmt {
    Payload payload;
    ExprPointer<Payload, Indirection, ptr_variant> condition;
    StmtPointer<Payload, Indirection, ptr_variant> body;
};
template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct ReturnStmt {
    Payload payload;
    ExprPointer<Payload, Indirection, ptr_variant> expr;
};

template <typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct TURoot {
    std::vector<StmtPointer<Payload, Indirection, ptr_variant>> statements;
};

template <typename Payload, typename Indirection, bool ptr_variant>
struct Family {
    USING_FAMILY(Payload, Indirection, ptr_variant);
};

template <typename Family>
void print_family() {
    std::cout << "Expression:       " << sizeof(typename Family::Expression) << std::endl;
    std::cout << "Statement:        " << sizeof(typename Family::Statement) << std::endl;
    std::cout << "ExprPointer:      " << sizeof(typename Family::ExprPointer) << std::endl;
    std::cout << "StmtPointer:      " << sizeof(typename Family::StmtPointer) << std::endl;
    std::cout << "  BinaryExpr:     " << sizeof(typename Family::BinaryExpr) << std::endl;
    std::cout << "  UnaryExpr:      " << sizeof(typename Family::UnaryExpr) << std::endl;
    std::cout << "  GroupingExpr:   " << sizeof(typename Family::GroupingExpr) << std::endl;
    std::cout << "  StringExpr:     " << sizeof(typename Family::StringExpr) << std::endl;
    std::cout << "  NumberExpr:     " << sizeof(typename Family::NumberExpr) << std::endl;
    std::cout << "  BoolExpr:       " << sizeof(typename Family::BoolExpr) << std::endl;
    std::cout << "  NilExpr:        " << sizeof(typename Family::NilExpr) << std::endl;
    std::cout << "  VarExpr:        " << sizeof(typename Family::VarExpr) << std::endl;
    std::cout << "  AssignExpr:     " << sizeof(typename Family::AssignExpr) << std::endl;
    std::cout << "  CallExpr:       " << sizeof(typename Family::CallExpr) << std::endl;

    std::cout << "  PrintStmt:      " << sizeof(typename Family::PrintStmt) << std::endl;
    std::cout << "  ExpressionStmt: " << sizeof(typename Family::ExpressionStmt) << std::endl;
    std::cout << "  VarDecl:        " << sizeof(typename Family::VarDecl) << std::endl;
    std::cout << "  FunDecl:        " << sizeof(typename Family::FunDecl) << std::endl;
    std::cout << "  BlockStmt:      " << sizeof(typename Family::BlockStmt) << std::endl;
    std::cout << "  IfStmt:         " << sizeof(typename Family::IfStmt) << std::endl;
    std::cout << "  WhileStmt:      " << sizeof(typename Family::WhileStmt) << std::endl;
    std::cout << "  ReturnStmt:     " << sizeof(typename Family::ReturnStmt) << std::endl;
}

} // namespace loxxy
