module;
#include <type_traits>
export module ast:type_traits;

import :nodes;
import :token;
import utils.stupid_type_traits;

using std::true_type;
using std::false_type;

export namespace loxxy {


template<typename T>
struct LiteralSTNImpl : false_type {};

template<typename Payload, typename Indirection, bool ptr_variant>
struct LiteralSTNImpl<NumberNode<Payload, Indirection, ptr_variant>> : true_type {};
template<typename Payload, typename Indirection, bool ptr_variant>
struct LiteralSTNImpl<StringNode<Payload, Indirection, ptr_variant>> : true_type {};
template<typename Payload, typename Indirection, bool ptr_variant>
struct LiteralSTNImpl<BoolNode<Payload, Indirection, ptr_variant>> : true_type {};
template<typename Payload, typename Indirection, bool ptr_variant>
struct LiteralSTNImpl<NilNode<Payload, Indirection, ptr_variant>> : true_type {};

template<typename T>
concept LiteralSTN = LiteralSTNImpl<T>::value;

template<typename T>
struct InnerSTNImpl : false_type {};

template<typename Payload, typename Indirection, bool ptr_variant>
struct InnerSTNImpl<BinaryNode<Payload, Indirection, ptr_variant>> : true_type {};
template<typename Payload, typename Indirection, bool ptr_variant>
struct InnerSTNImpl<UnaryNode<Payload, Indirection, ptr_variant>> : true_type {};
template<typename Payload, typename Indirection, bool ptr_variant>
struct InnerSTNImpl<GroupingNode<Payload, Indirection, ptr_variant>> : true_type {};

template<typename T>
concept InnerSTN = InnerSTNImpl<T>::value;

template<typename T>
concept ConcreteSTN = InnerSTN<T> || LiteralSTN<T>;



template<typename Payload, typename Indirection, bool ptr_variant, typename... Args>
struct ResolveNodeTypeImpl;

template<
    typename Payload, typename Indirection, bool ptr_variant,
    typename PayloadLhs, typename IndirectionLhs, bool ptr_variant_lhs,
    typename PayloadRhs, typename IndirectionRhs, bool ptr_variant_rhs
>
struct ResolveNodeTypeImpl<
    Payload, Indirection, ptr_variant,
    STNPointer<PayloadLhs, IndirectionLhs, ptr_variant_lhs>,
    STNPointer<PayloadRhs, IndirectionRhs, ptr_variant_rhs>,
    Token
> {
    using type = BinaryNode<Payload, Indirection, ptr_variant>;
};

template<
    typename Payload, typename Indirection, bool ptr_variant,
    typename PayloadChild, typename IndirectionChild, bool ptr_variant_child
>
struct ResolveNodeTypeImpl<
    Payload, Indirection, ptr_variant,
    STNPointer<PayloadChild, IndirectionChild, ptr_variant_child>
> {
    using type = GroupingNode<Payload, Indirection, ptr_variant>;
};

template<
    typename Payload, typename Indirection, bool ptr_variant,
    typename PayloadChild, typename IndirectionChild, bool ptr_variant_child
>
struct ResolveNodeTypeImpl<
    Payload, Indirection, ptr_variant,
    STNPointer<PayloadChild, IndirectionChild, ptr_variant_child>,
    Token
> {
    using type = UnaryNode<Payload, Indirection, ptr_variant>;
};

template<typename Payload, typename Indirection, bool ptr_variant>
struct ResolveNodeTypeImpl<Payload, Indirection, ptr_variant> {
    using type = NilNode<Payload, Indirection, ptr_variant>;
};

template<typename Payload, typename Indirection, bool ptr_variant>
struct ResolveNodeTypeImpl<Payload, Indirection, ptr_variant, bool> {
    using type = BoolNode<Payload, Indirection, ptr_variant>;
};

template<typename Payload, typename Indirection, bool ptr_variant>
struct ResolveNodeTypeImpl<Payload, Indirection, ptr_variant, double> {
    using type = NumberNode<Payload, Indirection, ptr_variant>;
};

template<typename Payload, typename Indirection, bool ptr_variant>
struct ResolveNodeTypeImpl<Payload, Indirection, ptr_variant, const persistent_string<>*> {
    using type = StringNode<Payload, Indirection, ptr_variant>;
};

template<typename Payload, typename Indirection, bool ptr_variant, typename... Args>
using ResolveNodeType = ResolveNodeTypeImpl<Payload, Indirection, ptr_variant, std::remove_cvref_t<Args>...>::type;



} // namespace loxxy