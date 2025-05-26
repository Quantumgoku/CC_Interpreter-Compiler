#pragma once
#include <string>
#include <vector>
#include <optional>
#include <cctype>
#include "token.hpp"

class Tokenizer {
public:
    Tokenizer(const std::string& source) : source(source) {}

    std::vector<Token> tokenize() {
        while (!isAtEnd()) {
            start = current;
            scanToken();
        }
        tokens.emplace_back(TokenType::END_OF_FILE, std::nullopt, std::nullopt, line);
        return tokens;
    }

private:
    std::string source;
    std::vector<Token> tokens;
    size_t start = 0;
    size_t current = 0;
    int line = 1;

    void scanToken() {
        char c = advance();
        switch (c) {
            case '(': addToken(TokenType::LEFT_PAREN); break;
            case ')': addToken(TokenType::RIGHT_PAREN); break;
            case '{': addToken(TokenType::LEFT_BRACE); break;
            case '}': addToken(TokenType::RIGHT_BRACE); break;
            case ',': addToken(TokenType::COMMA); break;
            case '.': addToken(TokenType::DOT); break;
            case '-': addToken(TokenType::MINUS); break;
            case '+': addToken(TokenType::PLUS); break;
            case ';': addToken(TokenType::SEMICOLON); break;
            case '*': addToken(TokenType::STAR); break;
            case '!': addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG); break;
            case '=': addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL); break;
            case '<': addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS); break;
            case '>': addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER); break;
            case '/':
                if (match('/')) {
                    while (peek() != '\n' && !isAtEnd()) advance();
                } else {
                    addToken(TokenType::SLASH);
                }
                break;
            case ' ':
            case '\r':
            case '\t':
                // Ignore whitespace
                break;
            case '\n':
                line++;
                break;
            case '"':
                string();
                break;
            default:
                if (isDigit(c)) {
                    number();
                } else if (isAlpha(c)) {
                    identifier();
                } else {
                    // Ignore unexpected characters for now
                }
                break;
        }
    }

    char advance() {
        return source[current++];
    }

    bool match(char expected) {
        if (isAtEnd() || source[current] != expected) return false;
        current++;
        return true;
    }

    char peek() {
        return isAtEnd() ? '\0' : source[current];
    }

    char peekNext() {
        return (current + 1 >= source.length()) ? '\0' : source[current + 1];
    }

    bool isAtEnd() {
        return current >= source.length();
    }

    void addToken(TokenType type, std::optional<std::string> literal = std::nullopt) {
        std::optional<std::string> text = (type == TokenType::END_OF_FILE) ? std::nullopt : std::optional<std::string>(source.substr(start, current - start));
        tokens.emplace_back(type, text, literal, line);
    }

    void string() {
        while (peek() != '"' && !isAtEnd()) {
            if (peek() == '\n') line++;
            advance();
        }

        if (isAtEnd()) {
            // Unterminated string
            return;
        }

        advance(); // Consume closing "
        std::string value = source.substr(start + 1, current - start - 2);
        addToken(TokenType::STRING, value);
    }

    void number() {
        while (isDigit(peek())) advance();

        if (peek() == '.' && isDigit(peekNext())) {
            advance();
            while (isDigit(peek())) advance();
        }

        addToken(TokenType::NUMBER, source.substr(start, current - start));
    }

    void identifier() {
        while (isAlphaNumeric(peek())) advance();

        std::string text = source.substr(start, current - start);
        TokenType type;
        if (text == "and") type = TokenType::AND;
        else if (text == "else") type = TokenType::ELSE;
        else if (text == "false") type = TokenType::FALSE;
        else if (text == "for") type = TokenType::FOR;
        else if (text == "fun") type = TokenType::FUN;
        else if (text == "if") type = TokenType::IF;
        else if (text == "nil") type = TokenType::NIL;
        else if (text == "or") type = TokenType::OR;
        else if (text == "print") type = TokenType::PRINT;
        else if (text == "return") type = TokenType::RETURN;
        else if (text == "super") type = TokenType::SUPER;
        else if (text == "this") type = TokenType::THIS;
        else if (text == "true") type = TokenType::TRUE;
        else if (text == "var") type = TokenType::VAR;
        else if (text == "while") type = TokenType::WHILE;
        else type = TokenType::IDENTIFIER;

        addToken(type);
    }

    bool isAlpha(char c) {
        return std::isalpha(c) || c == '_';
    }

    bool isAlphaNumeric(char c) {
        return isAlpha(c) || isDigit(c);
    }

    bool isDigit(char c) {
        return std::isdigit(c);
    }
};
