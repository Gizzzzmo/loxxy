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
struct LeafSTNImpl : false_type {};

template <typename Payload, typename Indirection, bool ptr_variant>
struct LeafSTNImpl<NumberExpr<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct LeafSTNImpl<StringExpr<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct LeafSTNImpl<BoolExpr<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct LeafSTNImpl<NilExpr<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct LeafSTNImpl<VarExpr<Payload, Indirection, ptr_variant>> : true_type {};

template <typename T>
concept LeafSTN = LeafSTNImpl<T>::value;

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
concept StatementSTN = StatementSTNImpl<T>::value;

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
concept ExpressionSTN = ExpressionSTNImpl<T>::value;

template <typename T>
concept ConcreteSTN = ExpressionSTN<T> || StatementSTN<T>;

template <typename T>
concept InnerSTN = ConcreteSTN<T> && !LeafSTN<T>;

template <typename T>
struct STNPointerImpl : false_type {};

template <typename Payload, typename Indirection, bool ptr_variant>
struct STNPointerImpl<ExprPointer<Payload, Indirection, ptr_variant>> : true_type {};
template <typename Payload, typename Indirection, bool ptr_variant>
struct STNPointerImpl<StmtPointer<Payload, Indirection, ptr_variant>> : true_type {};

template <typename T>
concept STNPointer = STNPointerImpl<T>::value;

} // namespace loxxy
