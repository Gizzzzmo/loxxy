module;
#include <type_traits>
export module ast:type_traits;

import :nodes;
import :token;
import utils.stupid_type_traits;

using std::false_type;
using std::true_type;

export namespace loxxy {

template <typename T>
struct marker {};

template <typename T>
constexpr marker<T> mark{};

template <typename T>
struct LiteralSTNImpl : false_type {};

template <typename Payload, typename Indirection, bool ptr_variant>
struct LiteralSTNImpl<NumberExpr<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct LiteralSTNImpl<StringExpr<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct LiteralSTNImpl<BoolExpr<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct LiteralSTNImpl<NilExpr<Payload, Indirection, ptr_variant>> : true_type {};

template <typename T>
concept LiteralSTN = LiteralSTNImpl<T>::value;

template <typename T>
struct InnerSTNImpl : false_type {};

template <typename Payload, typename Indirection, bool ptr_variant>
struct InnerSTNImpl<BinaryExpr<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct InnerSTNImpl<UnaryExpr<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct InnerSTNImpl<GroupingExpr<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct InnerSTNImpl<PrintStmt<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct InnerSTNImpl<ExpressionStmt<Payload, Indirection, ptr_variant>> : true_type {};

template <typename T>
struct StatementSTNImpl : false_type {};

template <typename Payload, typename Indirection, bool ptr_variant>
struct StatementSTNImpl<PrintStmt<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct StatementSTNImpl<ExpressionStmt<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct StatementSTNImpl<VarDecl<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct StatementSTNImpl<FunDecl<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct StatementSTNImpl<BlockStmt<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct StatementSTNImpl<IfStmt<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct StatementSTNImpl<WhileStmt<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct StatementSTNImpl<ReturnStmt<Payload, Indirection, ptr_variant>> : true_type {};

template <typename T>
struct ExpressionSTNImpl : false_type {};

template <typename Payload, typename Indirection, bool ptr_variant>
struct ExpressionSTNImpl<BinaryExpr<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct ExpressionSTNImpl<UnaryExpr<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct ExpressionSTNImpl<GroupingExpr<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct ExpressionSTNImpl<StringExpr<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct ExpressionSTNImpl<NumberExpr<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct ExpressionSTNImpl<BoolExpr<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct ExpressionSTNImpl<NilExpr<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct ExpressionSTNImpl<VarExpr<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct ExpressionSTNImpl<AssignExpr<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct ExpressionSTNImpl<CallExpr<Payload, Indirection, ptr_variant>> : true_type {};

template <typename T>
concept InnerSTN = InnerSTNImpl<T>::value;

template <typename T>
concept ConcreteSTN = InnerSTN<T> || LiteralSTN<T>;

template <typename T>
concept ExpressionSTN = ExpressionSTNImpl<T>::value;

template <typename T>
concept StatementSTN = StatementSTNImpl<T>::value;

template <typename Payload, typename Indirection, bool ptr_variant, typename... Args>
struct ResolveNodeTypeImpl;

template <
    typename Payload, typename Indirection, bool ptr_variant, typename PayloadLhs, typename IndirectionLhs,
    bool ptr_variant_lhs, typename PayloadRhs, typename IndirectionRhs, bool ptr_variant_rhs>
struct ResolveNodeTypeImpl<
    Payload, Indirection, ptr_variant, ExprPointer<PayloadLhs, IndirectionLhs, ptr_variant_lhs>,
    ExprPointer<PayloadRhs, IndirectionRhs, ptr_variant_rhs>, Token> {
    using type = BinaryExpr<Payload, Indirection, ptr_variant>;
};

template <
    typename Payload, typename Indirection, bool ptr_variant, typename PayloadChild, typename IndirectionChild,
    bool ptr_variant_child>
struct ResolveNodeTypeImpl<
    Payload, Indirection, ptr_variant, ExprPointer<PayloadChild, IndirectionChild, ptr_variant_child>> {
    using type = GroupingExpr<Payload, Indirection, ptr_variant>;
};

template <
    typename Payload, typename Indirection, bool ptr_variant, typename PayloadChild, typename IndirectionChild,
    bool ptr_variant_child>
struct ResolveNodeTypeImpl<
    Payload, Indirection, ptr_variant, ExprPointer<PayloadChild, IndirectionChild, ptr_variant_child>, Token> {
    using type = UnaryExpr<Payload, Indirection, ptr_variant>;
};

template <typename Payload, typename Indirection, bool ptr_variant>
struct ResolveNodeTypeImpl<Payload, Indirection, ptr_variant> {
    using type = NilExpr<Payload, Indirection, ptr_variant>;
};

template <typename Payload, typename Indirection, bool ptr_variant>
struct ResolveNodeTypeImpl<Payload, Indirection, ptr_variant, bool> {
    using type = BoolExpr<Payload, Indirection, ptr_variant>;
};

template <typename Payload, typename Indirection, bool ptr_variant>
struct ResolveNodeTypeImpl<Payload, Indirection, ptr_variant, double> {
    using type = NumberExpr<Payload, Indirection, ptr_variant>;
};

template <typename Payload, typename Indirection, bool ptr_variant>
struct ResolveNodeTypeImpl<Payload, Indirection, ptr_variant, const persistent_string<>*> {
    using type = StringExpr<Payload, Indirection, ptr_variant>;
};

template <
    typename Payload, typename Indirection, bool ptr_variant, typename StmtPayload, typename StmtIndirection,
    bool Stmt_ptr_variant>
struct ResolveNodeTypeImpl<
    Payload, Indirection, ptr_variant, marker<ExpressionStmt<StmtPayload, StmtIndirection, Stmt_ptr_variant>>> {
    using type = ExpressionStmt<Payload, Indirection, ptr_variant>;
};

template <
    typename Payload, typename Indirection, bool ptr_variant, typename StmtPayload, typename StmtIndirection,
    bool Stmt_ptr_variant>
struct ResolveNodeTypeImpl<
    Payload, Indirection, ptr_variant, marker<PrintStmt<StmtPayload, StmtIndirection, Stmt_ptr_variant>>> {
    using type = PrintStmt<Payload, Indirection, ptr_variant>;
};

template <typename Payload, typename Indirection, bool ptr_variant, typename... Args>
using ResolveNodeType = ResolveNodeTypeImpl<Payload, Indirection, ptr_variant, std::remove_cvref_t<Args>...>::type;

} // namespace loxxy
