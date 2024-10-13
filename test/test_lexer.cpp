
#include <gtest/gtest.h>
#include <sstream>
#include <vector>

import lexer;
import ast;

using namespace loxxy;

TEST(LoxxerTest, SingleCharTokens) {
    std::stringstream ss("(){},.-+;/*");
    std::vector<Token> tokens;
    Loxxer<std::stringstream> loxxer(std::move(ss), [&tokens](Token&& token){
        tokens.push_back(std::move(token));
        return true;
    });

    loxxer.scanTokens();
    EXPECT_EQ(tokens.size(), 12);
    EXPECT_EQ(tokens[0].getType(), TokenType::LEFT_PAREN);
    EXPECT_EQ(tokens[1].getType(), TokenType::RIGHT_PAREN);
    EXPECT_EQ(tokens[2].getType(), TokenType::LEFT_BRACE);
    EXPECT_EQ(tokens[3].getType(), TokenType::RIGHT_BRACE);
    EXPECT_EQ(tokens[4].getType(), TokenType::COMMA);
    EXPECT_EQ(tokens[5].getType(), TokenType::DOT);
    EXPECT_EQ(tokens[6].getType(), TokenType::MINUS);
    EXPECT_EQ(tokens[7].getType(), TokenType::PLUS);
    EXPECT_EQ(tokens[8].getType(), TokenType::SEMICOLON);
    EXPECT_EQ(tokens[9].getType(), TokenType::SLASH);
    EXPECT_EQ(tokens[10].getType(), TokenType::STAR);
    EXPECT_EQ(tokens[11].getType(), TokenType::END_OF_FILE);
}

TEST(LoxxerTest, TwoCharTokens) {
    std::stringstream ss("<=>===!=");
    std::vector<Token> tokens;
    Loxxer<std::stringstream> loxxer(std::move(ss), [&tokens](Token&& token){
        tokens.push_back(std::move(token));
        return true;
    });

    loxxer.scanTokens();

    EXPECT_EQ(tokens.size(), 5);
    EXPECT_EQ(tokens[0].getType(), TokenType::LESS_EQUAL);
    EXPECT_EQ(tokens[1].getType(), TokenType::GREATER_EQUAL);
    EXPECT_EQ(tokens[2].getType(), TokenType::EQUAL_EQUAL);
    EXPECT_EQ(tokens[3].getType(), TokenType::BANG_EQUAL);
    EXPECT_EQ(tokens[4].getType(), TokenType::END_OF_FILE);
}

TEST(LoxxerTest, Keywords) {
    std::stringstream ss("and class else false for fun if nil or print return super this true var while");
    std::vector<Token> tokens;
    Loxxer<std::stringstream> loxxer(std::move(ss), [&tokens](Token&& token){
        tokens.push_back(std::move(token));
        return true;
    });

    loxxer.scanTokens();

    EXPECT_EQ(tokens.size(), 17);
    EXPECT_EQ(tokens[0].getType(), TokenType::AND);
    EXPECT_EQ(tokens[1].getType(), TokenType::CLASS);
    EXPECT_EQ(tokens[2].getType(), TokenType::ELSE);
    EXPECT_EQ(tokens[3].getType(), TokenType::FALSE);
    EXPECT_EQ(tokens[4].getType(), TokenType::FOR);
    EXPECT_EQ(tokens[5].getType(), TokenType::FUN);
    EXPECT_EQ(tokens[6].getType(), TokenType::IF);
    EXPECT_EQ(tokens[7].getType(), TokenType::NIL);
    EXPECT_EQ(tokens[8].getType(), TokenType::OR);
    EXPECT_EQ(tokens[9].getType(), TokenType::PRINT);
    EXPECT_EQ(tokens[10].getType(), TokenType::RETURN);
    EXPECT_EQ(tokens[11].getType(), TokenType::SUPER);
    EXPECT_EQ(tokens[12].getType(), TokenType::THIS);
    EXPECT_EQ(tokens[13].getType(), TokenType::TRUE);
    EXPECT_EQ(tokens[14].getType(), TokenType::VAR);
    EXPECT_EQ(tokens[15].getType(), TokenType::WHILE);
    EXPECT_EQ(tokens[16].getType(), TokenType::END_OF_FILE);
}

TEST(LoxxerTest, Identifiers) {
    std::stringstream ss("blib blab blub blibblab slurp");
    std::vector<Token> tokens;
    Loxxer<std::stringstream> loxxer(std::move(ss), [&tokens](Token&& token){
        tokens.push_back(std::move(token));
        return true;
    });

    loxxer.scanTokens();

    EXPECT_EQ(tokens.size(), 6);
    EXPECT_EQ(tokens.back().getType(), TokenType::END_OF_FILE);
    tokens.pop_back();
    for (const Token& token : tokens) {
        EXPECT_EQ(token.getType(), TokenType::IDENTIFIER);
    }
}

TEST(LoxxerTest, StringLiterals) {
    std::stringstream ss("\"blib\"\"blab\"\"blub\"\"blibblab\"\"slurp\"");
    std::vector<Token> tokens;
    Loxxer<std::stringstream> loxxer(std::move(ss), [&tokens](Token&& token){
        tokens.push_back(std::move(token));
        return true;
    });

    loxxer.scanTokens();

    EXPECT_EQ(tokens.size(), 6);
    EXPECT_EQ(tokens.back().getType(), TokenType::END_OF_FILE);
    tokens.pop_back();
    for (const Token& token : tokens) {
        EXPECT_EQ(token.getType(), TokenType::STRING);
    }
}

TEST(LoxxerTest, NumberLiterals) {
    std::stringstream ss("0 1 90 278 3.66 555.4");
    std::vector<Token> tokens;
    Loxxer<std::stringstream> loxxer(std::move(ss), [&tokens](Token&& token){
        tokens.push_back(std::move(token));
        return true;
    });

    loxxer.scanTokens();

    EXPECT_EQ(tokens.size(), 6);
    EXPECT_EQ(tokens.back().getType(), TokenType::END_OF_FILE);
    tokens.pop_back();
    for (const Token& token : tokens) {
        EXPECT_EQ(token.getType(), TokenType::NUMBER);
    }

}