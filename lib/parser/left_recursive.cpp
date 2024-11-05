module;

#include <string_view>
#include <exception>
#include <concepts>
#include <iostream>
#include <optional>
#include <ostream>
#include <sstream>
#include <utility>

export module parser:left_recursive;
import ast;
import ast.boxed_node_builder;
import utils.stupid_type_traits;
import utils.string_store;

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
    using Node = ExprPointer<Payload, Indirection, ptr_variant>;

    public:
        template<typename... Args>
        Parser(istream& stream, Args&&... args) : stream(stream), node_builder(std::forward<Args>(args)...) {}
        void parse() {
            
            while (!eof) {
                try {
                    Node root = expression();
                    std::cout << &root << std::endl;
                    //std::cout << payload_of(root).hash << std::endl;
                } catch(const ParseError&) {
                    if (eof)
                        return;
                    stream.get();
                    continue;
                }
            }
        }
    private:
        template<typename... Args>
        Node make_node(Args&&... args) {
            return node_builder(std::forward<Args>(args)...);
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
        Node expression() {
            Node node = equality();

            optional<Token> op;
            if ( (op = match(COMMA)) ) {
                Node rhs = expression();
                node = make_node(std::move(node), std::move(rhs), op.value());
            }

            return node;
        }
        Node equality() {
            Node node = comparison();
            optional<Token> op;
            while ( (op = match(BANG_EQUAL, EQUAL_EQUAL)) ) {
                Node rhs = comparison();
                node = make_node(std::move(node), std::move(rhs), op.value());
            }

            return node;
        }
        Node comparison() {
            Node node = term();

            optional<Token> op;
            while ( (op = match(GREATER, GREATER_EQUAL, LESS, LESS_EQUAL)) ) {
                Node rhs = term();
                node = make_node(std::move(node), std::move(rhs), op.value());
            }

            return node;
        }
        Node term() {
            Node node = factor();
            optional<Token> op;
            while( (op = match(MINUS, PLUS)) ) {
                Node rhs = factor();
                node = make_node(std::move(node), std::move(rhs), op.value());
            }
            
            return node;
        }

        Node factor() {
            Node node = unary();

            optional<Token> op;
            while ( (op = match(SLASH, STAR)) ) {
                Node rhs = unary();
                node = make_node(std::move(node), std::move(rhs), op.value());
            }

            return node;
        }

        Node unary() {
            optional<Token> op = match(BANG, MINUS);
            if (op)
                return make_node(unary(), op.value());

            return primary();
        }

        Node primary() {
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
                return make_node(false);
            case TRUE:
                return make_node(true);
            case NIL:
                return make_node();
            case NUMBER:
                return make_node(token->getLiteral().number);
            case STRING:
                return make_node(token->getLiteral().string);
            case LEFT_PAREN:
                Node node = expression();
                expect(RIGHT_PAREN);
                return make_node(std::move(node));
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