#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <algorithm>
#include "token.hpp"

class Tokenizer {
public:
    Tokenizer(const std::string& input, bool print = false) : text(input), printToken(print) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        bool hitDef = false;
        while (!isAtEnd()) {
            start = current;
            char c = peek();
            if (isspace(c)) {
                if (c == '\n') line++;
                consume();
                continue;
            }
            if (c == '/' && peek(1) == '/') {
                while (!isAtEnd() && peek() != '\n') consume();
                continue;
            }
            if (c == '\"') {
                tokens.push_back(stringToken(hitDef));
                continue;
            }
            if (isalpha(c) || c == '_') {
                tokens.push_back(identifierToken());
                continue;
            }
            if (isdigit(c)) {
                tokens.push_back(numberToken());
                continue;
            }
            switch (c) {
                case '(': tokens.push_back(simpleToken(TokenType::LEFT_PAREN, "LEFT_PAREN")); break;
                case ')': tokens.push_back(simpleToken(TokenType::RIGHT_PAREN, "RIGHT_PAREN")); break;
                case '{': tokens.push_back(simpleToken(TokenType::LEFT_BRACE, "LEFT_BRACE")); break;
                case '}': tokens.push_back(simpleToken(TokenType::RIGHT_BRACE, "RIGHT_BRACE")); break;
                case ',': tokens.push_back(simpleToken(TokenType::COMMA, "COMMA")); break;
                case '.': tokens.push_back(simpleToken(TokenType::DOT, "DOT")); break;
                case '+': tokens.push_back(simpleToken(TokenType::PLUS, "PLUS")); break;
                case '-': tokens.push_back(simpleToken(TokenType::MINUS, "MINUS")); break;
                case '*': tokens.push_back(simpleToken(TokenType::STAR, "STAR")); break;
                case ';': tokens.push_back(simpleToken(TokenType::SEMICOLON, "SEMICOLON")); break;
                case '/': tokens.push_back(simpleToken(TokenType::SLASH, "SLASH")); break;
                case '!': tokens.push_back(match('=') ? complexToken(TokenType::BANG_EQUAL, "BANG_EQUAL", "!=") : simpleToken(TokenType::BANG, "BANG", "!")); break;
                case '=': tokens.push_back(match('=') ? complexToken(TokenType::EQUAL_EQUAL, "EQUAL_EQUAL", "==") : simpleToken(TokenType::EQUAL, "EQUAL", "=")); break;
                case '<': tokens.push_back(match('=') ? complexToken(TokenType::LESS_EQUAL, "LESS_EQUAL", "<=") : simpleToken(TokenType::LESS, "LESS", "<")); break;
                case '>': tokens.push_back(match('=') ? complexToken(TokenType::GREATER_EQUAL, "GREATER_EQUAL", ">=") : simpleToken(TokenType::GREATER, "GREATER", ">")); break;
                default:
                    std::cerr << "[line " << line << "] Error: Unexpected character: " << c << std::endl;
                    hitDef = true;
                    consume();
                    break;
            }
        }
        if (printToken) std::cout << "EOF  null" << std::endl;
        if (hitDef) exit(65);
        tokens.push_back(Token(TokenType::END_OF_FILE, std::nullopt, std::nullopt, line));
        return tokens;
    }

private:
    bool printToken = false;
    std::string text;
    int current = 0;
    int start = 0;
    int line = 1;

    char peek(int offset = 0) const {
        if (current + offset >= text.size()) return '\0';
        return text[current + offset];
    }
    char consume() { return text[current++]; }
    bool isAtEnd() const { return current >= text.size(); }
    bool match(char expected) {
        if (isAtEnd() || text[current] != expected) return false;
        current++;
        return true;
    }
    Token simpleToken(TokenType type, const std::string& typeName, const std::string& lexeme = "") {
        consume();
        if (printToken) std::cout << typeName << " " << lexeme << " null" << std::endl;
        return Token(type, lexeme.empty() ? std::nullopt : std::optional<std::string>(lexeme), std::nullopt, line);
    }
    Token complexToken(TokenType type, const std::string& typeName, const std::string& lexeme) {
        consume();
        if (printToken) std::cout << typeName << " " << lexeme << " null" << std::endl;
        return Token(type, lexeme, std::nullopt, line);
    }
    Token stringToken(bool& hitDef) {
        consume();
        std::string value;
        while (!isAtEnd() && peek() != '\"') {
            if (peek() == '\n') line++;
            value += consume();
        }
        if (isAtEnd()) {
            std::cerr << "[line " << line << "] Error: Unterminated string." << std::endl;
            hitDef = true;
            return Token(TokenType::STRING, std::nullopt, std::nullopt, line);
        }
        consume();
        if (printToken) std::cout << "STRING \"" << value << "\" " << value << std::endl;
        return Token(TokenType::STRING, value, value, line);
    }
    Token identifierToken() {
        while (isalnum(peek()) || peek() == '_') consume();
        std::string textVal = text.substr(start, current - start);
        static const std::unordered_map<std::string, TokenType> keywords = {
            {"and", TokenType::AND}, {"class", TokenType::CLASS}, {"else", TokenType::ELSE},
            {"false", TokenType::FALSE}, {"for", TokenType::FOR}, {"fun", TokenType::FUN},
            {"if", TokenType::IF}, {"nil", TokenType::NIL}, {"or", TokenType::OR},
            {"print", TokenType::PRINT}, {"return", TokenType::RETURN}, {"super", TokenType::SUPER},
            {"this", TokenType::THIS}, {"true", TokenType::TRUE}, {"var", TokenType::VAR},
            {"while", TokenType::WHILE}
        };
        TokenType type = keywords.count(textVal) ? keywords.at(textVal) : TokenType::IDENTIFIER;
        if (printToken) {
            std::string typeName = (type == TokenType::IDENTIFIER) ? "IDENTIFIER" : 
                (type == TokenType::AND ? "AND" :
                type == TokenType::CLASS ? "CLASS" :
                type == TokenType::ELSE ? "ELSE" :
                type == TokenType::FALSE ? "FALSE" :
                type == TokenType::FOR ? "FOR" :
                type == TokenType::FUN ? "FUN" :
                type == TokenType::IF ? "IF" :
                type == TokenType::NIL ? "NIL" :
                type == TokenType::OR ? "OR" :
                type == TokenType::PRINT ? "PRINT" :
                type == TokenType::RETURN ? "RETURN" :
                type == TokenType::SUPER ? "SUPER" :
                type == TokenType::THIS ? "THIS" :
                type == TokenType::TRUE ? "TRUE" :
                type == TokenType::VAR ? "VAR" :
                type == TokenType::WHILE ? "WHILE" : "IDENTIFIER");
            std::cout << typeName << " " << textVal << " null" << std::endl;
        }
        return Token(type, textVal, std::nullopt, line);
    }
    Token numberToken() {
        while (isdigit(peek())) consume();
        if (peek() == '.' && isdigit(peek(1))) {
            consume();
            while (isdigit(peek())) consume();
        }
        std::string numVal = text.substr(start, current - start);
        if (printToken) std::cout << "NUMBER " << numVal << " " << numVal << std::endl;
        return Token(TokenType::NUMBER, numVal, numVal, line);
    }
};
