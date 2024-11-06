module;

#include <vector>
#include <memory>
#include <type_traits>
#include <concepts>
#include <string>

export module ast:nodes;

import :token;
import :value;
import utils.string_store;
import utils.stupid_type_traits;
import utils.variant;

using utils::UniquePtrIndirection;
using utils::PointerIndirection;
using utils::SharedPtrIndirection;
using utils::PropConstIndirection;
using utils::MapTypes;
using std::same_as;
using std::true_type;
using std::false_type;
using utils::variant;


export namespace loxxy {

struct empty {};


template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct BinaryExpr;
template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct GroupingExpr;
template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct UnaryExpr;
template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct NumberExpr;
template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct StringExpr;
template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct BoolExpr;
template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct NilExpr;

template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
using Expression = variant<
    BinaryExpr<Payload, Indirection, ptr_variant>,
    UnaryExpr<Payload, Indirection, ptr_variant>,
    GroupingExpr<Payload, Indirection, ptr_variant>,
    NumberExpr<Payload, Indirection, ptr_variant>,
    StringExpr<Payload, Indirection, ptr_variant>,
    BoolExpr<Payload, Indirection, ptr_variant>,
    NilExpr<Payload, Indirection, ptr_variant>
>;

template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
class ExprPointer : public utils::WrappedVar<MapTypes<Expression<Payload, Indirection, ptr_variant>, Indirection>> {
    using Self = ExprPointer<Payload, Indirection, ptr_variant>;
    using Var = MapTypes<Expression<Payload, Indirection, ptr_variant>, Indirection>;
    using utils::WrappedVar<Var>::WrappedVar;
};

template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct BinaryExpr {
    Payload payload;
    ExprPointer<Payload, Indirection, ptr_variant> lhs;
    ExprPointer<Payload, Indirection, ptr_variant> rhs;
    Token op;
};

template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct GroupingExpr {
    Payload payload;
    ExprPointer<Payload, Indirection, ptr_variant> expr;
};

template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct UnaryExpr {
    Payload payload;
    ExprPointer<Payload, Indirection, ptr_variant> expr;
    Token op;
};

template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct NumberExpr {
    Payload payload;
    double x;
};

template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct StringExpr {
    Payload payload;
    const persistent_string<>* string;
};

template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct BoolExpr {
    Payload payload;
    bool x;
};

template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct NilExpr {
    Payload payload;
};


template<typename Payload = empty, bool ptr_variant = true>
using BoxedExpr = ExprPointer<Payload, UniquePtrIndirection, ptr_variant>;
template<typename Payload = empty, bool ptr_variant = true>
using RCExpr = ExprPointer<Payload, SharedPtrIndirection, ptr_variant>;


template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct ExpressionStmt {
    Payload payload;
    ExprPointer<Payload, Indirection, ptr_variant> expr;
};

template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct PrintStmt {
    Payload payload;
    ExprPointer<Payload, Indirection, ptr_variant> expr;
};

template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
using Statement = variant<
    ExpressionStmt<Payload, Indirection, ptr_variant>,
    PrintStmt<Payload, Indirection, ptr_variant>
>;

template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
class StmtPointer : public utils::WrappedVar<MapTypes<Statement<Payload, Indirection, ptr_variant>, Indirection>> {
    using Self = StmtPointer<Payload, Indirection, ptr_variant>;
    using Var = MapTypes<Statement<Payload, Indirection, ptr_variant>, Indirection>;
    using utils::WrappedVar<Var>::WrappedVar;
};

template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct TURoot {
    std::vector<StmtPointer<Payload, Indirection, ptr_variant>> statements;
};

template<typename Payload, typename Indirection, bool ptr_variant>
struct Family {
    using ExprPointer = ExprPointer<Payload, Indirection, ptr_variant>;

    using BinaryExpr = BinaryExpr<Payload, Indirection, ptr_variant>;
    using UnaryExpr = UnaryExpr<Payload, Indirection, ptr_variant>;
    using GroupingExpr = GroupingExpr<Payload, Indirection, ptr_variant>;
    using StringExpr = StringExpr<Payload, Indirection, ptr_variant>;
    using NumberExpr = NumberExpr<Payload, Indirection, ptr_variant>;
    using BoolExpr = BoolExpr<Payload, Indirection, ptr_variant>;
    using NilExpr = NilExpr<Payload, Indirection, ptr_variant>;

    using StmtPointer = StmtPointer<Payload, Indirection, ptr_variant>;

    using PrintStmt = PrintStmt<Payload, Indirection, ptr_variant>;
    using ExpressionStmt = ExpressionStmt<Payload, Indirection, ptr_variant>;

    using TURoot = TURoot<Payload, Indirection, ptr_variant>;
};

} // namespace loxxy
