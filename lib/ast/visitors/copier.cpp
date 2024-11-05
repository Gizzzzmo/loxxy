module;
#include <concepts>
#include <utility>
#include <memory>

export module ast.copier;
import ast;
import utils.stupid_type_traits;
import utils.variant;

using utils::IndirectVisitor;
using std::make_unique;

export namespace loxxy {

template<
    typename Payload,
    typename Indirection,
    bool ptr_variant,
    typename STNPtr,
    NodeBuilder<Payload, Indirection, ptr_variant> Builder,
    typename Resolver = void
>
struct ASTCopier
: public IndirectVisitor<
    ASTCopier<Payload, Indirection, ptr_variant, STNPtr, Builder, Resolver>,
    Resolver,
    Indirection
> {
    using Self = ASTCopier<Payload, Indirection, ptr_variant, STNPtr, Builder, Resolver>;
    using Parent = IndirectVisitor<Self, Resolver, Indirection>;
    using Parent::operator();

    template<NodeBuilder<Payload, Indirection, ptr_variant> Builder_, typename... Args>
    ASTCopier(Builder_&& builder, Args&&... args) : builder(std::forward<Builder_>(builder)),
        Parent(std::forward<Args>(args)...) {}

    STNPtr operator()(const BinaryExpr<Payload, Indirection, ptr_variant>& node) {
        auto copy_lhs = visit(*this, node.lhs);
        auto copy_rhs = visit(*this, node.rhs);
        
        return builder(
            std::move(copy_lhs),
            std::move(copy_rhs),
            node.op
        );
    }

    STNPtr operator()(const GroupingExpr<Payload, Indirection, ptr_variant>& node) {
        return builder(
            visit(*this, node.expr)
        );
    }

    STNPtr operator()(const UnaryExpr<Payload, Indirection, ptr_variant>& node) {
        return builder(
            visit(*this, node.expr),
            node.op
        );
    }

    STNPtr operator()(const NumberExpr<Payload, Indirection, ptr_variant>& node) {
        return builder(node.x);
    }

    STNPtr operator()(const StringExpr<Payload, Indirection, ptr_variant>& node) {
        return builder(node.string);
    }

    STNPtr operator()(const BoolExpr<Payload, Indirection, ptr_variant>& node) {
        return builder(node.x);
    }

    STNPtr operator()(const NilExpr<Payload, Indirection, ptr_variant>&) {
        return builder();
    }
    private:
        Builder builder;

};


} // namespace loxxy