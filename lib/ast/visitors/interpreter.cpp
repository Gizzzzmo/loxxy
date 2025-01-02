module;
#include <string_view>
#include <type_traits>
#include <exception>
#include <stdexcept>
#include <concepts>
#include <cstddef>
#include <optional>
#include <string>
#include <concepts>
#include <iostream>
#include "loxxy/ast.hpp"
#include <map>
//#include "tsl/htrie_map.h"

export module ast.interpreter;
import ast;
import utils.stupid_type_traits;
import utils.string_store;
import utils.variant;

using utils::IndirectVisitor;
using std::string;
using std::nullptr_t;
using std::same_as;


export namespace loxxy {


template<typename T, typename... U>
concept any_ref = (same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>> || ...) && std::is_reference_v<T>;

struct TypeError : std::runtime_error { using std::runtime_error::runtime_error; };
struct UndefinedVariable : std::runtime_error { using std::runtime_error::runtime_error; };

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
    utils::visit(ValuePrinter{ostream}, value);
    return ostream;
}


template<typename Payload, typename Indirection, bool ptr_variant, typename Resolver = void>
struct Interpreter : IndirectVisitor<
    Interpreter<Payload, Indirection, ptr_variant, Resolver>,
    Resolver,
    Indirection
> {
    USING_FAMILY(Payload, Indirection, ptr_variant);
    using Self = Interpreter<Payload, Indirection, ptr_variant, Resolver>;
    using Parent = IndirectVisitor<Self, Resolver, Indirection>;
    using Parent::operator();
    using Parent::Parent;

    void operator()(const ExpressionStmt& node) {
        utils::visit(*this, node.expr);
        return;
    }

    void operator()(const PrintStmt& node) {
        std::cout << utils::visit(*this, node.expr) << std::endl;
    }

    void operator()(const VarDecl& node) {
        Value init = nullptr;
        if (node.expr.has_value())
            init = utils::visit(*this, node.expr.value());
        
        variables[node.identifier] = std::move(init);
    }

    template<typename T>
    void operator()(const T&) {
    }

    Value operator()(const BinaryExpr& node) {
        auto lhs = utils::visit(*this, node.lhs);
        auto rhs = utils::visit(*this, node.rhs);
        switch (node.op.getType()) {
            case GREATER:
                return Value{utils::visit(Greater{}, lhs, rhs)};
            case GREATER_EQUAL:
                return Value{utils::visit(GreaterEq{}, lhs, rhs)};
            case LESS:
                return Value{utils::visit(Less{}, lhs, rhs)};
            case LESS_EQUAL:
                return Value{utils::visit(LessEq{}, lhs, rhs)};
            case EQUAL_EQUAL:
                return Value{utils::visit(Equals{}, lhs, rhs)};
            case BANG_EQUAL:
                return Value{!utils::visit(Equals{}, lhs, rhs)};
            case PLUS:
                return utils::visit(Plus{}, lhs, rhs);
            case MINUS:
                return Value{utils::visit(Minus{}, lhs, rhs)};
            case STAR:
                return Value{utils::visit(Times{}, lhs, rhs)};
            case SLASH:
                return Value{utils::visit(Divide{}, lhs, rhs)};
            default:
                throw std::runtime_error("malformed binary node");
        }
    }

    Value operator()(const GroupingExpr& node) {
        return utils::visit(*this, node.expr);
    }

    Value operator()(const UnaryExpr& node) {
        Value rhs = visit(*this, node.expr);
        switch (node.op.getType()) {
            case MINUS:
                return Value{utils::visit(Minus{}, rhs)};
            case BANG:
                return Value{utils::visit(Not{}, rhs)};
            default:
                throw std::runtime_error("malformed unary node");
        }
    }

    Value operator()(const NumberExpr& node) {
        return Value{node.x};
    }

    Value operator()(const StringExpr& node) {
        return Value{string(*node.string)};
    }

    Value operator()(const NilExpr& node) {
        return Value{nullptr};
    }

    Value operator()(const BoolExpr& node) {
        return Value{node.x};
    }

    Value operator()(const VarExpr& node) {
        if (variables.find(node.identifier) == variables.end()) {
            std::string error{};
            error.append(*node.identifier);
            error.append(" not defined");
            throw UndefinedVariable(error);
        }
        return variables[node.identifier];
    }

    Value operator()(const AssignExpr& node) {
        auto it = variables.find(node.identifier);
        if (it == variables.end()) {
            std::string error{};
            error.append(*node.identifier);
            error.append(" not defined");
            throw UndefinedVariable(error);
        }
        it->second = utils::visit(*this, node.expr);
        return it->second;
    }

    private:
        std::map<const persistent_string<>*, Value> variables; 
};

}
