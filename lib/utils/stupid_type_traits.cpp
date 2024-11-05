module;
#include <concepts>
#include <type_traits>
#include <functional>
#include <memory>
#include <experimental/propagate_const>

export module utils.stupid_type_traits;


using std::function;
using std::same_as;

export namespace utils {

template<typename T, typename I, typename E>
concept SubscriptableWith = requires(const T ct, std::remove_const_t<T> t, I i) {
    { t[i] } -> same_as<E&>;
    { ct[i] } -> same_as<const E&>;
};

template<typename T, typename In>
concept IsSimpleTypeFunction = requires() {
    typename T::template type<In>;
};

template<typename T, typename... Ins>
concept IsTypeFunction = (IsSimpleTypeFunction<T, Ins> && ...);


template<typename T, typename In>
concept IsSimpleIndirection = IsSimpleTypeFunction<T, In> && requires(T::template type<In> mapped) {
    { T::template get<In>(mapped) } -> same_as<In&>;
};

template<typename T, typename Res, typename In>
concept IsSimpleResolvingIndirection = IsSimpleTypeFunction<T, In>
&& requires(T::template type<In> mapped, Res& res, const Res& c_res) {
    { T::template get<In>(mapped, res) } -> std::convertible_to<In&>;
    { T::template get<In>(mapped, c_res) } -> std::convertible_to<const In&>;
};

template<typename T, typename Res, typename... Ins>
concept IsIndirection = 
    (same_as<Res, void> && (IsSimpleIndirection<T, Ins> && ...)) ||
    (!same_as<Res, void> && (IsSimpleResolvingIndirection<T, Res, Ins> && ...));

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


template<typename Variant, typename Fn>
using MapTypes = typename MapTypesImpl<Variant, Fn>::type;

template<typename Fn, typename Out, typename... Args>
concept Function = requires(Fn fn) {
    { static_cast<function<Out(Args...)>>(fn) } -> same_as<function<Out(Args...)>>;
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
    template<typename T>
    static constexpr T& get(const T& t) {
        return const_cast<T&>(t);
    }
};


struct UniquePtrIndirection {
    template<typename T>
    using type = std::unique_ptr<T>;
    template<typename T>
    static constexpr T& get(const std::unique_ptr<T>& ptr) { return *ptr; }
};

template<typename PtrLikeIndirection>
struct PropConstIndirection {
    template<typename T>
    using PointerLike = typename PtrLikeIndirection::template type<T>;
    template<typename T>
    using type = std::conditional_t<std::is_const_v<T>,
        const std::experimental::propagate_const<PointerLike<std::remove_const_t<T>>>, 
        std::experimental::propagate_const<PointerLike<T>>
    >;
    template<typename T>
    static constexpr T& get(type<T>& ptr) {
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

template<typename T, typename offset_t = size_t>
struct offset_pointer {
    constexpr offset_pointer(offset_t offset) : offset(offset) {}
    constexpr operator offset_t() { return offset; }

    offset_t offset;
};

template<typename offset_t = size_t>
struct OffsetPointerIndirection {
    template<typename T>
    using type = offset_pointer<T, offset_t>;
    template<typename T, SubscriptableWith<type<T>, T> Res>
    static constexpr decltype(auto) get(type<T> ptr, Res& res) { return res[ptr]; }
};

struct ReferenceIndirection {
    template<typename T>
    using type = T&;
    template<typename T>
    static constexpr T& get(type<T> t) { return t; } 
};

struct WrappedRefIndirection { 
    template<typename T>
    using type = std::reference_wrapper<T>;
    template<typename T>
    static constexpr T& get(type<T> t) { return t.get(); }
};

} // namespace utils
