module;
#include <type_traits>
#include <exception>
#include <stdexcept>
#include <concepts>
#include <cstddef>
#ifdef STD_VARIANT
#include <variant>
#else
#include <mpark/variant.hpp>
#endif
#include <optional>
#include <string>
#include <concepts>

export module ast.interpreter;
import ast;
import utils.stupid_type_traits;

using utils::IndirectVisitor;
using std::string;
using std::nullptr_t;
using std::same_as;
#ifdef STD_VARIANT
using std::variant;
#else
using mpark::variant;
#endif


export namespace loxxy {

using Value = variant<bool, double, string, nullptr_t>;

template<typename T, typename... U>
concept any_ref = (same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>> || ...) && std::is_reference_v<T>;

struct TypeError : std::runtime_error { using std::runtime_error::runtime_error; };

struct Equals {
    bool operator()(nullptr_t, nullptr_t) { return true; }
    bool operator()(bool lhs, bool rhs) { return lhs == rhs; }
    bool operator()(double lhs, double rhs) { return lhs == rhs; }
    bool operator()(const string& lhs, const string& rhs) { return lhs == rhs; }
    template<typename T, typename U> requires (!same_as<T, U>)
    bool operator()(T&&, U&&) { return false; }
};

struct Greater {
    bool operator()(double x, double y) { return x > y; }
    bool operator()(auto&&, auto&&) { throw TypeError("bad operands for greater than"); }
};

struct GreaterEq {
    bool operator()(double x, double y) { return x >= y; }
    bool operator()(auto&&, auto&&) { throw TypeError("bad operands for greater equal"); }
};

struct Less {
    bool operator()(double x, double y) { return x <= y; }
    bool operator()(auto&&, auto&&) { throw TypeError("bad operands for less than"); }
};

struct LessEq {
    bool operator()(double x, double y) { return x < y; }
    bool operator()(auto&&, auto&&) { throw TypeError("bad operands for less equal"); }
};

struct Not {
    bool operator()(bool value) { return !value; }
    bool operator()(nullptr_t) { return true; }
    bool operator()(auto&&) { return false; }
};


struct Plus {
    Value operator()(double x, double y) { return Value{x + y}; }
    Value operator()(any_ref<string> auto&& x, any_ref<string> auto&& y) { return Value{x + y}; }
    Value operator()(auto&&, auto&&) { throw TypeError("bad operands for addition"); }
};

struct Times {
    double operator()(double x, double y) { return x * y; }
    double operator()(auto&&, auto&&) { throw TypeError("bad operands for multiplication"); }
};

struct Divide {
    double operator()(double x, double y) { return x / y; }
    double operator()(auto&&, auto&&) { throw TypeError("bad operands for division"); }
};

struct Minus {
    double operator()(double x, double y) { return x - y; }
    double operator()(double x) { return -x; }
    double operator()(auto&&, auto&&) { throw TypeError("bad operands for subtraction"); }
    double operator()(auto&&) { throw TypeError("bad operand for negation"); }
};

struct ValuePrinter {
    std::ostream& ostream;
    void operator()(auto&& x) { ostream << x; }
    void operator()(bool x) { ostream << std::string_view(x ? "true" : "false"); }
    void operator()(nullptr_t) { ostream << std::string_view("nil"); }
};

std::ostream& operator<<(std::ostream& ostream, const Value& value) {
    //if (std::holds_alternative<double>(value))
    //    ostream << std::get<double>(value);
    //if (std::holds_alternative<string>(value))
    //    ostream << std::get<string>(value);
    //if (std::holds_alternative<bool>(value))
    //    ostream << (std::get<bool>(value) ? "true" : "false");
    //if (std::holds_alternative<nullptr_t>(value))
    //    ostream << "nil";

    loxxy::visit(ValuePrinter{ostream}, value);
    return ostream;
}


template<typename Payload, typename Indirection, bool ptr_variant, typename Resolver>
struct Interpreter : IndirectVisitor<
    Interpreter<Payload, Indirection, ptr_variant, Resolver>,
    Resolver,
    Indirection
> {
    using Self = Interpreter<Payload, Indirection, ptr_variant, Resolver>;
    using Parent = IndirectVisitor<Self, Resolver, Indirection>;
    using Parent::operator();
    using Parent::Parent;

    Value operator()(const BinaryNode<Payload, Indirection, ptr_variant>& node) {
        auto lhs = loxxy::visit(*this, node.lhs);
        auto rhs = loxxy::visit(*this, node.rhs);
        switch (node.op.getType()) {
            case GREATER:
                return Value{loxxy::visit(Greater{}, lhs, rhs)};
            case GREATER_EQUAL:
                return Value{loxxy::visit(GreaterEq{}, lhs, rhs)};
            case LESS:
                return Value{loxxy::visit(Less{}, lhs, rhs)};
            case LESS_EQUAL:
                return Value{loxxy::visit(LessEq{}, lhs, rhs)};
            case EQUAL_EQUAL:
                return Value{loxxy::visit(Equals{}, lhs, rhs)};
            case BANG_EQUAL:
                return Value{!loxxy::visit(Equals{}, lhs, rhs)};
            case PLUS:
                return loxxy::visit(Plus{}, lhs, rhs);
            case MINUS:
                return Value{loxxy::visit(Minus{}, lhs, rhs)};
            case STAR:
                return Value{loxxy::visit(Times{}, lhs, rhs)};
            case SLASH:
                return Value{loxxy::visit(Divide{}, lhs, rhs)};
            default:
                throw std::runtime_error("malformed binary node");
        }
    }

    Value operator()(const GroupingNode<Payload, Indirection, ptr_variant>& node) {
        return loxxy::visit(*this, node.expr);
    }

    Value operator()(const UnaryNode<Payload, Indirection, ptr_variant>& node) {
        Value rhs = visit(*this, node.expr);
        switch (node.op.getType()) {
            case MINUS:
                return Value{loxxy::visit(Minus{}, rhs)};
            case BANG:
                return Value{loxxy::visit(Not{}, rhs)};
            default:
                throw std::runtime_error("malformed unary node");
        }
    }

    Value operator()(const NumberNode<Payload, Indirection, ptr_variant>& node) {
        return Value{node.x};
    }

    Value operator()(const StringNode<Payload, Indirection, ptr_variant>& node) {
        return Value{string(*node.string)};
    }

    Value operator()(const NilNode<Payload, Indirection, ptr_variant>& node) {
        return Value{nullptr};
    }

    Value operator()(const BoolNode<Payload, Indirection, ptr_variant>& node) {
        return Value{node.x};
    }

};

}
