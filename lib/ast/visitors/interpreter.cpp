module;
#include "loxxy/ast.hpp"
#include <chrono>
#include <concepts>
#include <cstddef>
#include <iostream>
#include <map>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
// #include "tsl/htrie_map.h"

export module ast.interpreter;
import ast;
import utils.stupid_type_traits;
import utils.string_store;
import utils.variant;

using std::nullptr_t;
using std::same_as;
using std::string;
using utils::IndirectVisitor;

export namespace loxxy {

template <typename T, typename... U>
concept any_ref = (same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>> || ...) && std::is_reference_v<T>;

struct TypeError : std::runtime_error {
    using std::runtime_error::runtime_error;
};
struct UndefinedVariable : std::runtime_error {
    using std::runtime_error::runtime_error;
};
struct NotCallable : std::runtime_error {
    using std::runtime_error::runtime_error;
};
struct WrongNumberOfArguments : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct Equals {
    auto operator()(const LoxCallable& lhs, const LoxCallable& rhs) -> bool {
        return lhs.function_body == rhs.function_body;
    }
    auto operator()(const BuiltinCallable* lhs, const BuiltinCallable* rhs) -> bool { return lhs == rhs; }
    auto operator()(nullptr_t, nullptr_t) -> bool { return true; }
    auto operator()(bool lhs, bool rhs) -> bool { return lhs == rhs; }
    auto operator()(double lhs, double rhs) -> bool { return lhs == rhs; }
    auto operator()(const string& lhs, const string& rhs) -> bool { return lhs == rhs; }
    template <typename T, typename U>
        requires(!same_as<T, U>)
    auto operator()(T&&, U&&) -> bool {
        return false;
    }
};

struct Greater {
    auto operator()(double x, double y) -> bool { return x > y; }
    auto operator()(auto&&, auto&&) -> bool { throw TypeError("bad operands for greater than"); }
};

struct GreaterEq {
    auto operator()(double x, double y) -> bool { return x >= y; }
    auto operator()(auto&&, auto&&) -> bool { throw TypeError("bad operands for greater equal"); }
};

struct Less {
    auto operator()(double x, double y) -> bool { return x < y; }
    auto operator()(auto&&, auto&&) -> bool { throw TypeError("bad operands for less than"); }
};

struct LessEq {
    auto operator()(double x, double y) -> bool { return x <= y; }
    auto operator()(auto&&, auto&&) -> bool { throw TypeError("bad operands for less equal"); }
};

struct Not {
    auto operator()(bool value) -> bool { return !value; }
    auto operator()(nullptr_t) -> bool { return true; }
    auto operator()(auto&&) -> bool { return false; }
};

auto truthy(const Value& v) -> bool { return !visit(Not{}, v); }

struct Plus {
    auto operator()(double x, double y) -> Value { return Value{x + y}; }
    auto operator()(any_ref<string> auto&& x, any_ref<string> auto&& y) -> Value { return Value{x + y}; }
    auto operator()(auto&&, auto&&) -> Value { throw TypeError("bad operands for addition"); }
};

struct Times {
    auto operator()(double x, double y) -> double { return x * y; }
    auto operator()(auto&&, auto&&) -> double { throw TypeError("bad operands for multiplication"); }
};

struct Divide {
    auto operator()(double x, double y) -> double { return x / y; }
    auto operator()(auto&&, auto&&) -> double { throw TypeError("bad operands for division"); }
};

struct Minus {
    auto operator()(double x, double y) -> double { return x - y; }
    auto operator()(double x) -> double { return -x; }
    auto operator()(auto&&, auto&&) -> double { throw TypeError("bad operands for subtraction"); }
    auto operator()(auto&&) -> double { throw TypeError("bad operand for negation"); }
};

struct ValuePrinter {
    std::ostream& ostream;
    void operator()(auto&& x) { ostream << x; }
    void operator()(bool x) { ostream << std::string_view(x ? "true" : "false"); }
    void operator()(nullptr_t) { ostream << std::string_view("nil"); }
    void operator()(const BuiltinCallable* x) {
        ostream << "<builtin fn: " << std::string_view(x->name) << "#" << x->arity << "@" << &x->fn << ">";
    }
    void operator()(const LoxCallable& x) { ostream << "<function>"; }
};

auto operator<<(std::ostream& ostream, const Value& value) -> std::ostream& {
    visit(ValuePrinter{ostream}, value);
    return ostream;
}

template <typename Payload, typename Indirection, bool ptr_variant, typename Resolver = void>
struct Interpreter : IndirectVisitor<Interpreter<Payload, Indirection, ptr_variant, Resolver>, Resolver, Indirection> {

    USING_FAMILY(Payload, Indirection, ptr_variant);
    using Self = Interpreter<Payload, Indirection, ptr_variant, Resolver>;
    using Parent = IndirectVisitor<Self, Resolver, Indirection>;
    using Parent::operator();
    using Parent::Parent;

    using Adhoc = utils::Adhoc<Indirection>;

    static auto _clock_builtin(std::span<loxxy::Value> vs) -> Value {
        const static auto t0 = std::chrono::high_resolution_clock::now();
        auto t = std::chrono::high_resolution_clock::now();
        return static_cast<double>((t - t0).count()) / 1000000000;
    }

    static constexpr BuiltinCallable clock_builtin{.fn = _clock_builtin, .name = "clock", .arity = 0};

    class Call {
        using Interpreter = Self;

    public:
        Call(const std::span<Value> args, Interpreter& interpreter) : args(args), interpreter(interpreter) {
            interpreter.return_value = std::nullopt;
        }

        auto operator()(const BuiltinCallable* fn) -> Value {
            if (args.size() != fn->arity) {
                std::stringstream error_msg;
                error_msg << "got " << args.size() << " arguments, for builtin function '" << fn->name
                          << "' with arity " << fn->arity;
                throw WrongNumberOfArguments(error_msg.str());
            }

            return fn->fn(args);
        }

        auto operator()(const LoxCallable& fn) -> Value {
            auto body = reinterpret_cast<const std::vector<StmtPointer>*>(fn.function_body);
            if (fn.arg_names.size() != args.size()) {
                std::stringstream error_msg;
                error_msg << "function expects " << fn.arg_names.size() << " arguments, but got " << args.size() << ".";
                throw WrongNumberOfArguments(error_msg.str());
            }

            interpreter.variables.emplace_back();
            for (size_t i = 0; i < args.size(); i++) {
                interpreter.variables.back()[fn.arg_names[i]] = args[i];
            }

            interpreter.return_value = std::nullopt;
            for (const StmtPointer& statement : *body) {
                visit(interpreter, statement);
                if (interpreter.return_value.has_value())
                    break;
            }
            interpreter.variables.pop_back();

            if (!interpreter.return_value.has_value())
                interpreter.return_value = nullptr;

            return interpreter.return_value.value();
        }

        template <typename T>
        auto operator()(const T& value) -> Value {
            throw NotCallable("bad callee");
        }

    private:
        const std::span<Value> args;
        Interpreter& interpreter;
    };

    Interpreter(const persistent_string<>* clock_id) { variables.front().emplace(clock_id, &clock_builtin); }

    void addBuiltin(const persistent_string<>* identifier, Value&& value) {
        variables.front().emplace(identifier, std::move(value));
    }

    void operator()(const ExpressionStmt& node) {
        visit(*this, node.expr);
        return;
    }

    void operator()(const PrintStmt& node) { std::cout << visit(*this, node.expr) << std::endl; }

    void operator()(const VarDecl& node) {
        Value init = nullptr;
        if (node.expr.has_value())
            init = visit(*this, node.expr.value());

        variables.back()[node.identifier] = std::move(init);
    }

    void operator()(const FunDecl& node) {
        LoxCallable fn{node.args, &node.body};
        variables.back()[node.identifier] = Value{fn};
    }

    void operator()(const BlockStmt& node) {
        variables.emplace_back();
        for (const auto& statement : node.statements) {
            visit(*this, statement);
        }
        variables.pop_back();
    }

    void operator()(const IfStmt& node) {
        if (truthy(visit(*this, node.condition)))
            visit(*this, node.then_branch);
        else if (node.else_branch.has_value())
            visit(*this, node.else_branch.value());
    }

    void operator()(const WhileStmt& node) {
        while (truthy(visit(*this, node.condition)))
            visit(*this, node.body);
    }

    void operator()(const ReturnStmt& node) { return_value = visit(*this, node.expr); }

    template <typename T>
    void operator()(const T&) {}

    auto operator()(const BinaryExpr& node) -> Value {
        auto lhs = visit(*this, node.lhs);
        if (node.op.getType() == AND && visit(Not{}, lhs))
            return Value{false};

        if (node.op.getType() == OR && truthy(lhs))
            return Value{true};

        auto rhs = visit(*this, node.rhs);
        switch (node.op.getType()) {
        case GREATER:
            return Value{visit(Greater{}, lhs, rhs)};
        case GREATER_EQUAL:
            return Value{visit(GreaterEq{}, lhs, rhs)};
        case LESS:
            return Value{visit(Less{}, lhs, rhs)};
        case LESS_EQUAL:
            return Value{visit(LessEq{}, lhs, rhs)};
        case EQUAL_EQUAL:
            return Value{visit(Equals{}, lhs, rhs)};
        case BANG_EQUAL:
            return Value{!visit(Equals{}, lhs, rhs)};
        case PLUS:
            return visit(Plus{}, lhs, rhs);
        case MINUS:
            return Value{visit(Minus{}, lhs, rhs)};
        case STAR:
            return Value{visit(Times{}, lhs, rhs)};
        case SLASH:
            return Value{visit(Divide{}, lhs, rhs)};
        case AND:
        case OR:
            return Value{truthy(rhs)};
        default:
            throw std::runtime_error("malformed binary node");
        }
    }

    auto operator()(const GroupingExpr& node) -> Value { return visit(*this, node.expr); }

    auto operator()(const UnaryExpr& node) -> Value {
        Value rhs = visit(*this, node.expr);
        switch (node.op.getType()) {
        case MINUS:
            return Value{visit(Minus{}, rhs)};
        case BANG:
            return Value{visit(Not{}, rhs)};
        default:
            throw std::runtime_error("malformed unary node");
        }
    }

    auto operator()(const NumberExpr& node) -> Value { return Value{node.x}; }

    auto operator()(const StringExpr& node) -> Value { return Value{string(*node.string)}; }

    auto operator()(const NilExpr& node) -> Value { return Value{nullptr}; }

    auto operator()(const BoolExpr& node) -> Value { return Value{node.x}; }

    auto operator()(const VarExpr& node) -> Value {
        Value* loc = getVarAddress(node.identifier);
        if (loc == nullptr) {
            std::string error{};
            error.append(*node.identifier);
            error.append(" not defined");
            throw UndefinedVariable(error);
        }
        return *loc;
    }

    auto operator()(const AssignExpr& node) -> Value {
        Value* loc = getVarAddress(node.identifier);

        if (loc == nullptr) {
            std::string error{};
            error.append(*node.identifier);
            error.append(" not defined");
            throw UndefinedVariable(error);
        }

        *loc = visit(*this, node.expr);
        return *loc;
    }

    auto operator()(const CallExpr& node) -> Value {
        std::vector<Value> args;
        args.reserve(node.arguments.size());
        for (const ExprPointer& arg : node.arguments) {
            args.push_back(visit(*this, arg));
        }

        Value callee = visit(*this, node.callee);
        Call call{args, *this};

        return visit(call, callee);
    }

private:
    auto getVarAddress(const persistent_string<>* identifier) -> Value* {
        for (auto& env : variables | std::ranges::views::reverse) {
            auto it = env.find(identifier);
            if (it == env.end())
                continue;

            return &it->second;
        }

        return nullptr;
    }

    std::vector<std::map<const persistent_string<>*, Value>> variables{1};
    std::optional<Value> return_value{};
};

} // namespace loxxy
