module;

#include <concepts>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <utility>
#include <array>
#include <functional>
#include <iostream>

#include "tsl/htrie_map.h"

export module lexer;

import ast;
import utils.string_store;

using namespace utils;
using std::byte;

struct CompileTimeInit {
    alignas(persistent_string<char>) std::array<byte, 1<<13> lex_store;
    byte* start_simple;
    byte* start_ids;
    size_t n_ids; 
    size_t simple_space;
    int id_space_exponent;
    byte* greater_equal; 
    byte* less_equal; 
    byte* equal_equal; 
    byte* bang_equal;
};

static CompileTimeInit getInitialIdStore() {
    CompileTimeInit init;

    byte* ptr = init.lex_store.begin();
    init.start_simple = ptr;

    init.simple_space = sizeof(persistent_string<char>) + 2* sizeof(char);
    init.simple_space += alignof(persistent_string<char>) - init.simple_space % alignof(persistent_string<char>);
    assert(init.simple_space%alignof(persistent_string<char>) == 0);

    for (int i = 0; i < 256; i++) {
        auto str = persistent_string<char>::construct_at(ptr);
        str->len = 1;
        str->chars[0] = static_cast<char>(i);
        ptr += init.simple_space;
    }

    const char* greater_equal = ">=";
    const char* less_equal = "<=";
    const char* equal_equal = "==";
    const char* bang_equal = "!=";

    init.greater_equal = ptr;
    auto str = persistent_string<char>::construct_at(ptr);
    str->len = 2;
    for(int i = 0; i < str->len + 1; i++){
        str->chars[i] = greater_equal[i];
    }
    ptr += init.simple_space;


    init.less_equal = ptr;
    str = persistent_string<char>::construct_at(ptr);
    str->len = 2;
    for(int i = 0; i < str->len + 1; i++){
        str->chars[i] = less_equal[i];
    }
    ptr += init.simple_space;


    init.equal_equal = ptr;
    str = persistent_string<char>::construct_at(ptr);
    str->len = 2;
    for(int i = 0; i < str->len + 1; i++){
        str->chars[i] = equal_equal[i];
    }
    ptr += init.simple_space;


    init.bang_equal = ptr;
    str = persistent_string<char>::construct_at(ptr);
    str->len = 2;
    for(int i = 0; i < str->len + 1; i++){
        str->chars[i] = bang_equal[i];
    }
    ptr += init.simple_space;

    const char* keywords[] {
        "and", "class", "else", "false",
        "for", "fun", "if", "nil", "or", "print", "return", "super",
        "this", "true", "var", "while"
    };
    size_t max_len = 0;
    init.n_ids = 0;
    init.start_ids = ptr;
    for (const char* keyword : keywords) {
        size_t len = std::char_traits<char>::length(keyword);
        init.n_ids++;
        if (len > max_len)
            max_len = len;
    }
    size_t space_per_keyword = 1;
    init.id_space_exponent = 0;
    while (space_per_keyword < max_len + sizeof(persistent_string<char>) || space_per_keyword < alignof(persistent_string<char>)) {
        space_per_keyword <<= 1;
        init.id_space_exponent++;
    }

    for (const char* keyword : keywords) {
        size_t len = std::char_traits<char>::length(keyword);
        
        auto str = persistent_string<char>::construct_at(ptr);
        str->len = len;
        for(int i = 0; i < len; i++){
            str->chars[i] = keyword[i];
        }
        ptr += space_per_keyword;
    }
    
    return init;
}

enum LiteralFormat {
    DEC, HEX, BIN, OCT
};


static const CompileTimeInit init = getInitialIdStore();

export namespace loxxy {

template<typename istream = std::ifstream>
class Loxxer {

    public:
        Loxxer(istream&& file, std::function<bool(Token&&)> sink, size_t line = 0, size_t column = 0)
        : file(std::move(file)), sink(sink), line(line), column(column) {
            initStoreAndTable();
        }

        Loxxer(const std::filesystem::path& filepath, std::function<bool(Token&&)> sink)
        requires (std::same_as<istream, std::ifstream>)
        : file(filepath), sink(sink), line(0), column(0) {
            initStoreAndTable();
        }

        bool scanTokens() {
            while(!file.eof() && !file.fail() && !done) {
                scanToken();
            }

            if (file.eof())
                addToken(
                    END_OF_FILE,
                    reinterpret_cast<persistent_string<char>*>(init.start_simple)
                );

            return hadError;
        }

        bool scanTokensLine() {

            int last_line = line;
            while(!file.eof() && !file.fail() && !done) {
                scanToken();
                if (last_line != line)
                    break;

                last_line = line;
            }

            if (file.eof())
                addToken(
                    END_OF_FILE,
                    reinterpret_cast<persistent_string<char>*>(init.start_simple)
                );

            return hadError;
        }


    private:
        void initStoreAndTable() {
            const size_t incr = 1<<init.id_space_exponent;

            for (size_t i = 0; i < init.n_ids; i++) {
                persistent_string<char>* str_ptr = reinterpret_cast<persistent_string<char>*>(
                    &init.start_ids[i*incr]
                );
                table.insert(*str_ptr, str_ptr);
            }
            lex_store.start_recording();
            string_store.start_recording();
        }
        void error(std::string_view message) {
            std::cerr << "[line " << line << ":" << column << "] " 
                << "Error: " << message << std::endl;
            hadError = true;
        }
        bool match(char c) {
            if (file.peek() != c || file.eof())
                return false;
            file.get();
            return true;
        }
        void addToken(
            TokenType type,
            const persistent_string<char>* lexeme,
            Literal literal = Literal()
        ) { sink(Token(type, lexeme, literal, line, column)); }

        static bool isAlpha(char c) { 
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
        }

        static bool isDigit(char c) {
            return c >= '0' && c <= '9';
        }

        static bool isHexDigit(char c) {
            return isDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
        }

        static bool isOctDigit(char c) {
            return c >= '0' && c <= '7';
        }

        static bool isBinDigit(char c) {
            return c == '0' || c == '1';
        }

        static bool isAlphaNumeric(char c) {
            return isAlpha(c) || isDigit(c);
        }

        static double parseNumber(LiteralFormat format, std::string_view str) {
            double base;
            switch (format) {
            case DEC:
                base = 10;
                break;
            case HEX:
                base = 16;
                break;
            case BIN:
                base = 2;
                break;
            case OCT:
                base = 8;
                break;
            }
            double mult = base;
            double d = 0;
            bool after_decimal_point = false;
            for (char c : str) {
                if (c == '.') {
                    after_decimal_point = true;
                    continue;
                }
                
                double digit;
                if (c >= 'A' && c <= 'F')
                    digit = c - 'A' + 10;
                else if (c >= 'a' && c <= 'f')
                    digit = c - 'a' + 10;
                else
                    digit = c - '0';
                
                if (!after_decimal_point) {
                    d *= mult;
                    d += digit;
                }
                else {
                    d += digit/mult;
                    mult *= base;
                }
            }
            return d;
        }

        const persistent_string<char>* addToTableIfNotExists(const persistent_string<char>* str) {
            auto it = table.find(*str);
            if (it != table.end())
                return it.value();

            table.insert(*str, str);
            return str;
        }

        const persistent_string<char>* resolveStringStoreRecording() {
            const persistent_string<char>* str = string_store.peek_recording();
            const persistent_string<char>* str_resolved = addToTableIfNotExists(str);
            if (str == str_resolved) {
                string_store.finish_recording();
                string_store.start_recording();
            }
            else
                string_store.reset_recording();

            return str_resolved;
        }

        const persistent_string<char>* resolveLexStoreRecording() {
            const persistent_string<char>* lexeme = lex_store.peek_recording();
            const persistent_string<char>* lexeme_resolved = addToTableIfNotExists(lexeme);
            if (lexeme == lexeme_resolved) {
                lex_store.finish_recording();
                lex_store.start_recording();
            }
            else
                lex_store.reset_recording();

            return lexeme_resolved;
        }

        void addIdentifier(char start) {
            lex_store.recordChar(start);

            while (!file.eof() && isAlphaNumeric(file.peek()))
                lex_store.recordChar(file.get());

            const persistent_string<char>* id = resolveLexStoreRecording();
            
            TokenType type;
            if (reinterpret_cast<const byte*>(id) >= init.lex_store.begin()
                && reinterpret_cast<const byte*>(id) < init.lex_store.end())
            {
                size_t offset = reinterpret_cast<const byte*>(id) - init.start_ids;
                type = static_cast<TokenType>(AND + (offset>>init.id_space_exponent));
            }
            else
                type = IDENTIFIER;

            addToken(type, id);
        }

        void addIntegralLiteral(char start) {
            const persistent_string<char>* lexeme = lex_store.peek_recording();
            
            size_tt number_start = lexeme->len;
            lex_store.recordChar(start);
            LiteralFormat format = DEC;

            std::function<bool(char)> filter = isDigit;
            if (start == '0') {
                if(match('x')) {
                    format = HEX;
                    lex_store.recordChar('x');
                    number_start = lexeme->len;
                    filter = isHexDigit;
                }
                else if(match('b')){
                    format = BIN;
                    lex_store.recordChar('b');
                    number_start = lexeme->len;
                    filter = isBinDigit;
                }
                else if(match('o')) {
                    format = OCT;
                    lex_store.recordChar('o');
                    number_start = lexeme->len;
                    filter = isOctDigit;
                }
            }

            if (format != DEC && !filter(file.peek())) {
                error("missing digit");
                return;
            }
            
            bool floating_point = false;
            while(!file.eof() && (filter(file.peek()) || file.peek() == '.')) {
                if (file.peek() == '.' && floating_point)
                    break;
                if (file.peek() == '.')
                    floating_point = true;

                lex_store.recordChar(file.get());
            }

            while (!file.eof() && isHexDigit(file.peek())) {
                lex_store.recordChar(file.get());
                error("invalid digit");
            }

            size_tt number_end = lexeme->len;

            while (!file.eof() && isAlphaNumeric(file.peek())) {
                lex_store.recordChar(file.get());
            }
            
            std::string_view number_string(
                &lexeme->chars[number_start], 
                number_end - number_start
            );
            Literal literal = parseNumber(format, number_string);

            lexeme = resolveLexStoreRecording();

            addToken(NUMBER, lexeme, literal);
        }

        char escapeSequence() {
            lex_store.recordChar('\\');
            char c = file.get();
            lex_store.recordChar(c);
            
            switch (c) {
            case 'n':
                return '\n';
            case '\\':
                return '\\';
            case '"':
                return '"';
            case 't':
                return '\t';
            case '0':
                return '\0';
            default:
                error("invalid escape sequence");
                return '0';
            }
        }

        void addStringLiteral() {
            lex_store.recordChar('"');
            while(!file.eof() && file.peek() != '"') {
                if (match('\\'))
                    string_store.recordChar(escapeSequence());
                else {
                    string_store.recordChar(file.peek());
                    lex_store.recordChar(file.get());
                }
            }
            if (file.eof()) {
                error("unterminated string literal");
            }
            lex_store.recordChar(file.get());
            
            const persistent_string<char>* lexeme = resolveLexStoreRecording();
            const persistent_string<char>* string = resolveStringStoreRecording();

            addToken(STRING, lexeme, string);
        }

        void scanToken() {
            char c = file.get();
            switch (c) {
            case '\n':
                line++;
                column = 0;
                return;
            case ' ':
            case '\t':
                break;
            case '{':
                addToken(
                    LEFT_BRACE, 
                    reinterpret_cast<persistent_string<char>*>(
                        &init.start_simple[c*init.simple_space]
                    )
                );
                break;
            case '}':
                addToken(
                    RIGHT_BRACE,
                    reinterpret_cast<persistent_string<char>*>(
                        &init.start_simple[c*init.simple_space]
                    )
                );
                break;
            case '(':
                addToken(
                    LEFT_PAREN,
                    reinterpret_cast<persistent_string<char>*>(
                        &init.start_simple[c*init.simple_space]
                    )
                );
                break;
            case ')':
                addToken(
                    RIGHT_PAREN,
                    reinterpret_cast<persistent_string<char>*>(
                        &init.start_simple[c*init.simple_space]
                    )
                );
                break;
            case ',':
                addToken(
                    COMMA,
                    reinterpret_cast<persistent_string<char>*>(
                        &init.start_simple[c*init.simple_space]
                    )
                );
                break;
            case '.':
                addToken(
                    DOT, 
                    reinterpret_cast<persistent_string<char>*>(
                        &init.start_simple[c*init.simple_space]
                    )
                );
                break;
            case '-':
                addToken(
                    MINUS,
                    reinterpret_cast<persistent_string<char>*>(
                        &init.start_simple[c*init.simple_space]
                    )
                );
                break;
            case '+':
                addToken(
                    PLUS,
                    reinterpret_cast<persistent_string<char>*>(
                        &init.start_simple[c*init.simple_space]
                    )
                );
                break;
            case ';':
                addToken(
                    SEMICOLON,
                    reinterpret_cast<persistent_string<char>*>(
                        &init.start_simple[c*init.simple_space]
                    )
                );
                break;
            case '/':
                if (match('/')) {
                    std::string comment;
                    comment.reserve(32);
                    comment.append("//");

                    while(!file.eof() && '\n' != file.peek())
                        file.get();
                    
                    //addToken(COMMENT, std::move(comment));
                    break;
                }

                addToken(
                    SLASH,
                    reinterpret_cast<persistent_string<char>*>(
                        &init.start_simple[c*init.simple_space]
                    )
                );
                break;
            case '*':
                addToken(
                    STAR,
                    reinterpret_cast<persistent_string<char>*>(
                        &init.start_simple[c*init.simple_space]
                    )
                );
                break;
            case '!':
                if (match('='))
                    addToken(
                        BANG_EQUAL,
                        reinterpret_cast<persistent_string<char>*>(init.bang_equal)
                    );
                else
                    addToken(
                        BANG,
                        reinterpret_cast<persistent_string<char>*>(
                            &init.start_simple[c*init.simple_space]
                        )
                    );
                break;
            case '=':
                if (match('='))
                    addToken(
                        EQUAL_EQUAL,
                        reinterpret_cast<persistent_string<char>*>(init.equal_equal)
                    );
                else
                    addToken(
                        EQUAL,
                        reinterpret_cast<persistent_string<char>*>(
                            &init.start_simple[c*init.simple_space]
                        )
                    );
                break;
            case '<':
                if (match('='))
                    addToken(
                        LESS_EQUAL,
                        reinterpret_cast<persistent_string<char>*>(init.less_equal)
                    );
                else
                    addToken(
                        LESS,
                        reinterpret_cast<persistent_string<char>*>(
                            &init.start_simple[c*init.simple_space]
                        )
                    );
                break;
            case '>':
                if (match('='))
                    addToken(
                        GREATER_EQUAL,
                        reinterpret_cast<persistent_string<char>*>(init.greater_equal)
                    );
                else
                    addToken(
                        GREATER,
                        reinterpret_cast<persistent_string<char>*>(
                            &init.start_simple[c*init.simple_space]
                        )
                    );
                break;
            case '"':
                addStringLiteral();
                break;
            default:
                if (isAlpha(c))
                    addIdentifier(c);
                else if (isDigit(c))
                    addIntegralLiteral(c);
                break;
            }
            column++;
        }
        istream file;

        std::function<bool(Token&&)> sink;
        size_t line;
        size_t column;
        bool done = false;
        bool hadError = false;

        tsl::htrie_map<char, const persistent_string<char>*> table;

        persistent_string_store<char> lex_store;
        persistent_string_store<char> string_store;
};

} // namespace loxxy