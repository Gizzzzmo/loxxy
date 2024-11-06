module;
#include <type_traits>
#include <utility>
#include <concepts>
#include <memory>
export module ast:building;
import :nodes;
import :token;
import :type_traits;
import utils.string_store;
import utils.stupid_type_traits;

using std::same_as;
using std::convertible_to;
using std::make_unique;

export namespace loxxy {

template<typename T, typename Payload, typename Indirection, bool ptr_variant>
concept NodeBuilder = requires(
    T t,
    ExprPointer<Payload, Indirection, ptr_variant>&& node,
    Token token,
    double x,
    bool b,
    const persistent_string<>* str,
    marker<BinaryExpr<Payload, Indirection, ptr_variant>> binary_expr,
    marker<UnaryExpr<Payload, Indirection, ptr_variant>> unary_expr,
    marker<GroupingExpr<Payload, Indirection, ptr_variant>> grouping_expr,
    marker<NumberExpr<Payload, Indirection, ptr_variant>> number_expr,
    marker<StringExpr<Payload, Indirection, ptr_variant>> string_expr,
    marker<BoolExpr<Payload, Indirection, ptr_variant>> bool_expr,
    marker<NilExpr<Payload, Indirection, ptr_variant>> nil_expr,
    marker<ExpressionStmt<Payload, Indirection, ptr_variant>> expr_stmt,
    marker<PrintStmt<Payload, Indirection, ptr_variant>> print_stmt
) {
    { t(
        binary_expr,
        std::move(node),
        std::move(node),
        token
    ) } -> convertible_to<ExprPointer<Payload, Indirection, ptr_variant>>;
    { t(
        unary_expr,
        std::move(node),
        token
    ) } -> convertible_to<ExprPointer<Payload, Indirection, ptr_variant>>;
    { t(
        grouping_expr,
        std::move(node)
    ) } -> convertible_to<ExprPointer<Payload, Indirection, ptr_variant>>;
    { t(string_expr, str) } -> convertible_to<ExprPointer<Payload, Indirection, ptr_variant>>;
    { t(number_expr, x) } -> convertible_to<ExprPointer<Payload, Indirection, ptr_variant>>;
    { t(nil_expr) } -> convertible_to<ExprPointer<Payload, Indirection, ptr_variant>>;
    { t(bool_expr, b) } -> convertible_to<ExprPointer<Payload, Indirection, ptr_variant>>;
    { t(expr_stmt, std::move(node)) } -> convertible_to<StmtPointer<Payload, Indirection, ptr_variant>>;
    { t(print_stmt, std::move(node)) } -> convertible_to<StmtPointer<Payload, Indirection, ptr_variant>>;
};

template<typename T, typename Payload, typename Indirection, bool ptr_variant>
concept NodeCopier = requires(
    T t,
    ExprPointer<Payload, Indirection, ptr_variant>&& node,
    Token token,
    double x,
    bool b,
    const persistent_string<>* str,
    const Payload& payload,
    marker<BinaryExpr<Payload, Indirection, ptr_variant>> binary_expr,
    marker<UnaryExpr<Payload, Indirection, ptr_variant>> unary_expr,
    marker<GroupingExpr<Payload, Indirection, ptr_variant>> grouping_expr,
    marker<NumberExpr<Payload, Indirection, ptr_variant>> number_expr,
    marker<StringExpr<Payload, Indirection, ptr_variant>> string_expr,
    marker<BoolExpr<Payload, Indirection, ptr_variant>> bool_expr,
    marker<NilExpr<Payload, Indirection, ptr_variant>> nil_expr,
    marker<ExpressionStmt<>> expr_stmt,
    marker<PrintStmt<>> print_stmt
) {
    { t(
        binary_expr,
        payload,
        std::move(node),
        std::move(node),
        token
    ) } -> convertible_to<ExprPointer<Payload, Indirection, ptr_variant>>;
    { t(
        unary_expr,
        payload,
        std::move(node),
        token
    ) } -> convertible_to<ExprPointer<Payload, Indirection, ptr_variant>>;
    { t(
        grouping_expr,
        payload,
        std::move(node)
    ) } -> convertible_to<ExprPointer<Payload, Indirection, ptr_variant>>;
    { t(string_expr, payload, str) } -> convertible_to<ExprPointer<Payload, Indirection, ptr_variant>>;
    { t(number_expr, payload, x) } -> convertible_to<ExprPointer<Payload, Indirection, ptr_variant>>;
    { t(nil_expr, payload) } -> convertible_to<ExprPointer<Payload, Indirection, ptr_variant>>;
    { t(bool_expr, payload, b) } -> convertible_to<ExprPointer<Payload, Indirection, ptr_variant>>;
    { t(expr_stmt, std::move(node)) } -> convertible_to<StmtPointer<Payload, Indirection, ptr_variant>>;
    { t(print_stmt, std::move(node)) } -> convertible_to<StmtPointer<Payload, Indirection, ptr_variant>>;
};


template<typename T, typename Payload, typename Indirection, bool ptr_variant>
concept PayloadBuilder = requires(
    T t,
    ExprPointer<Payload, Indirection, ptr_variant>& node,
    Token& token,
    double& x,
    bool& b,
    const persistent_string<>* str,
    marker<BinaryExpr<Payload, Indirection, ptr_variant>> binary_expr,
    marker<UnaryExpr<Payload, Indirection, ptr_variant>> unary_expr,
    marker<GroupingExpr<Payload, Indirection, ptr_variant>> grouping_expr,
    marker<NumberExpr<Payload, Indirection, ptr_variant>> number_expr,
    marker<StringExpr<Payload, Indirection, ptr_variant>> string_expr,
    marker<BoolExpr<Payload, Indirection, ptr_variant>> bool_expr,
    marker<NilExpr<Payload, Indirection, ptr_variant>> nil_expr,
    marker<ExpressionStmt<>> expr_stmt,
    marker<PrintStmt<>> print_stmt
) {
    { t(binary_expr, node, node, token) } -> same_as<Payload>;
    { t(unary_expr, node, token) } -> same_as<Payload>;
    { t(grouping_expr, node) } -> same_as<Payload>;
    { t(string_expr, str) } -> same_as<Payload>;
    { t(number_expr, x) } -> same_as<Payload>;
    { t(nil_expr) } -> same_as<Payload>;
    { t(bool_expr, b) } -> same_as<Payload>;
    { t(expr_stmt, node) } -> same_as<Payload>;
    { t(print_stmt, node) } -> same_as<Payload>;
};

template<typename Payload = empty>
struct DefaultPayloadBuilder {
    template<typename... Args>
    constexpr Payload operator()(Args&&...) const { return Payload{}; }
};

static_assert(PayloadBuilder<DefaultPayloadBuilder<>, empty, UniquePtrIndirection, true>);
static_assert(PayloadBuilder<DefaultPayloadBuilder<int>, int, UniquePtrIndirection, true>);
static_assert(PayloadBuilder<DefaultPayloadBuilder<>, empty, PointerIndirection, true>);

} // namespace loxxy
