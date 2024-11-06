module;

#include <iostream>

export module ast.printer;
import ast;
import utils.stupid_type_traits;
import utils.variant;

using utils::IndirectVisitor;

export namespace loxxy {

template<typename Payload, typename Indirection, bool ptr_variant, typename Resolver = void>
struct ASTPrinter : 
    IndirectVisitor<
        ASTPrinter<Payload, Indirection, ptr_variant, Resolver>,
        Resolver,
        Indirection
    >
{
    using Self = ASTPrinter<Payload, Indirection, ptr_variant, Resolver>;
    using Parent = IndirectVisitor<Self, Resolver, Indirection>;
    
    using Parent::operator();

    template<typename... Args>
    ASTPrinter(std::ostream& stream, Args&&... args) : Parent(std::forward<Args>(args)...),
        stream(stream) {}

    void operator()(const BinaryExpr<Payload, Indirection, ptr_variant>& node) {
        stream << node.op.getLexeme() << " (";
        visit(*this, node.lhs);
        stream << ") (";
        visit(*this, node.rhs);
        stream << ")";
    }

    void operator()(const GroupingExpr<Payload, Indirection, ptr_variant>& node) {
        stream << "(";
        visit(*this, node.expr);
        stream << ")";
    }

    void operator()(const StringExpr<Payload, Indirection, ptr_variant>& node) {
        stream << "\"" << *node.string << "\"";
    }
    void operator()(const NumberExpr<Payload, Indirection, ptr_variant>& node) {
        stream << node.x;
    }
    void operator()(const UnaryExpr<Payload, Indirection, ptr_variant>& node) {
        stream << node.op.getLexeme() << " (";
        visit(*this, node.expr);
        stream << ") ";
    }
    void operator()(const BoolExpr<Payload, Indirection, ptr_variant>& node) {
        stream << (node.x ? "true" : "false");
    }

    void operator()(const NilExpr<Payload, Indirection, ptr_variant>& node) {
        stream << "nil";
    }

    void operator()(const PrintStmt<Payload, Indirection, ptr_variant>& node) {
        stream << "PRINT ( ";
        visit(*this, node.expr);
        stream << " )";
    }

    void operator()(const ExpressionStmt<Payload, Indirection, ptr_variant>& node) {
        stream << "EXPR_STMT (";
        visit(*this, node.expr);
        stream << " )";
    }
    private:
        std::ostream& stream;
};

template<typename Payload, typename Indirection, bool ptr_variant>
inline std::ostream& operator<<(std::ostream& ostream, const ExprPointer<Payload, Indirection, ptr_variant>& node) {
    visit(ASTPrinter<Payload, Indirection, true, void>{ostream}, node);
    return ostream;
}

template<typename Payload, typename Indirection, bool ptr_variant>
inline std::ostream& operator<<(std::ostream& ostream, const StmtPointer<Payload, Indirection, ptr_variant>& node) {
    visit(ASTPrinter<Payload, Indirection, true, void>{ostream}, node);
    return ostream;
}

template<typename Resolver>
struct resolver_stream {
    std::ostream& ostream;
    Resolver resolver;
};

template<>
struct resolver_stream<void> {
    std::ostream& stream;
};

template<typename T, typename Resolver>
resolver_stream<Resolver> operator<<(resolver_stream<Resolver>& ostream, const T& t) {
    ostream.ostream << t;
    return ostream;
}

template<typename Resolver, typename Payload, typename Indirection, bool ptr_variant>
    requires(!std::same_as<Resolver, void>)
resolver_stream<Resolver>& operator<<(
    resolver_stream<Resolver>& ostream,
    const ExprPointer<Payload, Indirection, ptr_variant>& node
) {
    visit(
        ASTPrinter<Payload, Indirection, ptr_variant, Resolver>(
            ostream.ostream, ostream.resolver
        ),
        node
    );
    return ostream;
}

} // namespace loxxy