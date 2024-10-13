
#include <algorithm>
#include <memory>
#include <string>
#include <concepts>
#include <type_traits>
#include <experimental/propagate_const>
#ifdef STD_VARIANT
#include <variant>
#else
#include <mpark/variant.hpp>
#endif


#ifdef STD_VARIANT
using std::variant;
using std::visit;
#else
using mpark::variant;
using mpark::visit;
#endif
using std::experimental::propagate_const;
using std::same_as;

struct empty{};

#if 1
#if 1
template<typename Payload = empty>
struct Binary;
template<typename Payload = empty>
struct Unary;
template<typename Payload = empty>
struct Leaf;


template<typename Payload = empty>
using Node = variant<Binary<Payload>, Unary<Payload>, Leaf<Payload>>;


//template<typename Payload = empty>
//using BoxedNode = MapTypes<Node<Payload>, PropConstIndirection<UniquePtrIndirection>>;
template<typename Payload = empty>
using BoxedNode = variant<
    std::unique_ptr<Binary<Payload>>,
    std::unique_ptr<Unary<Payload>>,
    std::unique_ptr<Leaf<Payload>>
>;


template<typename Payload>
struct Binary {
    BoxedNode<Payload> lhs;
    BoxedNode<Payload> rhs;
    int depth;
    Payload payload;
};

template<typename Payload>
struct Unary {
    BoxedNode<Payload> child;
    Payload payload;
};

template<typename Payload>
struct Leaf {
    std::string x;
    Payload payload;
};

auto test_make_node() {
    BoxedNode<> node = std::make_unique<Binary<>>(
        std::make_unique<Leaf<>>("x"),
        std::make_unique<Binary<>>(
            std::make_unique<Leaf<>>("y"),
            std::make_unique<Leaf<>>("z")
        )
    );
    return node;
}
#else

struct Binary;
struct Unary;
struct Leaf;

using Node = variant<Binary, Unary, Leaf>;


using BoxedNode = variant<
    propagate_const<std::unique_ptr<Binary>>,
    propagate_const<std::unique_ptr<Unary>>,
    propagate_const<std::unique_ptr<Leaf>>
>;


struct Binary {
    BoxedNode lhs;
    BoxedNode rhs;
    int depth;
};

struct Unary {
    BoxedNode child;
};

struct Leaf {
    std::string x;
};


auto test_make_node() {
    BoxedNode node = std::make_unique<Binary>(
        std::make_unique<Leaf>("x"),
        std::make_unique<Binary>(
            std::make_unique<Leaf>("y"),
            std::make_unique<Leaf>("z")
        )
    );
    return node;
}
#endif
#else


template<typename T, typename In>
concept SimpleTypeFunction = requires() {
    typename T::template type<In>;
};

template<typename T, typename... Ins>
concept IsTypeFunction = (SimpleTypeFunction<T, Ins> && ...);


template<typename T, typename In>
concept IsIndirection = SimpleTypeFunction<T, In> && requires(T::template type<In> mapped) {
    { T::template get<In>(mapped) } -> std::same_as<In&>;
};

template<typename T, typename... Ins>
concept IsIndirection = (IsIndirection<T, Ins> && ...);


template<typename Container, typename... Fn>
struct MapTypesImpl;

template<template<typename...> class ContainerTempl, typename... Ts, IsTypeFunction<Ts...> Fn>
struct MapTypesImpl<ContainerTempl<Ts...>, Fn> {
    using type = ContainerTempl<typename Fn::template type<Ts>...>;
};

template<template<typename...> class ContainerTempl, typename... Ts, IsTypeFunction<Ts...> Fn>
struct MapTypesImpl<const ContainerTempl<Ts...>, Fn> {
    using type = ContainerTempl<typename Fn::template type<const Ts>...>;
};

template<template<typename...> class ContainerTempl, typename... Ts, IsTypeFunction<Ts...> Fn>
struct MapTypesImpl<volatile ContainerTempl<Ts...>, Fn> {
    using type = ContainerTempl<typename Fn::template type<volatile Ts>...>;
};

template<template<typename...> class ContainerTempl, typename... Ts, IsTypeFunction<Ts...> Fn>
struct MapTypesImpl<const volatile ContainerTempl<Ts...>, Fn> {
    using type = ContainerTempl<typename Fn::template type<const volatile Ts>...>;
};

template<typename... Fn>
struct ComposedIndirection;

template<>
struct ComposedIndirection<> {
    template<typename T>
    using type = T;
    template<typename T>
    static constexpr T get(T&& t) { return std::forward<T>(t); }
};

template<typename Fn1, typename... Fns>
struct ComposedIndirection<Fn1, Fns...> {
    using Tail = ComposedIndirection<Fns...>;
    template<typename T>
    using type = typename Fn1::template type<
        typename ComposedIndirection<Fns...>::template type<T>
    >;

    template<typename T>
    static constexpr T get(const type<T>& mapped) {
        return Tail::template get<T>(
            Fn1::template get<typename Tail::template type<T>>()
        );
    }
};

using IdentityIndirection = ComposedIndirection<>;

struct ConstIndirection {
    template<typename T>
    using type = const T;
};


struct UniquePtrIndirection {
    template<typename T>
    using type = std::unique_ptr<T>;
    template<typename T>
    static constexpr T& get(const std::unique_ptr<T>& ptr) { return *ptr; }
};

template<typename PtrlikeFunctor>
struct PropConstIndirection {
    template<typename T>
    using Pointerlike = typename PtrlikeFunctor::template type<T>;
    template<typename T>
    using type = std::experimental::propagate_const<Pointerlike<T>>;
    template<typename T>
    static constexpr T& get(std::experimental::propagate_const<Pointerlike<T>>& ptr) {
        return *ptr;
    }
    template<typename T>
    static constexpr const T& get(const std::experimental::propagate_const<Pointerlike<T>>& ptr) {
        return *ptr;
    }
};

struct PointerIndirection {
    template<typename T>
    using type = T*;
    template<typename T>
    static constexpr T& get(T* ptr) { return *ptr; }
};

struct SharedPtrIndirection {
    template<typename T>
    using type = std::shared_ptr<T>;
    template<typename T>
    static constexpr T& get(const std::shared_ptr<T> ptr) { return *ptr; }
};

template<typename Variant, typename Fn>
using MapTypes = typename MapTypesImpl<Variant, Fn>::type;

template<typename Payload = empty>
struct BinaryNode;
template<typename Payload = empty>
struct GroupingNode;
template<typename Payload = empty>
struct UnaryNode;
template<typename Payload = empty>
struct NumberNode;
template<typename Payload = empty>
struct StringNode;
template<typename Payload = empty>
struct BoolNode;
template<typename Payload = empty>
struct NilNode;

template<typename T, typename Payload = empty>
concept LiteralNode = same_as<T, NumberNode<Payload>>
    || same_as<T, StringNode<Payload>>
    || same_as<T, BoolNode<Payload>>
    || same_as<T, NilNode<Payload>>;

template<typename Payload = empty>
using SyntaxTreeNode = variant<
    BinaryNode<Payload>,
    UnaryNode<Payload>,
    GroupingNode<Payload>,
    NumberNode<Payload>,
    StringNode<Payload>,
    BoolNode<Payload>,
    NilNode<Payload>
>;

// variant of unique pointers to some SyntaxTreeNode
template<typename Payload = empty>
using BoxedSTN = MapTypes<SyntaxTreeNode<Payload>, PropConstIndirection<UniquePtrIndirection>>;
// variant of shared pointers (RC = reference counted) to some SyntaxTreeNode
template<typename Payload = empty>
using RCSTN = MapTypes<SyntaxTreeNode<Payload>, PropConstIndirection<SharedPtrIndirection>>;
// variant of pointers to some SyntaxTreeNode
template<typename Payload = empty>
using STNPointer = MapTypes<SyntaxTreeNode<Payload>, PropConstIndirection<PointerIndirection>>;

template<typename T, typename Payload = empty>
concept STN = same_as<T, SyntaxTreeNode<Payload>>
    || same_as<T, BoxedSTN<Payload>>
    || same_as<T, RCSTN<Payload>>
    || same_as<T, STNPointer<Payload>>;

template<typename Payload>
struct BinaryNode {
    BoxedSTN<Payload> lhs;
    BoxedSTN<Payload> rhs;
    Payload payload;
};

template<typename Payload>
struct GroupingNode {
    BoxedSTN<Payload> expr;
    Payload payload;
};

template<typename Payload>
struct UnaryNode {
    BoxedSTN<Payload> expr;
    Payload payload;
};

template<typename Payload>
struct NumberNode {
    double x;
    Payload payload;
};

template<typename Payload>
struct StringNode {
    std::string str;
    Payload payload;
};

template<typename Payload>
struct BoolNode {
    bool x;
    Payload payload;
};

template<typename Payload>
struct NilNode {
    Payload payload;
};


auto test() {
    BoxedSTN<> node = std::make_unique<GroupingNode<>>(
        std::make_unique<NilNode<>>()
    );
    return node;
}
#endif


//int main() {
//    Leaf<int> leaf{"leaf", 10};
//
//
//    BoxedVar<int> node = std::make_unique<Inner<int>>(
//        std::make_unique<Leaf<int>>("x", 0),
//        std::make_unique<Inner<int>>(
//            std::make_unique<Leaf<int>>("y", 1),
//            std::make_unique<Leaf<int>>("z", 1),
//            1
//        ),
//        2
//    );
//
//    BoxedVar<> node1 = std::make_unique<Inner<>>(
//        std::make_unique<Leaf<>>("x"),
//        std::make_unique<Inner<>>(
//            std::make_unique<Leaf<>>("y"),
//            std::make_unique<Leaf<>>("z")
//        )
//    );
//    
//
//    return 0;
//}