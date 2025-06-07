#pragma once
#include "token.hpp"
#include "tokenizer.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

class Parser{
public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

    std::unique_ptr<Expr> parseExpr(){
        try{
            std::optional<std::unique_ptr<Expr>> expr = expression();
            if(expr){
                if(!isAtEnd()){
                    throw ParseError("Unexpected tokens after expression.");
                }
                return std::move(expr.value());
            }
        }catch(const ParseError&){
            synchronize();
        }
        return nullptr;
    }

    std::vector<std::unique_ptr<Stmt>> parse(){
        std::vector<std::unique_ptr<Stmt>> statements;
        while(!isAtEnd()){
            statements.push_back(declaration());
        }
        return statements;
    }

    class ParseError : public std::runtime_error {
    public:
        explicit ParseError(const std::string& message) : std::runtime_error(message) {}
    };

private:

    std::unique_ptr<Stmt> declaration(){
        try{
            if(match({TokenType::VAR})) return varDeclaration();
            auto stmtOpt = statement();
            if (stmtOpt) {
                return std::move(stmtOpt.value());
            }
            return nullptr;
        }catch(ParseError error){
            synchronize();
            throw;
        }
    }

    std::unique_ptr<Stmt> varDeclaration(){
        Token name = try_consume(TokenType::IDENTIFIER, "Expect variable name.");
        std::optional<std::unique_ptr<Expr>> initializer;
        if(match({TokenType::EQUAL})){
            initializer = expression();
            if(!initializer) {
                throw error(peek(), "Expect expression after '='.");
            }
        }
        try_consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
        if(!initializer) {
            initializer = std::make_unique<Literal>(std::monostate{});
        }
        return std::make_unique<Var>(name, std::move(initializer.value()));
    }

    std::optional<std::unique_ptr<Stmt>> statement(){
        if(match({TokenType::PRINT})) return printStatement();

        return expressionStatement();
    }

    std::optional<std::unique_ptr<Stmt>> printStatement(){
        std::optional<std::unique_ptr<Expr>> value = expression();
        try_consume(TokenType::SEMICOLON,"Expect ';' after value.");
        if(!value) return std::nullopt;
        return std::make_unique<Print>(std::move(value.value()));
    }

    std::optional<std::unique_ptr<Stmt>> expressionStatement(){
        std::optional<std::unique_ptr<Expr>> expr = expression();
        try_consume(TokenType::SEMICOLON,"Expect ';' after expression.");
        if(!expr) return std::nullopt;
        return std::make_unique<Expression>(std::move(expr.value()));
    }

    void synchronize(){
        consume();
        while(!isAtEnd()){
            if(previous().getType() == TokenType::SEMICOLON) return;
            switch(peek().getType()){
                case TokenType::CLASS:
                case TokenType::FUN:
                case TokenType::VAR:
                case TokenType::FOR:
                case TokenType::IF:
                case TokenType::WHILE:
                case TokenType::PRINT:
                case TokenType::RETURN:
                    return;
            }
            consume();
        }
    }

    std::optional<std::unique_ptr<Expr>> expression() {
        std::optional<std::unique_ptr<Expr>> expr = equality();
        if (expr) {
            return std::move(expr);
        }
        return std::nullopt;
    }

    std::optional<std::unique_ptr<Expr>> equality(){
        std::optional<std::unique_ptr<Expr>> expr = comparison();
        if(!expr) return std::nullopt;
        while(match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL})){
            Token op = previous();
            std::optional<std::unique_ptr<Expr>> right = comparison();
            if(!right) return std::nullopt;
            expr = std::make_unique<Binary>(std::move(expr.value()), op, std::move(right.value()));
        }
        return expr;
    }

    std::optional<std::unique_ptr<Expr>> comparison(){
        std::optional<std::unique_ptr<Expr>> expr = term();
        if(!expr) return std::nullopt;
        while(match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL})){
            Token op = previous();
            std::optional<std::unique_ptr<Expr>> right = term();
            if(!right) return std::nullopt;
            expr = std::make_unique<Binary>(std::move(expr.value()),op,std::move(right.value()));
        }
        return expr;
    }

    std::optional<std::unique_ptr<Expr>> term(){
        std::optional<std::unique_ptr<Expr>> expr = factor();
        if(!expr) return std::nullopt;
        while(match({TokenType::MINUS, TokenType::PLUS})){
            Token op = previous();
            std::optional<std::unique_ptr<Expr>> right = factor();
            if(!right) return std::nullopt;
            expr = std::make_unique<Binary>(std::move(expr.value()), op, std::move(right.value()));
        }
        return expr;
    }

    std::optional<std::unique_ptr<Expr>> factor(){
        std::optional<std::unique_ptr<Expr>> expr = unary();
        if(!expr) return std::nullopt;
        while(match({TokenType::SLASH, TokenType::STAR})){
            Token op = previous();
            std::optional<std::unique_ptr<Expr>> right = unary();
            if(!right) return std::nullopt;
            expr = std::make_unique<Binary>(std::move(expr.value()), op, std::move(right.value()));
        }
        return expr;
    }

    std::optional<std::unique_ptr<Expr>> unary(){
        if(match({TokenType::BANG, TokenType::MINUS})){
            Token op = previous();
            std::optional<std::unique_ptr<Expr>> right = unary();
            if(!right) return std::nullopt;
            return std::make_unique<Unary>(op, std::move(right.value()));
        }
        return primary();
    }

    std::optional<std::unique_ptr<Expr>> primary(){
        if(match({TokenType::TRUE})){
            return std::make_unique<Literal>(true);
        }
        if(match({TokenType::FALSE})){
            return std::make_unique<Literal>(false);
        }
        if(match({TokenType::NIL})){
            return std::make_unique<Literal>(std::monostate{});
        }
        if(match({TokenType::NUMBER, TokenType::STRING})){
            return std::make_unique<Literal>(previous().getLiteral());
        }
        if(match({TokenType::LEFT_PAREN})){
            std::optional<std::unique_ptr<Expr>> expr = expression();
            try_consume(TokenType::RIGHT_PAREN, "Expected ')' after expression.");
            if(!expr){
                return std::nullopt;
            }
            return std::make_unique<Grouping>(std::move(expr.value()));
        }
        if(match({TokenType::IDENTIFIER})){
            return std::make_unique<Literal>(previous().getLexme());
        }else{
            std::string lexme = peek().getLexme();
            if (lexme.empty()) lexme = "unknown";
            throw error(peek(), "Unexpected token: '" + lexme + "'. Expected an expression.");
        }
        return std::nullopt;
    }

    bool match(std::initializer_list<TokenType> types) {
        for (TokenType type : types) {
            if (check(type)) {
                consume();
                return true;
            }
        }
        return false;
    }

    bool check(TokenType type) const {
        if (isAtEnd()) return false;
        return tokens[current].getType() == type;
    }

    Token consume(){
        if(!isAtEnd()) current++;
        return previous();
    }

    Token try_consume(TokenType type, std::string errorMessage){
        if (check(type)) {
            return consume();
        }
        throw error(peek(), errorMessage);
    }

    ParseError error(const Token& token, std::string message){
        if (token.getType() == TokenType::END_OF_FILE) {
            return ParseError("[line " + std::to_string(token.getLine()) + "] Error at end: " + message);
        } else {
            return ParseError("[line " + std::to_string(token.getLine()) + "] Error at '" + token.getLexme() + "': " + message);
        }
    }

    bool isAtEnd() const {
        return current >= tokens.size() || tokens[current].getType() == TokenType::END_OF_FILE;
    }

    Token peek(){
        return tokens[current];
    }

    Token previous() const {
        return tokens[current - 1];
    }

    std::vector<Token> tokens;
    size_t current = 0;
};