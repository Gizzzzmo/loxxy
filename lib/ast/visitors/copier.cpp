module;
#include "loxxy/ast.hpp"
#include <concepts>
#include <memory>
#include <utility>

export module ast.copier;
import ast;
import utils.stupid_type_traits;
import utils.variant;

using std::make_unique;
using utils::IndirectVisitor;

export namespace loxxy {

template <typename Payload, typename Indirection, bool ptr_variant, typename Builder, typename Resolver = void>
struct ASTCopier
    : public IndirectVisitor<ASTCopier<Payload, Indirection, ptr_variant, Builder, Resolver>, Resolver, Indirection> {
    using Target = Family<typename Builder::Payload, typename Builder::Indirection, Builder::ptr_variant>;

    using Self = ASTCopier<Payload, Indirection, ptr_variant, Builder, Resolver>;
    using Parent = IndirectVisitor<Self, Resolver, Indirection>;
    USING_FAMILY(Payload, Indirection, ptr_variant);

    using Parent::operator();

    template <NodeBuilder<Payload, Indirection, ptr_variant> Builder_, typename... Args>
    ASTCopier(Builder_&& builder, Args&&... args)
        : builder(std::forward<Builder_>(builder)), Parent(std::forward<Args>(args)...) {}

    auto operator()(const BinaryExpr& node) {
        auto copy_lhs = visit(*this, node.lhs);
        auto copy_rhs = visit(*this, node.rhs);

        return builder(mark<typename Target::BinaryExpr>, std::move(copy_lhs), std::move(copy_rhs), node.op);
    }

    auto operator()(const GroupingExpr& node) {
        return builder(mark<typename Target::GroupingExpr>, visit(*this, node.expr));
    }

    auto operator()(const UnaryExpr& node) {
        return builder(mark<typename Target::UnaryExpr>, visit(*this, node.expr), node.op);
    }

    auto operator()(const NumberExpr& node) { return builder(mark<typename Target::NumberExpr>, node.x); }

    auto operator()(const StringExpr& node) { return builder(mark<typename Target::StringExpr>, node.string); }

    auto operator()(const BoolExpr& node) { return builder(mark<typename Target::BoolExpr>, node.x); }

    auto operator()(const NilExpr&) { return builder(mark<typename Target::NilExpr>); }

    auto operator()(const VarExpr& node) { return builder(mark<typename Target::VarExpr>, node.identifier); }

    auto operator()(const AssignExpr& node) {
        return builder(mark<typename Target::AssignExpr>, node.identifier, node.expr);
    }

    auto operator()(const ExpressionStmt& node) { return builder(mark<typename Target::ExpressionStmt>, node.expr); }

    auto operator()(const PrintStmt& node) { return builder(mark<typename Target::PrintStmt>, node.expr); }

    auto operator()(const VarDecl& node) { return builder(mark<typename Target::VarDecl>, node.identifier, node.expr); }

private:
    Builder builder;
};

} // namespace loxxy
