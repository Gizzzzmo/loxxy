module;
#include <type_traits>
#include <utility>
#include <concepts>
#include <memory>
export module ast:building;
import :nodes;
import :token;
import utils.string_store;
import utils.stupid_type_traits;

using std::same_as;
using std::make_unique;

export namespace loxxy {



template<typename T, typename Payload, typename Indirection, bool ptr_variant>
concept NodeBuilder = requires(
    T t,
    ExprPointer<Payload, Indirection, ptr_variant>&& node,
    Token token,
    double x,
    bool b,
    const persistent_string<>* str
) {
    { t(node, node, token) } -> same_as<ExprPointer<Payload, Indirection, ptr_variant>>;
    { t(node, token) } -> same_as<ExprPointer<Payload, Indirection, ptr_variant>>;
    { t(node) } -> same_as<ExprPointer<Payload, Indirection, ptr_variant>>;
    { t(str) } -> same_as<ExprPointer<Payload, Indirection, ptr_variant>>;
    { t(x) } -> same_as<ExprPointer<Payload, Indirection, ptr_variant>>;
    { t() } -> same_as<ExprPointer<Payload, Indirection, ptr_variant>>;
    { t(b) } -> same_as<ExprPointer<Payload, Indirection, ptr_variant>>;
};


template<typename T, typename Payload, typename Indirection, bool ptr_variant>
concept PayloadBuilder = requires(
    T t,
    ExprPointer<Payload, Indirection, ptr_variant>& node,
    Token& token,
    double& x,
    bool& b,
    const persistent_string<>* str
) {
    { t(node, node, token) } -> same_as<Payload>;
    { t(node, token) } -> same_as<Payload>;
    { t(node) } -> same_as<Payload>;
    { t(str) } -> same_as<Payload>;
    { t(x) } -> same_as<Payload>;
    { t() } -> same_as<Payload>;
    { t(b) } -> same_as<Payload>;
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
