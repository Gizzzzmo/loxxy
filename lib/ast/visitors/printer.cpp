module;

#include "loxxy/ast.hpp"
#include <iostream>

export module ast.printer;
import ast;
import utils.stupid_type_traits;
import utils.variant;

using utils::IndirectVisitor;

export namespace loxxy {

template <typename Payload, typename Indirection, bool ptr_variant, typename Resolver = void>
struct ASTPrinter : IndirectVisitor<ASTPrinter<Payload, Indirection, ptr_variant, Resolver>, Resolver, Indirection> {
    using Self = ASTPrinter<Payload, Indirection, ptr_variant, Resolver>;
    using Parent = IndirectVisitor<Self, Resolver, Indirection>;
    USING_FAMILY(Payload, Indirection, ptr_variant);

    using Parent::operator();

    template <typename... Args>
    ASTPrinter(std::ostream& stream, Args&&... args) : Parent(std::forward<Args>(args)...), stream(stream) {}

    void operator()(const BinaryExpr& node) {
        stream << node.op.getLexeme() << " (";
        visit(*this, node.lhs);
        stream << ") (";
        visit(*this, node.rhs);
        stream << ")";
    }

    void operator()(const GroupingExpr& node) {
        stream << "( ";
        visit(*this, node.expr);
        stream << " )";
    }

    void operator()(const StringExpr& node) { stream << "\"" << *node.string << "\""; }
    void operator()(const NumberExpr& node) { stream << node.x; }
    void operator()(const UnaryExpr& node) {
        stream << node.op.getLexeme() << " ( ";
        visit(*this, node.expr);
        stream << " ) ";
    }
    void operator()(const BoolExpr& node) { stream << (node.x ? "true" : "false"); }

    void operator()(const NilExpr& node) { stream << "nil"; }

    void operator()(const VarExpr& node) { stream << *node.identifier; }

    void operator()(const AssignExpr& node) {
        stream << "ASSIGN_EXPR ( " << *node.identifier;
        stream << " = ";
        visit(*this, node.expr);
        stream << " ) ";
    }

    void operator()(const CallExpr& node) {
        stream << "CALL_EXPR ( ";
        visit(*this, node.callee);
        stream << "( ";
        for (const auto& arg : node.arguments) {
            visit(*this, arg);
            if (&arg == &node.arguments.back())
                break;
            stream << " , ";
        }
        stream << " ) ) ";
    }

    void operator()(const PrintStmt& node) {
        stream << "PRINT ( ";
        visit(*this, node.expr);
        stream << " ) ";
    }

    void operator()(const ExpressionStmt& node) {
        stream << "EXPR_STMT ( ";
        visit(*this, node.expr);
        stream << " ) ";
    }

    void operator()(const FunDecl& node) {
        stream << "FUN_DECL " << *node.identifier << " ( ";

        for (auto& argument : node.args) {
            stream << *argument;
            if (&argument != &node.args.back())
                stream << ", ";
        }
        stream << " ) {\n";

        for (const auto& stmt : node.body) {
            visit(*this, stmt);
            stream << "\n";
        }
        stream << "}";
    }

    void operator()(const VarDecl& node) {
        stream << "VAR_DECL ( " << *node.identifier;
        if (node.expr) {
            stream << " = ( ";
            visit(*this, node.expr.value());
            stream << " ) ";
        }
        stream << " ) ";
    }

    void operator()(const BlockStmt& node) {
        stream << "BLOCK { \n";
        for (const auto& stmt : node.statements) {
            visit(*this, stmt);
            stream << "\n";
        }
        stream << "}";
    }

    void operator()(const IfStmt& node) {
        stream << "IF ( ";
        visit(*this, node.condition);
        stream << " ) THEN ";

        visit(*this, node.then_branch);
        if (node.else_branch.has_value()) {
            stream << " ELSE ";
            visit(*this, node.else_branch.value());
        }
    }

    void operator()(const WhileStmt& node) {
        stream << "WHILE ( ";
        visit(*this, node.condition);
        stream << " ) ";

        visit(*this, node.body);
    }

    void operator()(const ReturnStmt& node) {
        stream << "RETURN ( ";
        visit(*this, node.expr);
        stream << " ) ";
    }

private:
    std::ostream& stream;
};

template <typename Payload, typename Indirection, bool ptr_variant>
inline std::ostream& operator<<(std::ostream& ostream, const ExprPointer<Payload, Indirection, ptr_variant>& node) {
    visit(ASTPrinter<Payload, Indirection, ptr_variant, void>{ostream}, node);
    return ostream;
}

template <typename Payload, typename Indirection, bool ptr_variant>
inline std::ostream& operator<<(std::ostream& ostream, const StmtPointer<Payload, Indirection, ptr_variant>& node) {
    visit(ASTPrinter<Payload, Indirection, ptr_variant, void>{ostream}, node);
    return ostream;
}

template <typename Resolver>
struct resolver_stream {
    std::ostream& ostream;
    Resolver resolver;
};

template <>
struct resolver_stream<void> {
    std::ostream& stream;
};

template <typename T, typename Resolver>
auto operator<<(resolver_stream<Resolver>& ostream, const T& t) -> resolver_stream<Resolver> {
    ostream.ostream << t;
    return ostream;
}

template <typename Resolver, typename Payload, typename Indirection, bool ptr_variant>
    requires(!std::same_as<Resolver, void>)
auto operator<<(resolver_stream<Resolver>& ostream, const ExprPointer<Payload, Indirection, ptr_variant>& node)
    -> resolver_stream<Resolver>& {
    visit(ASTPrinter<Payload, Indirection, ptr_variant, Resolver>(ostream.ostream, ostream.resolver), node);
    return ostream;
}

} // namespace loxxy
