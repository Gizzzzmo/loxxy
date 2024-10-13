module;

#include <iostream>

export module ast.printer;
import ast;
import utils.stupid_type_traits;

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

    void operator()(const BinaryNode<Payload, Indirection, ptr_variant>& node) {
        stream << node.op.getLexeme() << " (";
        visit(*this, node.lhs);
        stream << ") (";
        visit(*this, node.rhs);
        stream << ")";
    }

    void operator()(const GroupingNode<Payload, Indirection, ptr_variant>& node) {
        stream << "(";
        visit(*this, node.expr);
        stream << ")";
    }

    void operator()(const StringNode<Payload, Indirection, ptr_variant>& node) {
        stream << "\"" << *node.string << "\"";
    }
    void operator()(const NumberNode<Payload, Indirection, ptr_variant>& node) {
        stream << node.x;
    }
    void operator()(const UnaryNode<Payload, Indirection, ptr_variant>& node) {
        stream << node.op.getLexeme() << " (";
        visit(*this, node.expr);
        stream << ") ";
    }
    void operator()(const BoolNode<Payload, Indirection, ptr_variant>& node) {
        stream << (node.x ? "true" : "false");
    }

    void operator()(const NilNode<Payload, Indirection, ptr_variant>& node) {
        stream << "nil";
    }
    private:
        std::ostream& stream;
};

template<typename Payload, typename Indirection, bool ptr_variant>
inline std::ostream& operator<<(std::ostream& ostream, const STNPointer<Payload, Indirection, ptr_variant>& node) {
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
    const STNPointer<Payload, Indirection, ptr_variant>& node
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