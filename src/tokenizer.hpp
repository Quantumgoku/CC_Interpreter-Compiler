#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include<algorithm>
#include "token.hpp"

class Tokenizer {
public:
    Tokenizer(const std::string& input) : text(input) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        bool hitDef = false;
        while (!isAtEnd()) {
            char c = peek();
            if (isspace(c)) {
                if (c == '\n') line++;
                consume();
                continue;
            }
            if (c == '/' && peek(1) == '/') {
                // Skip comment
                while (!isAtEnd() && peek() != '\n') consume();
                continue;
            }
            if (c == '"') {
                tokens.push_back(stringToken());
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
            // Multi-char operators
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
                case '!':
                    tokens.push_back(match('=') ?
                        complexToken(TokenType::BANG_EQUAL, "BANG_EQUAL", "!=") :
                        simpleToken(TokenType::BANG, "BANG"));
                    break;
                case '=':
                    tokens.push_back(match('=') ?
                        complexToken(TokenType::EQUAL_EQUAL, "EQUAL_EQUAL", "==") :
                        simpleToken(TokenType::EQUAL, "EQUAL"));
                    break;
                case '<':
                    tokens.push_back(match('=') ?
                        complexToken(TokenType::LESS_EQUAL, "LESS_EQUAL", "<=") :
                        simpleToken(TokenType::LESS, "LESS"));
                    break;
                case '>':
                    tokens.push_back(match('=') ?
                        complexToken(TokenType::GREATER_EQUAL, "GREATER_EQUAL", ">=") :
                        simpleToken(TokenType::GREATER, "GREATER"));
                    break;
                default:
                    std::cerr << "[line " << line << "] Error: Unexpected character: " << c << std::endl;
                    hitDef = true;
                    consume();
                    break;
            }
        }
        std::cout << "EOF  null" << std::endl;
        if (hitDef) exit(65);
        tokens.push_back(Token(TokenType::END_OF_FILE, std::nullopt, std::nullopt, line));
        return tokens;
    }

private:
    std::string text;
    int current = 0;
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

    Token simpleToken(TokenType type, const char* name) {
        char c = consume();
        std::cout << name << " " << c << " null" << std::endl;
        return Token(type, std::string(1, c), std::nullopt, line);
    }

    Token complexToken(TokenType type, const char* name, const std::string& lex) {
        consume(); consume();
        std::cout << name << " " << lex << " null" << std::endl;
        return Token(type, lex, std::nullopt, line);
    }

    // String literal
    Token stringToken() {
        consume(); 
        std::string value;
        while (!isAtEnd() && peek() != '"') {
            if (peek() == '\n') line++;
            value += consume();
        }
        if (isAtEnd()) {
            std::cerr << "[line " << line << "] Error: Unterminated string." << std::endl;
            exit(65);
        }
        consume(); 
        std::cout << "STRING \"" << value << "\" " << value << std::endl;
        return Token(TokenType::STRING, value, value, line);
    }

    // Identifier or keyword
    Token identifierToken() {
        std::string value;
        while (isalpha(peek()) || isdigit(peek()) || peek() == '_') value += consume();
        static const std::unordered_map<std::string, TokenType> keywords = {
            {"and", TokenType::AND}, {"class", TokenType::CLASS}, {"else", TokenType::ELSE},
            {"false", TokenType::FALSE}, {"fun", TokenType::FUN}, {"for", TokenType::FOR},
            {"if", TokenType::IF}, {"nil", TokenType::NIL}, {"or", TokenType::OR},
            {"print", TokenType::PRINT}, {"return", TokenType::RETURN}, {"super", TokenType::SUPER},
            {"this", TokenType::THIS}, {"true", TokenType::TRUE}, {"var", TokenType::VAR},
            {"while", TokenType::WHILE}
        };
        auto it = keywords.find(value);
        std::string keywordType = (it!=keywords.end()) ? it->first : "IDENTIFIER";
        std::transform(keywordType.begin(), keywordType.end(), keywordType.begin(), [](unsigned char c){ return std::toupper(c);});
        TokenType type = (it != keywords.end()) ? it->second : TokenType::IDENTIFIER;
        std::cout << (type == TokenType::IDENTIFIER ? "IDENTIFIER " : keywordType)<<" " << value << " null" << std::endl;
        return Token(type, value, std::nullopt, line);
    }

    // Number literal
    Token numberToken() {
        std::string value;
        while (isdigit(peek())) value += consume();
        bool isFloat = false;
        if (peek() == '.' && isdigit(peek(1))) {
            isFloat = true;
            value += consume(); // consume '.'
            while (isdigit(peek())) value += consume();
        }
        std::string normalized = normalizeNumberLiteral(value);
        std::cout << "NUMBER " << value << " " << normalized << std::endl;
        return Token(TokenType::NUMBER, value, normalized, line);
    }

    // Normalize number for output
    std::string normalizeNumberLiteral(const std::string& numStr) {
        size_t dotPos = numStr.find('.');
        if (dotPos == std::string::npos) return numStr + ".0";
        size_t endPos = numStr.size() - 1;
        while (endPos > dotPos && numStr[endPos] == '0') endPos--;
        if (endPos == dotPos) endPos++;
        return numStr.substr(0, endPos + 1);
    }
};