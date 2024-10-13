module;

#include <memory>
#include <type_traits>
#include <concepts>
#ifdef STD_VARIANT
#include <variant>
#else
#include <mpark/variant.hpp>
#endif
#include <string>

export module ast:nodes;

import :token;
import utils.string_store;
import utils.stupid_type_traits;

using utils::UniquePtrIndirection;
using utils::PointerIndirection;
using utils::SharedPtrIndirection;
using utils::PropConstIndirection;
using utils::MapTypes;
using std::same_as;
using std::true_type;
using std::false_type;
#ifdef STD_VARIANT
#define VISIT std::visit
using std::variant;
#else
#define VISIT mpark::visit
using mpark::variant;
#endif


export namespace loxxy {

struct empty {};


template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct BinaryNode;
template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct GroupingNode;
template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct UnaryNode;
template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct NumberNode;
template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct StringNode;
template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct BoolNode;
template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct NilNode;

template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
using SyntaxTreeNode = variant<
    BinaryNode<Payload, Indirection, ptr_variant>,
    UnaryNode<Payload, Indirection, ptr_variant>,
    GroupingNode<Payload, Indirection, ptr_variant>,
    NumberNode<Payload, Indirection, ptr_variant>,
    StringNode<Payload, Indirection, ptr_variant>,
    BoolNode<Payload, Indirection, ptr_variant>,
    NilNode<Payload, Indirection, ptr_variant>
>;

template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
class STNPointer {
    using Self = STNPointer<Payload, Indirection, ptr_variant>;
    public:
        using pointer_t = std::conditional_t<ptr_variant,
            MapTypes<SyntaxTreeNode<Payload, Indirection, ptr_variant>, Indirection>,
            typename Indirection::template type<SyntaxTreeNode<Payload, Indirection, ptr_variant>>
        >;
        template<typename... Args>
        STNPointer(Args... args) : pointer(std::forward<Args>(args)...) {}
        operator pointer_t&() { return pointer; }
        operator const pointer_t&() const { return pointer; }
        template<typename T>
        Self& operator=(T&& other) {
            pointer = other;
            return *this;
        }
        auto& get_visitable() requires(ptr_variant) { return pointer; }
        const auto& get_visitable() const requires(ptr_variant)  { return pointer; }
        
        template<typename... Args> requires(!ptr_variant)
        decltype(auto) get_visitable(Args&&... args) {
            return Indirection::template get<SyntaxTreeNode<Payload, Indirection, ptr_variant>>(
                pointer,
                std::forward<Args>(args)...
            );
        }

        template<typename... Args> requires(!ptr_variant)
        decltype(auto) get_visitable(Args&&... args) const {
            return Indirection::template get<SyntaxTreeNode<Payload, Indirection, ptr_variant>>(
                pointer,
                std::forward<Args>(args)...
            );
        }

    private:
        pointer_t pointer;
};

template<typename T>
struct IsVarImpl : std::false_type {};

template<typename... Ts>
struct IsVarImpl<variant<Ts...>> : std::true_type {};

template<typename T>
concept IsVar = IsVarImpl<std::remove_cvref_t<T>>::value;

template<typename Visitor, IsVar Variant>
decltype(auto) extract_visitable(Visitor&&, Variant&& arg) {
    return std::forward<Variant>(arg);
}
template<typename Visitor, typename Pointer>
decltype(auto) extract_visitable(Visitor&&, Pointer&& pointer) {
    return pointer.get_visitable();
}
template<typename Visitor, typename Pointer> requires(requires(Visitor v) {{ v.resolver };})
decltype(auto) extract_visitable(Visitor&& visitor, Pointer&& pointer) {
    return pointer.get_visitable(visitor.resolver);
}

template<typename Visitor, typename... Args>
decltype(auto) visit(Visitor&& visitor, Args&&... args) {
    return VISIT(
        std::forward<Visitor>(visitor),
        extract_visitable(
            std::forward<Visitor>(visitor),
            std::forward<Args>(args)
        )...
    ); 
}

template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct BinaryNode {
    Payload payload;
    STNPointer<Payload, Indirection, ptr_variant> lhs;
    STNPointer<Payload, Indirection, ptr_variant> rhs;
    Token op;
};

template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct GroupingNode {
    Payload payload;
    STNPointer<Payload, Indirection, ptr_variant> expr;
};

template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct UnaryNode {
    Payload payload;
    STNPointer<Payload, Indirection, ptr_variant> expr;
    Token op;
};

template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct NumberNode {
    Payload payload;
    double x;
};

template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct StringNode {
    Payload payload;
    const persistent_string<>* string;
};

template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct BoolNode {
    Payload payload;
    bool x;
};

template<typename Payload = empty, typename Indirection = UniquePtrIndirection, bool ptr_variant = true>
struct NilNode {
    Payload payload;
};


template<typename Payload = empty, bool ptr_variant = true>
using BoxedSTN = STNPointer<Payload, UniquePtrIndirection, ptr_variant>;
template<typename Payload = empty, bool ptr_variant = true>
using RCSTN = STNPointer<Payload, SharedPtrIndirection, ptr_variant>;



} // namespace loxxy
