module;
#include <cstddef>
#include <concepts>
#ifdef STD_VARIANT
#include <variant>
#else
#include <mpark/variant.hpp>
#endif
#include <memory>
#include <type_traits>
#include <functional>
#include <experimental/propagate_const>

export module utils.stupid_type_traits;


#ifdef STD_VARIANT
using std::variant;
#else
using mpark::variant;
#endif
using std::remove_const_t;
using std::conditional_t;
using std::is_const_v;
using std::remove_volatile_t;
using std::is_volatile_v;
using std::remove_cv_t;
using std::function;
using std::same_as;

template<typename T>
static constexpr bool is_cv_v = is_const_v<T> && is_volatile_v<T>;

export namespace utils {

template<typename T, typename I, typename E>
concept SubscriptableWith = requires(const T ct, remove_const_t<T> t, I i) {
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
    using type = std::conditional_t<is_const_v<T>,
        const std::experimental::propagate_const<PointerLike<remove_const_t<T>>>, 
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
    offset_pointer(offset_t offset) : offset(offset) {}
    operator offset_t() { return offset; }

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
    T& get(type<T> t) { return t; } 
};

struct WrappedRefIndirection { 
    template<typename T>
    using type = std::reference_wrapper<T>;
    template<typename T>
    T& get(type<T> t) { return t.get(); }
};

template<typename Variant, typename Fn>
using MapTypes = typename MapTypesImpl<Variant, Fn>::type;


template<typename Resolver>
struct ResolverHolder {
    template<typename... Args>
    ResolverHolder(Args&&... args) : resolver(std::forward<Args>(args)...) {}
    
    Resolver resolver;
};

template<>
struct ResolverHolder<void> {
    template<typename... Args>
    ResolverHolder(Args&&...) {}
};

} // namespace utils (exported)

namespace utils {

template<typename Visitor, typename Resolver, typename Indirection>
struct IndirectVisitorImpl {
    template<typename T>
    using Mapped = typename Indirection::template type<T>;

    template<typename T> requires(IsIndirection<Indirection, Resolver, T>)
    decltype(auto) operator()(Indirection::template type<T>& mapped) {
        decltype(auto) unmapped = unmap<T>(mapped);
        return (*static_cast<Visitor*>(this))(unmapped);
    }
    
    template<typename T> requires(IsIndirection<Indirection, Resolver, T> && !is_const_v<Mapped<T>>)
    decltype(auto) operator()(const Indirection::template type<T>& mapped) {
        decltype(auto) unmapped = unmap<T>(mapped);
        return (*static_cast<Visitor*>(this))(unmapped);
    }
    
    template<typename T> requires(IsIndirection<Indirection, Resolver, T> && !is_volatile_v<Mapped<T>>)
    decltype(auto) operator()(volatile Indirection::template type<T>& mapped)  {
        decltype(auto) unmapped = unmap<T>(mapped);
        return (*static_cast<Visitor*>(this))(unmapped);
    }
    
    template<typename T> requires(
        IsIndirection<Indirection, Resolver, T> &&
        !is_const_v<Mapped<T>> &&
        !is_volatile_v<Mapped<T>>
    )
    decltype(auto) operator()(const volatile Mapped<T>& mapped)  {
        decltype(auto) unmapped = unmap<T>(mapped);
        return (*static_cast<Visitor*>(this))(unmapped);
    }
    

    template<typename... Ts> requires(IsIndirection<Indirection, Resolver, variant<Ts...>>)
    decltype(auto) visit(Mapped<variant<Ts...>>& mapped) {
        decltype(auto) unmapped = unmap<variant<Ts...>>(mapped);
        return std::visit(*reinterpret_cast<Visitor*>(this), unmapped);
    }

    template<typename... Ts> requires(IsIndirection<Indirection, Resolver, const variant<Ts...>>)
    decltype(auto) visit(Mapped<const variant<Ts...>>& mapped) {
        decltype(auto) unmapped = unmap<const variant<Ts...>>(mapped);
        return std::visit(*reinterpret_cast<Visitor*>(this), unmapped);
    }

    template<typename... Ts> requires(IsIndirection<Indirection, Resolver, volatile variant<Ts...>>)
    decltype(auto) visit(Mapped<volatile variant<Ts...>>& mapped) {
        decltype(auto) unmapped = unmap<volatile variant<Ts...>>(mapped);
        return std::visit(*reinterpret_cast<Visitor*>(this), unmapped);
    }

    template<typename... Ts> requires(IsIndirection<Indirection, Resolver, const volatile variant<Ts...>>)
    decltype(auto) visit(Mapped<const volatile variant<Ts...>>& mapped) {
        decltype(auto) unmapped = unmap<const volatile variant<Ts...>>(mapped);
        return std::visit(*reinterpret_cast<Visitor*>(this), unmapped);
    }

    private:
        template<typename T> requires (same_as<Resolver, void>)
        decltype(auto) unmap(Mapped<T>& mapped) {
            return Indirection::template get<T>(mapped);
        }
        template<typename T> requires (!same_as<Resolver, void>)
        decltype(auto) unmap(Mapped<T>& mapped) {
            return Indirection::template get<T>(mapped, reinterpret_cast<ResolverHolder<Resolver>*>(this)->resolver);
        }

        template<typename T> requires (same_as<Resolver, void> && !is_const_v<Mapped<T>>)
        decltype(auto) unmap(const Mapped<T>& mapped) {
            return Indirection::template get<T>(mapped);
        }
        template<typename T> requires (!same_as<Resolver, void> && !is_const_v<Mapped<T>>)
        decltype(auto) unmap(const Mapped<T>& mapped) {
            return Indirection::template get<T>(mapped, reinterpret_cast<ResolverHolder<Resolver>*>(this)->resolver);
        }

        template<typename T> requires (same_as<Resolver, void> && !is_volatile_v<Mapped<T>>)
        decltype(auto) unmap(volatile Mapped<T>& mapped) {
            return Indirection::template get<T>(mapped);
        }
        template<typename T> requires (!same_as<Resolver, void> && !is_volatile_v<Mapped<T>>)
        decltype(auto) unmap(volatile Mapped<T>& mapped) {
            return Indirection::template get<T>(mapped, reinterpret_cast<ResolverHolder<Resolver>*>(this)->resolver);
        }

        template<typename T> requires (same_as<Resolver, void> && !is_const_v<Mapped<T>> && !is_volatile_v<Mapped<T>>)
        decltype(auto) unmap(const volatile Mapped<T>& mapped) {
            return Indirection::template get<T>(mapped);
        }
        template<typename T> requires (!same_as<Resolver, void> && !is_const_v<Mapped<T>> && !is_volatile_v<Mapped<T>>)
        decltype(auto) unmap(const volatile Mapped<T>& mapped) {
            return Indirection::template get<T>(mapped, reinterpret_cast<ResolverHolder<Resolver>*>(this)->resolver);
        }
};

} // namespace utils (unexported)

export namespace utils {

template<typename Visitor, typename Resolver, typename... Indirections>
struct IndirectVisitor : ResolverHolder<Resolver>, IndirectVisitorImpl<Visitor, Resolver, Indirections>... {
    using ResolverHolder<Resolver>::ResolverHolder;
    using IndirectVisitorImpl<Visitor, Resolver, Indirections>::operator()...;
};

template<typename Fn, typename Out, typename... Args>
concept Function = requires(Fn fn) {
    { static_cast<function<Out(Args...)>>(fn) } -> same_as<function<Out(Args...)>>;
};

} // namespace utils
