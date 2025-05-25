#pragma once
#include <string>
#include <vector>
#include <optional>
#include <iostream>


enum class TokenType {
    // Single-character tokens.
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE, COMMA, DOT,
    MINUS, PLUS, SEMICOLON, SLASH, STAR,
    // One or two character tokens.
    BANG, BANG_EQUAL, EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL, LESS, LESS_EQUAL,
    // Literals.
    IDENTIFIER, STRING, NUMBER,
    // Keywords.
    AND, CLASS, ELSE, FALSE, FUN, FOR, IF, NIL, OR, PRINT, RETURN,
    SUPER, THIS, TRUE, VAR, WHILE,
    END_OF_FILE,
};

struct Token {
    TokenType type;
    std::optional<std::string> lexme;
    std::optional<std::string> lit;
    int line;
    Token(TokenType type, std::optional<std::string> lexme, std::optional<std::string> lit, int line)
        : type(type), lexme(lexme), lit(lit), line(line) {}
};