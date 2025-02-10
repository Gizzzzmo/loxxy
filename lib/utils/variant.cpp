module;
#include <iostream>
#ifdef STD_VARIANT
#define SOURCE std
#include <variant>
#else
#define SOURCE mpark
#include <mpark/variant.hpp>
#endif
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

export module utils.variant;

import utils.stupid_type_traits;

using std::conditional_t;
using std::is_const_v;
using std::is_volatile_v;
using std::remove_const_t;
using std::remove_cv_t;
using std::remove_volatile_t;
using std::same_as;

#if __has_attribute(always_inline) || defined(__GNUC__)
#define ALWAYS_INLINE __attribute__((__always_inline__)) inline
#elif defined(_MSC_VER)
#define ALWAYS_INLINE __forceinline
#else
#define ALWAYS_INLINE inline
#endif

struct VarWrapperMarker {};
struct IndirectVarWrapperMarker {};

export namespace utils {

#ifdef STD_VARIANT
using std::bad_variant_access;
using std::monostate;
using std::variant;
#else
using mpark::bad_variant_access;
using mpark::monostate;
using mpark::variant;
#endif

template <typename T>
struct IsVarImpl : std::false_type {};

template <typename... Ts>
struct IsVarImpl<variant<Ts...>> : std::true_type {};

template <typename T>
concept IsVar = IsVarImpl<std::remove_cvref_t<T>>::value;

template <typename Var, typename Indirection = void>
struct WrappedVar;

template <IsVar Var, typename Indirection>
    requires(!std::is_const_v<Var> && !std::is_volatile_v<Var> && !std::is_reference_v<Var>)
struct WrappedVar<Var, Indirection> : IndirectVarWrapperMarker {
    static constexpr size_t variant_size = SOURCE::variant_size_v<Var>;
    template <size_t i>
    using variant_alternative = SOURCE::variant_alternative_t<i, Var>;

    template <typename... Args>
    constexpr WrappedVar(Args&&... args) : wrapped(std::forward<Args>(args)...) {}

    template <typename Self, typename T>
    constexpr auto operator=(this Self&& self, T&& t) -> auto&& {
        Indirection::template get<Var>(std::forward(self.wrapped)) = std::forward<T>(t);
        return std::forward<Self>(self);
    }

    constexpr ALWAYS_INLINE auto&& get_visitable(this auto&& self) {
        return Indirection::template get<Var>(self.wrapped);
    }

    template <typename Self, typename Res>
    constexpr ALWAYS_INLINE auto&& get_visitable(this Self&& self, Res&& res) {
        return Indirection::template get<Var>(self.wrapped, std::forward<Res>(res));
    }

private:
    typename Indirection::template type<Var> wrapped;
};
template <IsVar Var>
    requires(!std::is_const_v<Var> && !std::is_volatile_v<Var> && !std::is_reference_v<Var>)
struct WrappedVar<Var, void> : VarWrapperMarker {
    static constexpr size_t variant_size = SOURCE::variant_size_v<Var>;
    template <size_t i>
    using variant_alternative = SOURCE::variant_alternative_t<i, Var>;

    template <typename... Args>
    constexpr WrappedVar(Args&&... args) : wrapped(std::forward<Args>(args)...) {}

    template <typename Self, typename... Args>
    constexpr Self operator=(this Self&& self, Args&&... args) {
        self.wrapped.operator=(std::forward<Args>(args)...);
        return std::forward<Self>(self);
    }

    constexpr ALWAYS_INLINE auto&& get_visitable(this auto&& self) { return self.wrapped; }

private:
    Var wrapped;
};

template <typename T>
concept ResolvingVisitor = requires(T t) {
    { t.resolver };
};

template <typename T>
concept IsWrappedVar = std::derived_from<std::remove_cvref_t<T>, VarWrapperMarker>;
template <typename T>
concept IsIndirectWrappedVar = std::derived_from<std::remove_cvref_t<T>, IndirectVarWrapperMarker>;

constexpr ALWAYS_INLINE decltype(auto) extract_visitable(auto&&, IsVar auto&& var) { return var; }

template <typename Visitor, IsWrappedVar Arg>
constexpr ALWAYS_INLINE decltype(auto) extract_visitable(Visitor&&, Arg&& wrapped_var) {
    return std::forward<Arg>(wrapped_var).get_visitable();
}

template <typename Visitor, IsIndirectWrappedVar Arg>
    requires(!ResolvingVisitor<Visitor>)
constexpr ALWAYS_INLINE decltype(auto) extract_visitable(Visitor&&, Arg&& wrapped_var) {
    return std::forward<Arg>(wrapped_var).get_visitable();
}

template <typename Visitor, IsIndirectWrappedVar Arg>
    requires(ResolvingVisitor<Visitor>)
constexpr ALWAYS_INLINE decltype(auto) extract_visitable(Visitor&& visitor, Arg&& wrapped_var) {
    return std::forward<Arg>(wrapped_var).get_visitable(visitor.resolver);
}

template <typename Visitor, typename... Args>
constexpr ALWAYS_INLINE decltype(auto) visit(Visitor&& visitor, Args&&... args) {
    return SOURCE::visit(
        std::forward<Visitor>(visitor), extract_visitable(std::forward<Visitor>(visitor), std::forward<Args>(args))...
    );
}

template <typename T, typename Arg>
    requires(IsVar<Arg> || IsWrappedVar<Arg>)
constexpr ALWAYS_INLINE bool holds_alternative(Arg&& arg) {
    return SOURCE::holds_alternative<T>(extract_visitable(0, std::forward<Arg>(arg)));
}

template <typename T, typename Arg>
    requires(IsVar<Arg> || IsWrappedVar<Arg>)
constexpr ALWAYS_INLINE decltype(auto) get(Arg&& arg) {
    return SOURCE::get<T>(extract_visitable(0, std::forward<Arg>(arg)));
}

template <typename T>
constexpr ALWAYS_INLINE auto get_if(auto* arg) {
    return SOURCE::get_if<T>(&extract_visitable(0, *arg));
}

template <typename T, IsWrappedVar Arg, typename Resolver>
constexpr ALWAYS_INLINE bool holds_alternative(Arg&& arg, Resolver&& resolver) {
    return SOURCE::holds_alternative<T>(std::forward<Arg>(arg).get_visitable(std::forward<Resolver>(resolver)));
}

template <typename T, IsWrappedVar Arg, typename Resolver>
constexpr ALWAYS_INLINE decltype(auto) get(Arg&& arg, Resolver&& resolver) {
    return SOURCE::get<T>(std::forward<Arg>(arg).get_visitable(std::forward<Resolver>(resolver)));
}

template <typename T, typename Resolver>
constexpr ALWAYS_INLINE auto get_if(auto* arg, Resolver&& resolver) {
    return SOURCE::get_if<T>(&arg->get_visitable(std::forward<Resolver>(resolver)));
}

template <typename T>
    requires(IsVar<T> || IsWrappedVar<T>)
struct variant_size;

template <IsVar T>
struct variant_size<T> : SOURCE::variant_size<T> {};

template <IsWrappedVar T>
struct variant_size<T> : std::integral_constant<size_t, T::variant_size> {};

template <typename T>
    requires(IsVar<T> || IsWrappedVar<T>)
constexpr size_t variant_size_v = variant_size<T>::value;

template <size_t i, typename T>
    requires(IsVar<T> || IsWrappedVar<T>)
struct variant_alternative;

template <size_t i, IsVar T>
struct variant_alternative<i, T> : SOURCE::variant_alternative<i, T> {};

template <size_t i, IsWrappedVar T>
struct variant_alternative<i, T> {
    using type = typename T::variant_alternative;
};

template <size_t i, typename T>
    requires(IsVar<T> || IsWrappedVar<T>)
using variant_alternative_t = typename variant_alternative<i, T>::type;

} // namespace utils

namespace utils {

template <typename Resolver>
struct ResolverHolder {
    template <typename... Args>
    ResolverHolder(Args&&... args) : resolver(std::forward<Args>(args)...) {}

    Resolver resolver;
};

template <>
struct ResolverHolder<void> {
    template <typename... Args>
    ResolverHolder(Args&&...) {}
};

template <typename Visitor, typename Resolver, typename Indirection>
struct IndirectVisitorImpl {
    template <typename T>
    using Mapped = typename Indirection::template type<T>;

    template <typename T>
        requires(IsIndirection<Indirection, Resolver, T>)
    decltype(auto) operator()(Indirection::template type<T>& mapped) {
        decltype(auto) unmapped = unmap<T>(mapped);
        return (*static_cast<Visitor*>(this))(unmapped);
    }

    template <typename T>
        requires(IsIndirection<Indirection, Resolver, T> && !is_const_v<Mapped<T>>)
    decltype(auto) operator()(const Indirection::template type<T>& mapped) {
        decltype(auto) unmapped = unmap<T>(mapped);
        return (*static_cast<Visitor*>(this))(unmapped);
    }

    template <typename T>
        requires(IsIndirection<Indirection, Resolver, T> && !is_volatile_v<Mapped<T>>)
    decltype(auto) operator()(volatile Indirection::template type<T>& mapped) {
        decltype(auto) unmapped = unmap<T>(mapped);
        return (*static_cast<Visitor*>(this))(unmapped);
    }

    template <typename T>
        requires(IsIndirection<Indirection, Resolver, T> && !is_const_v<Mapped<T>> && !is_volatile_v<Mapped<T>>)
    decltype(auto) operator()(const volatile Mapped<T>& mapped) {
        decltype(auto) unmapped = unmap<T>(mapped);
        return (*static_cast<Visitor*>(this))(unmapped);
    }

private:
    template <typename T>
        requires(same_as<Resolver, void>)
    decltype(auto) unmap(Mapped<T>& mapped) {
        return Indirection::template get<T>(mapped);
    }

    template <typename T>
        requires(!same_as<Resolver, void>)
    decltype(auto) unmap(Mapped<T>& mapped) {
        return Indirection::template get<T>(mapped, reinterpret_cast<ResolverHolder<Resolver>*>(this)->resolver);
    }

    template <typename T>
        requires(same_as<Resolver, void> && !is_const_v<Mapped<T>>)
    decltype(auto) unmap(const Mapped<T>& mapped) {
        return Indirection::template get<T>(mapped);
    }

    template <typename T>
        requires(!same_as<Resolver, void> && !is_const_v<Mapped<T>>)
    decltype(auto) unmap(const Mapped<T>& mapped) {
        return Indirection::template get<T>(mapped, reinterpret_cast<ResolverHolder<Resolver>*>(this)->resolver);
    }

    template <typename T>
        requires(same_as<Resolver, void> && !is_volatile_v<Mapped<T>>)
    decltype(auto) unmap(volatile Mapped<T>& mapped) {
        return Indirection::template get<T>(mapped);
    }

    template <typename T>
        requires(!same_as<Resolver, void> && !is_volatile_v<Mapped<T>>)
    decltype(auto) unmap(volatile Mapped<T>& mapped) {
        return Indirection::template get<T>(mapped, reinterpret_cast<ResolverHolder<Resolver>*>(this)->resolver);
    }

    template <typename T>
        requires(same_as<Resolver, void> && !is_const_v<Mapped<T>> && !is_volatile_v<Mapped<T>>)
    decltype(auto) unmap(const volatile Mapped<T>& mapped) {
        return Indirection::template get<T>(mapped);
    }

    template <typename T>
        requires(!same_as<Resolver, void> && !is_const_v<Mapped<T>> && !is_volatile_v<Mapped<T>>)
    decltype(auto) unmap(const volatile Mapped<T>& mapped) {
        return Indirection::template get<T>(mapped, reinterpret_cast<ResolverHolder<Resolver>*>(this)->resolver);
    }
};

} // namespace utils

export namespace utils {

template <typename Visitor, typename Resolver, typename... Indirections>
struct IndirectVisitor : ResolverHolder<Resolver>, IndirectVisitorImpl<Visitor, Resolver, Indirections>... {
    using ResolverHolder<Resolver>::ResolverHolder;
    using IndirectVisitorImpl<Visitor, Resolver, Indirections>::operator()...;
};

template <typename Resolver, typename... Indirections>
struct Adhoc : ResolverHolder<Resolver> {
    template <typename... Args>
    Adhoc(Args&&... args) : ResolverHolder<Resolver>(std::forward<Args>(args)...) {}
    template <typename... Fns>
    struct Vis : Fns..., IndirectVisitor<Vis<Fns...>, Resolver, Indirections...> {
        using Parent = IndirectVisitor<Vis<Fns...>, Resolver, Indirections...>;
        using Fns::operator()...;
        using Parent::operator();

        // Visitor(Resolver resolver) requires(!same_as<Resolver, void>) :
        // Parent(std::forward<Resolver>(resolver)) {}
        template <any_ref<Resolver> Res, typename... Args>
            requires(!std::same_as<Resolver, void>)
        Vis(Res&& resolver, Args&&... args) : Fns(std::forward<Args>(args))..., Parent(std::forward<Res>(resolver)) {}

        template <typename... Args>
            requires(std::same_as<Resolver, void>)
        Vis(Args&&... args) : Fns(std::forward<Args>(args))... {}
    };

    template <typename... Fns>
    auto make_visitor(Fns&&... fns) {
        if constexpr (std::same_as<Resolver, void>)
            return Vis<Fns...>{std::forward<Fns>(fns)...};
        else
            return Vis<Fns...>{this->resolver, std::forward<Fns>(fns)...};
    }
};

} // namespace utils
