module;

#include "loxxy/ast.hpp"
#include <concepts>
#include <exception>
#include <iostream>
#include <optional>
#include <ostream>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

export module parser.rd;
import ast;
import ast.boxed_node_builder;
import ast.printer;
import utils.stupid_type_traits;
import utils.string_store;
import utils.stupid_type_traits;
import utils.variant;

using std::optional;
using std::same_as;
using utils::Adhoc;

export namespace loxxy {

class ParseError : std::exception {};

template <typename istream, typename Builder>
class Parser {
    class ScopeGuard {
    public:
        ScopeGuard(int& x) : x(x) {
            if (x >= 0)
                x++;
        }
        ~ScopeGuard() noexcept {
            if (x >= 0)
                x--;
        }

    private:
        int& x;
    };

    using Payload = typename Builder::Payload;
    using Indirection = typename Builder::Indirection;
    static constexpr bool ptr_variant = Builder::ptr_variant;
    USING_FAMILY(Payload, Indirection, ptr_variant);

    using Adhoc = Adhoc<Indirection>;

public:
    template <typename... Args>
    Parser(istream& stream, Args&&... args) : stream(stream), node_builder(std::forward<Args>(args)...) {}

    auto parseRepl() -> TURoot {
        scope_level = 0;
        TURoot root;
        while (!match(END_OF_FILE)) {
            auto maybe_decl = declaration();
            if (maybe_decl.has_value())
                root.statements.push_back(std::move(maybe_decl.value()));

            if (matchNewLine())
                break;
        }
        return root;
    }

    auto parse() -> TURoot {
        scope_level = -1;
        TURoot root;
        while (!match(END_OF_FILE)) {
            auto maybe_decl = declaration();
            if (maybe_decl.has_value())
                root.statements.push_back(std::move(maybe_decl.value()));
            if (matchNewLine())
                break;
        }
        return root;
    }

    void reset() { eof = std::nullopt; }

private:
    auto peekType() -> TokenType {
        if (eof)
            return END_OF_FILE;

        return stream.peek().getType();
    }

    auto getToken() -> Token {
        skipped_line_since_last_token = false;
        if (eof)
            return eof.value();

        if (peekType() == END_OF_FILE) {
            eof = stream.get();
            return eof.value();
        }

        return stream.get();
    }

    auto matchNewLine() -> bool {
        if (peekType() == NEW_LINE) {
            getToken();
            return true;
        }
        return false;
    }

    void skipNewLines() {
        while (peekType() == NEW_LINE) {
            getToken();
            skipped_line_since_last_token = true;
        }
    }

    template <typename NodeType, typename... Args>
    auto make_node(Args&&... args) {
        return node_builder(mark<NodeType>, std::forward<Args>(args)...);
    }

    template <std::same_as<TokenType>... T>
    auto check(T... types) -> bool {
        skipNewLines();
        return ((peekType() == types) || ...);
    }

    template <std::same_as<TokenType>... T>
    auto match(T... types) -> optional<Token> {
        if (suspend_matching)
            return std::nullopt;
        if (check(types...))
            return getToken();

        return std::nullopt;
    }

    template <std::same_as<TokenType>... T>
    auto matchAllBut(T... types) -> optional<Token> {
        if (suspend_matching)
            return std::nullopt;
        if (!check(types...))
            return getToken();

        return std::nullopt;
    }

    auto expect(TokenType type) -> Token {
        optional<Token> t = match(type);
        if (t.has_value())
            return t.value();

        std::stringstream message;
        message << "Expected " << type << " token, but found " << stream.peek().getType() << " token instead.";

        throw error(stream.peek(), message.str());
    }

    auto error(const Token& token, const std::string_view& message) -> ParseError {
        std::cerr << "[line " << token.getLine() << "] Error: " << message << std::endl;
        panic = true;
        return ParseError{};
    }

    auto declaration() -> optional<StmtPointer> {
        while (!eof) {
            try {
                if (match(VAR))
                    return var_declaration();

                return statement();
            } catch (ParseError& err) { synchronize(); }
        }

        return std::nullopt;
    }

    auto var_declaration() -> StmtPointer {
        Token identifier = expect(IDENTIFIER);

        std::optional<ExprPointer> initializer;
        if (match(EQUAL))
            initializer = expression();

        expect(SEMICOLON);
        return make_node<VarDecl>(&identifier.getLexeme(), std::move(initializer));
    }

    auto statement() -> StmtPointer {
        if (match(FOR))
            return for_statement();
        if (match(WHILE))
            return while_statement();
        if (match(IF))
            return if_statement();
        if (match(PRINT))
            return print_statement();
        if (match(LEFT_BRACE))
            return block();

        return expression_statement();
    }

    auto for_statement() -> StmtPointer {
        ScopeGuard guard(scope_level);
        expect(LEFT_PAREN);

        std::vector<StmtPointer> for_loop_block;
        for_loop_block.reserve(2);

        if (match(VAR))
            for_loop_block.push_back(var_declaration());
        else if (!match(SEMICOLON))
            for_loop_block.push_back(expression_statement());

        ExprPointer condition = check(SEMICOLON) ? make_node<BoolExpr>(true) : expression();
        expect(SEMICOLON);

        optional<StmtPointer> increment;
        if (!check(RIGHT_PAREN))
            increment = make_node<ExpressionStmt>(expression());
        expect(RIGHT_PAREN);

        StmtPointer loop_body = statement();
        if (increment.has_value()) {
            std::vector<StmtPointer> loop_body_stmts;
            loop_body_stmts.reserve(2);
            loop_body_stmts.push_back(std::move(loop_body));
            loop_body_stmts.push_back(std::move(increment.value()));
            loop_body = make_node<BlockStmt>(std::move(loop_body_stmts));
        }
        StmtPointer while_loop = make_node<WhileStmt>(std::move(condition), std::move(loop_body));

        if (for_loop_block.empty())
            return std::move(while_loop);

        for_loop_block.push_back(std::move(while_loop));
        return make_node<BlockStmt>(std::move(for_loop_block));
    }

    auto while_statement() -> StmtPointer {
        ScopeGuard guard(scope_level);
        expect(LEFT_PAREN);
        ExprPointer condition = expression();
        expect(RIGHT_PAREN);

        StmtPointer body = statement();
        return make_node<WhileStmt>(std::move(condition), std::move(body));
    }

    auto if_statement() -> StmtPointer {
        ExprPointer condition = [this]() -> ExprPointer {
            ScopeGuard guard(scope_level);
            expect(LEFT_PAREN);
            ExprPointer condition = expression();
            expect(RIGHT_PAREN);
            return condition;
        }();

        StmtPointer then_branch = statement();
        optional<StmtPointer> else_branch;

        if ((scope_level != 0 || stream.peek().getType() != NEW_LINE) && match(ELSE))
            else_branch = statement();

        return make_node<IfStmt>(std::move(condition), std::move(then_branch), std::move(else_branch));
    }

    auto print_statement() -> StmtPointer {
        ExprPointer expr = expression();
        expect(SEMICOLON);
        return make_node<PrintStmt>(std::move(expr));
    }

    auto expression_statement() -> StmtPointer {
        ExprPointer expr = expression();

        if (suspend_matching) {
            suspend_matching = false;
            return make_node<PrintStmt>(std::move(expr));
        }
        expect(SEMICOLON);
        return make_node<ExpressionStmt>(std::move(expr));
    }

    auto block() -> StmtPointer {
        ScopeGuard guard{scope_level};

        std::vector<StmtPointer> statements{};

        while (!check(RIGHT_BRACE, END_OF_FILE)) {
            optional<StmtPointer> decl = declaration();
            if (!decl.has_value())
                break;
            statements.push_back(std::move(decl.value()));
        }

        expect(RIGHT_BRACE);

        return make_node<BlockStmt>(std::move(statements));
    }

    auto expression() -> ExprPointer { return comma(); }

    auto comma() -> ExprPointer {
        ExprPointer node = assignment();

        optional<Token> op;
        if ((op = match(COMMA))) {
            ExprPointer rhs = assignment();
            node = make_node<BinaryExpr>(std::move(node), std::move(rhs), op.value());
        }

        return node;
    }

    auto assignment() -> ExprPointer {
        ExprPointer node = disjunction();

        optional<Token> op;
        if ((op = match(EQUAL))) {
            ExprPointer value = disjunction();

            auto getAssignTarget = Adhoc::make_visitor(
                [](const VarExpr& node) { return node.identifier; },
                []<typename T>(const T& node) -> const persistent_string<>* { return nullptr; }
            );

            const persistent_string<>* assign_target = utils::visit(getAssignTarget, node);
            if (assign_target != nullptr)
                return make_node<AssignExpr>(assign_target, std::move(value));

            error(op.value(), "Invalid assignment target");
        }
        return node;
    }

    auto disjunction() -> ExprPointer {
        ExprPointer node = conjunction();

        optional<Token> op;
        while ((op = match(OR))) {
            ExprPointer rhs = conjunction();
            node = make_node<BinaryExpr>(std::move(node), std::move(rhs), op.value());
        }

        return node;
    }

    auto conjunction() -> ExprPointer {
        ExprPointer node = equality();

        optional<Token> op;
        while ((op = match(AND))) {
            ExprPointer rhs = equality();
            node = make_node<BinaryExpr>(std::move(node), std::move(rhs), op.value());
        }

        return node;
    }

    auto equality() -> ExprPointer {
        ExprPointer node = comparison();

        optional<Token> op;
        while ((op = match(BANG_EQUAL, EQUAL_EQUAL))) {
            ExprPointer rhs = comparison();
            node = make_node<BinaryExpr>(std::move(node), std::move(rhs), op.value());
        }

        return node;
    }

    auto comparison() -> ExprPointer {
        ExprPointer node = term();

        optional<Token> op;
        while ((op = match(GREATER, GREATER_EQUAL, LESS, LESS_EQUAL))) {
            ExprPointer rhs = term();
            node = make_node<BinaryExpr>(std::move(node), std::move(rhs), op.value());
        }

        return node;
    }

    auto term() -> ExprPointer {
        ExprPointer node = factor();

        optional<Token> op;
        while ((op = match(MINUS, PLUS))) {
            ExprPointer rhs = factor();
            node = make_node<BinaryExpr>(std::move(node), std::move(rhs), op.value());
        }

        return node;
    }

    auto factor() -> ExprPointer {
        ExprPointer node = unary();

        optional<Token> op;
        while ((op = match(SLASH, STAR))) {
            ExprPointer rhs = unary();
            node = make_node<BinaryExpr>(std::move(node), std::move(rhs), op.value());
        }

        return node;
    }

    auto unary() -> ExprPointer {
        optional<Token> op = match(BANG, MINUS);
        if (op)
            return make_node<UnaryExpr>(unary(), op.value());

        return call();
    }

    auto call() -> ExprPointer {
        ExprPointer node = primary();

        while (true) {
            if (scope_level == 0 && stream.peek().getType() == NEW_LINE) {
                suspend_matching = true;
                break;
            }

            if (match(LEFT_PAREN)) {
                ScopeGuard guard(scope_level);
                node = finishCall(std::move(node));
            } else
                break;
        }

        return std::move(node);
    }

    auto finishCall(ExprPointer&& callee) -> ExprPointer {
        std::vector<ExprPointer> arguments;

        if (!check(RIGHT_PAREN)) {
            do {
                arguments.push_back(assignment());
            } while (match(COMMA));
        }

        expect(RIGHT_PAREN);

        return make_node<CallExpr>(std::move(callee), std::move(arguments));
    }

    auto primary() -> ExprPointer {
        optional<Token> token = match(FALSE, TRUE, NIL, NUMBER, STRING, LEFT_PAREN, END_OF_FILE, IDENTIFIER);

        if (!token)
            throw error(stream.peek(), "Expected primary expression");

        auto getExpr = [this, &token]() -> ExprPointer {
            switch (token->getType()) {
            default:
                std::unreachable();
            case END_OF_FILE:
                throw error(token.value(), "Unexpected end of file");
            case FALSE:
                return make_node<BoolExpr>(false);
            case TRUE:
                return make_node<BoolExpr>(true);
            case NIL:
                return make_node<NilExpr>();
            case NUMBER:
                return make_node<NumberExpr>(token->getLiteral().number);
            case STRING:
                return make_node<StringExpr>(token->getLiteral().string);
            case IDENTIFIER:
                return make_node<VarExpr>(&token->getLexeme());
            case LEFT_PAREN:
                ScopeGuard guard{scope_level};
                ExprPointer node = expression();
                expect(RIGHT_PAREN);
                return make_node<GroupingExpr>(std::move(node));
            }
        };

        ExprPointer expr = getExpr();

        return expr;
    }

    void synchronize() {
        getToken(); // consume erroneous token
        optional<Token> t;

        while ((t = matchAllBut(END_OF_FILE))) {
            if (t->getType() == SEMICOLON) // synchronize after every semicolon
                return;

            if (check(CLASS, FUN, VAR, FOR, IF, WHILE, PRINT, RETURN)) // synchronize before any of these keywords
                return;
        }
    }

    istream& stream;
    Builder node_builder;
    optional<Token> eof = std::nullopt;
    bool panic = false;
    int scope_level;
    bool suspend_matching = false;
    bool skipped_line_since_last_token;
};

template <typename Stream, typename Builder>
Parser(Stream, Builder) -> Parser<Stream, Builder>;

} // namespace loxxy
