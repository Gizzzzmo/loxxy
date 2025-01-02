module;

#include <string_view>
#include <exception>
#include <concepts>
#include <optional>
#include <iostream>
#include <optional>
#include <ostream>
#include <sstream>
#include <utility>
#include "loxxy/ast.hpp"

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


export namespace loxxy {

class ParseError : std::exception {};

template<typename istream, typename Builder>
class Parser {
    using Payload = typename Builder::Payload;
    using Indirection = typename Builder::Indirection;
    static constexpr bool ptr_variant = Builder::ptr_variant;
    USING_FAMILY(Payload, Indirection, ptr_variant);

    public:
        template<typename... Args>
        Parser(istream& stream, Args&&... args) : stream(stream), node_builder(std::forward<Args>(args)...) {}
        auto parse() -> TURoot {
            TURoot root;
            while (!eof && !match(END_OF_FILE)) {
                try {
                    auto maybe_decl = declaration(); 
                    if (maybe_decl)
                        root.statements.push_back(std::move(maybe_decl.value()));
                    if (matchNewLine())
                        break;
                } catch(const ParseError&) {
                    if (eof || match(END_OF_FILE))
                        break;
                    stream.get();
                    continue;
                }
            }
            return root;
        }
    private:
        auto matchNewLine() -> bool {
            if (stream.peek().getType() == NEW_LINE) {
                stream.get();
                return true;
            }
            return false;
        }

        void skipNewLines() {
            while (stream.peek().getType() == NEW_LINE)
                stream.get();
        }
        
        template<typename NodeType, typename... Args>
        auto make_node(Args&&... args) {
            return node_builder(mark<NodeType>, std::forward<Args>(args)...);
        }

        template<std::same_as<TokenType>... T>
        auto match(T... types) -> optional<Token> {
            skipNewLines();
            if (!eof && ((stream.peek().getType() == types) || ...))
                return stream.get();

            return std::nullopt;
        }

        template<std::same_as<TokenType>... T>
        auto matchAllBut(T... types) -> optional<Token> {
            skipNewLines();
            if (!eof && ((stream.peek().getType() != types) && ...))
                return stream.get();

            return std::nullopt;
        }

        auto expect(TokenType type) -> Token {
            optional<Token> t = match(type);
            if (t)
                return t.value();

            std::stringstream message;
            message << "Expected " << type << " token, but found " 
                << stream.peek().getType() << " token instead.";

            throw error(stream.peek(), message.str());
        }

        auto error(const Token& token, const std::string_view& message) -> ParseError {
            std::cerr << "[line " << token.getLine() << "] Error: " << message << std::endl;
            panic = true;
            return ParseError{};
        }

        auto declaration() -> optional<StmtPointer> {
            while (!eof && !match(END_OF_FILE)) { 
                try {
                    if (match(VAR))
                        return var_declaration();

                    return statement();
                } catch (ParseError& err) {
                    synchronize();
                }
            }
            eof = true;
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
            if (match(PRINT))
                return print_statement();

            return expression_statement();
        }

        auto print_statement() -> StmtPointer {
            ExprPointer expr = expression();
            expect(SEMICOLON);
            return make_node<PrintStmt>(std::move(expr));
        }

        auto expression_statement() -> StmtPointer {
            ExprPointer expr = expression();
            expect(SEMICOLON);
            return make_node<ExpressionStmt>(std::move(expr));
        }

        auto expression() -> ExprPointer {
            return assignment();
        }

        struct VariableName : IndirectVisitor<VariableName, void, Indirection> {
            using IndirectVisitor<VariableName, void, Indirection>::IndirectVisitor;
            using IndirectVisitor<VariableName, void, Indirection>::operator();

            template<typename T>
            auto operator()(const T&) const -> const persistent_string<>*{
                return nullptr;
            }
            auto operator()(const VarExpr& expr) -> const persistent_string<>* {
                return expr.identifier;
            }
        };

        auto assignment() -> ExprPointer {
            ExprPointer node = equality();

            optional<Token> op;
            if ( (op = match(EQUAL)) ) {
                ExprPointer value = assignment();
                
                const persistent_string<>* assign_target = utils::visit(VariableName{}, node);
                if (assign_target != nullptr)
                    return make_node<AssignExpr>(assign_target, std::move(value));
                
                error(op.value(), "Invalid assignment target");
            }
            return node;
        }

        auto comma() -> ExprPointer {
            ExprPointer node = equality();

            optional<Token> op;
            if ( (op = match(COMMA)) ) {
                ExprPointer rhs = comma();
                node = make_node<BinaryExpr>(std::move(node), std::move(rhs), op.value());
            }

            return node;
        }
        
        auto equality() -> ExprPointer {
            ExprPointer node = comparison();
            optional<Token> op;
            while ( (op = match(BANG_EQUAL, EQUAL_EQUAL)) ) {
                ExprPointer rhs = comparison();
                node = make_node<BinaryExpr>(std::move(node), std::move(rhs), op.value());
            }

            return node;
        }

        auto comparison() -> ExprPointer {
            ExprPointer node = term();

            optional<Token> op;
            while ( (op = match(GREATER, GREATER_EQUAL, LESS, LESS_EQUAL)) ) {
                ExprPointer rhs = term();
                node = make_node<BinaryExpr>(std::move(node), std::move(rhs), op.value());
            }

            return node;
        }

        auto term() -> ExprPointer  {
            ExprPointer node = factor();
            optional<Token> op;
            while( (op = match(MINUS, PLUS)) ) {
                ExprPointer rhs = factor();
                node = make_node<BinaryExpr>(std::move(node), std::move(rhs), op.value());
            }
            
            return node;
        }

        auto factor() -> ExprPointer {
            ExprPointer node = unary();

            optional<Token> op;
            while ( (op = match(SLASH, STAR)) ) {
                ExprPointer rhs = unary();
                node = make_node<BinaryExpr>(std::move(node), std::move(rhs), op.value());
            }

            return node;
        }

        auto unary() -> ExprPointer {
            optional<Token> op = match(BANG, MINUS);
            if (op)
                return make_node<UnaryExpr>(unary(), op.value());

            return primary();
        }

        auto primary() -> ExprPointer {
            optional<Token> token = match(
                FALSE, TRUE, NIL, NUMBER, STRING, LEFT_PAREN, END_OF_FILE, IDENTIFIER
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
            case IDENTIFIER:
                return make_node<VarExpr>(&token->getLexeme());
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
                default:
                    break;
                }
            }
            eof = true;
        }

        istream& stream;
        Builder node_builder;
        bool eof = false;
        bool panic = false;
};

template<typename Stream, typename Builder>
Parser(Stream, Builder) -> Parser<Stream, Builder>;

} // namespace loxxy
