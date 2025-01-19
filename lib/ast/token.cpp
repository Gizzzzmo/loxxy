module;

#include <iostream>

export module ast:token;
import utils.string_store;

using utils::persistent_string;

export namespace loxxy {

using namespace utils;

enum TokenType {
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    COMMA,
    DOT,
    MINUS,
    PLUS,
    SEMICOLON,
    SLASH,
    STAR,

    BANG,
    BANG_EQUAL,
    EQUAL,
    EQUAL_EQUAL,
    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL,

    IDENTIFIER,
    STRING,
    NUMBER,
    COMMENT,

    AND,
    CLASS,
    ELSE,
    FALSE,
    FOR,
    FUN,
    IF,
    NIL,
    OR,
    PRINT,
    RETURN,
    SUPER,
    THIS,
    TRUE,
    VAR,
    WHILE,

    END_OF_FILE,
    NEW_LINE
};

inline auto operator<<(std::ostream& ostream, TokenType type) -> std::ostream& {
    switch (type) {
    case LEFT_PAREN:
        ostream << "LEFT_PAREN";
        break;
    case RIGHT_PAREN:
        ostream << "RIGHT_PAREN";
        break;
    case LEFT_BRACE:
        ostream << "LEFT_BRACE";
        break;
    case RIGHT_BRACE:
        ostream << "RIGHT_BRACE";
        break;
    case COMMA:
        ostream << "COMMA";
        break;
    case DOT:
        ostream << "DOT";
        break;
    case MINUS:
        ostream << "MINUS";
        break;
    case PLUS:
        ostream << "PLUS";
        break;
    case SEMICOLON:
        ostream << "SEMICOLON";
        break;
    case SLASH:
        ostream << "SLASH";
        break;
    case STAR:
        ostream << "STAR";
        break;
    case BANG:
        ostream << "BANG";
        break;
    case BANG_EQUAL:
        ostream << "BANG_EQUAL";
        break;
    case EQUAL:
        ostream << "EQUAL";
        break;
    case EQUAL_EQUAL:
        ostream << "EQUAL_EQUAL";
        break;
    case GREATER:
        ostream << "GREATER";
        break;
    case GREATER_EQUAL:
        ostream << "GREATER_EQUAL";
        break;
    case LESS:
        ostream << "LESS";
        break;
    case LESS_EQUAL:
        ostream << "LESS_EQUAL";
        break;
    case IDENTIFIER:
        ostream << "IDENTIFIER";
        break;
    case STRING:
        ostream << "STRING";
        break;
    case NUMBER:
        ostream << "NUMBER";
        break;
    case AND:
        ostream << "AND";
        break;
    case CLASS:
        ostream << "CLASS";
        break;
    case ELSE:
        ostream << "ELSE";
        break;
    case FALSE:
        ostream << "FALSE";
        break;
    case FUN:
        ostream << "FUN";
        break;
    case FOR:
        ostream << "FOR";
        break;
    case IF:
        ostream << "IF";
        break;
    case NIL:
        ostream << "NIL";
        break;
    case OR:
        ostream << "OR";
        break;
    case PRINT:
        ostream << "PRINT";
        break;
    case RETURN:
        ostream << "RETURN";
        break;
    case SUPER:
        ostream << "SUPER";
        break;
    case THIS:
        ostream << "THIS";
        break;
    case TRUE:
        ostream << "TRUE";
        break;
    case VAR:
        ostream << "VAR";
        break;
    case WHILE:
        ostream << "WHILE";
        break;
    case COMMENT:
        ostream << "COMMENT";
        break;
    case END_OF_FILE:
        ostream << "END_OF_FILE";
        break;
    case NEW_LINE:
        ostream << "NEW_LINE";
        break;
    }
    return ostream;
};

union Literal {
    double number;
    const persistent_string<char>* string;
    Literal(double number) : number(number) {}
    Literal(const persistent_string<char>* string) : string(string) {}
    Literal() : number(0) {}
};

class Token {
public:
    Token(TokenType type, const persistent_string<char>* lexeme, Literal literal, int line, int column)
        : type(type), lexeme(lexeme), literal(literal), line(line), column(column) {}

    friend auto operator<<(std::ostream& ostream, const Token& token) -> std::ostream& {
        ostream << token.type << " " << *token.lexeme;
        if (token.type == TokenType::STRING)
            ostream << " " << *token.literal.string;
        else if (token.type == TokenType::NUMBER)
            ostream << " " << token.literal.number;
        ostream << ":" << token.line << "," << token.column;
        return ostream;
    }

    auto getType() const -> TokenType { return type; }

    auto getLexeme() const -> const persistent_string<char>& { return *lexeme; }

    auto getLiteral() const -> const Literal& { return literal; }

    auto getLine() const -> int { return line; }

    auto getColumn() const -> int { return column; }

private:
    TokenType type;
    const persistent_string<char>* lexeme;
    Literal literal;
    int line;
    int column;
};

} // namespace loxxy
