#pragma once
#include <string>
#include <vector>
#include <optional>
#include "token.hpp"

class Scanner {
public:
    Scanner(const std::string& source);
    std::vector<Token> scanTokens();

private:
    std::string source;
    std::vector<Token> tokens;
    size_t start = 0;
    size_t current = 0;
    int line = 1;

    void scanToken();
    char advance();
    bool match(char expected);
    char peek();
    char peekNext();
    bool isAtEnd();
    void addToken(TokenType type, std::optional<std::string> literal = std::nullopt);
    void string();
    void number();
    void identifier();
    void handleTwoCharToken(char expected, TokenType matchType, TokenType defaultType);
    bool isAlpha(char c);
    bool isAlphaNumeric(char c);
    bool isDigit(char c);
    std::optional<std::string> extractLiteral(TokenType type);
};
