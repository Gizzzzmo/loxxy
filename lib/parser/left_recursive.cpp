module;

#include <string_view>
#include <exception>
#include <concepts>
#include <iostream>
#include <optional>
#include <ostream>
#include <sstream>
#include <utility>
#include "loxxy/ast.hpp"

export module parser:left_recursive;
import ast;
import ast.boxed_node_builder;
import ast.printer;
import utils.stupid_type_traits;
import utils.string_store;
import utils.variant;

using std::optional;
using std::same_as;


export namespace loxxy {

class ParseError : std::exception {};

template<
    typename istream = std::basic_stringstream<Token>, typename Payload = empty,
    typename Indirection = UniquePtrIndirection, bool ptr_variant = true,
    NodeBuilder<Payload, Indirection, ptr_variant> Builder = BoxedNodeBuilder<Payload>
>
class Parser {
    USING_FAMILY(Payload, Indirection, ptr_variant);

    public:
        template<typename... Args>
        Parser(istream& stream, Args&&... args) : stream(stream), node_builder(std::forward<Args>(args)...) {}
        TURoot parse() {
            TURoot root;
            while (!eof && !match(END_OF_FILE)) {
                try {
                    root.statements.push_back(statement());
                    std::cout << root.statements.back() << std::endl;
                } catch(const ParseError&) {
                    if (eof)
                        break;
                    stream.get();
                    continue;
                }
            }
            return root;
        }
    private:
        template<typename NodeType, typename... Args>
        auto make_node(Args&&... args) {
            return node_builder(mark<NodeType>, std::forward<Args>(args)...);
        }

        template<std::same_as<TokenType>... T>
        optional<Token> match(T... types) {
            if (!eof && ((stream.peek().getType() == types) || ...))
                return stream.get();

            return std::nullopt;
        }
        template<std::same_as<TokenType>... T>
        optional<Token> matchAllBut(T... types) {
            if (!eof && ((stream.peek().getType() != types) && ...))
                return stream.get();

            return std::nullopt;
        }
        Token expect(TokenType type) {
            optional<Token> t = match(type);
            if (t)
                return t.value();
            
            std::stringstream message;
            message << "Expected " << type << " token, but found " 
                << stream.peek().getType() << " token instead.";

            throw error(stream.peek(), message.str());
        }
        ParseError error(const Token& token, const std::string_view& message) {
            std::cerr << "[line " << token.getLine() << "] Error: " << message << std::endl;
            panic = true;
            return ParseError();
        }
        StmtPointer statement() {
            if (match(PRINT))
                return print_statement();

            return expression_statement();
        }
        StmtPointer print_statement() {
            ExprPointer expr = expression();
            expect(SEMICOLON);
            return make_node<PrintStmt>(std::move(expr));
        }
        StmtPointer expression_statement() {
            ExprPointer expr = expression();
            expect(SEMICOLON);
            return make_node<ExpressionStmt>(std::move(expr));
        }
        ExprPointer expression() {
            ExprPointer node = equality();

            optional<Token> op;
            if ( (op = match(COMMA)) ) {
                ExprPointer rhs = expression();
                node = make_node<BinaryExpr>(std::move(node), std::move(rhs), op.value());
            }

            return node;
        }
        ExprPointer equality() {
            ExprPointer node = comparison();
            optional<Token> op;
            while ( (op = match(BANG_EQUAL, EQUAL_EQUAL)) ) {
                ExprPointer rhs = comparison();
                node = make_node<BinaryExpr>(std::move(node), std::move(rhs), op.value());
            }

            return node;
        }
        ExprPointer comparison() {
            ExprPointer node = term();

            optional<Token> op;
            while ( (op = match(GREATER, GREATER_EQUAL, LESS, LESS_EQUAL)) ) {
                ExprPointer rhs = term();
                node = make_node<BinaryExpr>(std::move(node), std::move(rhs), op.value());
            }

            return node;
        }
        ExprPointer term() {
            ExprPointer node = factor();
            optional<Token> op;
            while( (op = match(MINUS, PLUS)) ) {
                ExprPointer rhs = factor();
                node = make_node<BinaryExpr>(std::move(node), std::move(rhs), op.value());
            }
            
            return node;
        }

        ExprPointer factor() {
            ExprPointer node = unary();

            optional<Token> op;
            while ( (op = match(SLASH, STAR)) ) {
                ExprPointer rhs = unary();
                node = make_node<BinaryExpr>(std::move(node), std::move(rhs), op.value());
            }

            return node;
        }

        ExprPointer unary() {
            optional<Token> op = match(BANG, MINUS);
            if (op)
                return make_node<UnaryExpr>(unary(), op.value());

            return primary();
        }

        ExprPointer primary() {
            optional<Token> token = match(
                FALSE, TRUE, NIL, NUMBER, STRING, LEFT_PAREN, END_OF_FILE
            );
            
            if (!token)
                throw error(stream.peek(), "Expected primary expression");
            
            switch(token->getType()) {
            default:
                std::unreachable();
            case END_OF_FILE:
                eof = true;
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
            case LEFT_PAREN:
                ExprPointer node = expression();
                expect(RIGHT_PAREN);
                return make_node<GroupingExpr>(std::move(node));
            }
        }
        void synchronize() {
            stream.get();
            optional<Token> t;

            while( (t = matchAllBut(END_OF_FILE)) ) {
                if (t->getType() == SEMICOLON)
                    return;

                switch (stream.peek().getType()) {
                case CLASS:
                case FUN:
                case VAR:
                case FOR:
                case IF:
                case WHILE:
                case PRINT:
                case RETURN:
                    return;
                }
            }
            eof = true;
        }

        istream& stream;
        Builder node_builder;
        bool eof = false;
        bool panic = false;
};

} // namespace loxxy