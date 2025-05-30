#pragma once
#include<string>
#include<memory>
#include<vector>
#include<sstream>
#include "Expr.hpp"
#include "token.hpp"

class Interpreter : public ExprVisitorEval {
public:

    literal evaluate(const Expr& expr) const{
        return expr.accept(*this);
    }

    literal visit(const Literal& expr) const override{
        return expr.value;
    }

    literal visit(const Grouping& expr) const override {
        return evaluate(*expr.expression);
    }

    literal visit(const Binary& expr) const override {
        literal left = evaluate(*expr.left);
        literal right = evaluate(*expr.right);
        switch(expr.op.getType()){
            case TokenType::PLUS:
                if(std::holds_alternative<double>(left) && std::holds_alternative<double>(right)){
                    return std::get<double>(left) + std::get<double>(right);
                }
                if(std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)){
                    return std::get<std::string>(left) + std::get<std::string>(right);
                }
                throw RuntimeError(expr.op, "Operands must be two numbers or two strings.");
            
            case TokenType::MINUS:
                checkNumberOperand(expr.op, {left, right});
                return std::get<double>(left) - std::get<double>(right);
            case TokenType::STAR:
                checkNumberOperand(expr.op, {left, right});
                return std::get<double>(left) * std::get<double>(right);
            case TokenType::SLASH:
                checkNumberOperand(expr.op, {left, right});
                return std::get<double>(left) / std::get<double>(right);
            case TokenType::EQUAL_EQUAL:
                return left == right;
            case TokenType::BANG_EQUAL:
                return left != right;
            case TokenType::GREATER:
                checkNumberOperand(expr.op, {left, right});
                return std::get<double>(left) > std::get<double>(right);
            case TokenType::GREATER_EQUAL:
                checkNumberOperand(expr.op, {left, right});
                return std::get<double>(left) >= std::get<double>(right);
            case TokenType::LESS:
                checkNumberOperand(expr.op, {left, right});
                return std::get<double>(left) < std::get<double>(right);
            case TokenType::LESS_EQUAL:
                checkNumberOperand(expr.op, {left, right});
                return std::get<double>(left) <= std::get<double>(right);
        }
        return std::monostate{};
    }

    literal visit(const Unary& expr) const override {
        literal right = evaluate(*expr.right);

        switch(expr.op.getType()){
            case TokenType::MINUS:
                checkNumberOperand(expr.op, {right});
                return -(std::get<double>(right));
            case TokenType::BANG:
                return !isTruthy(right);
        }  
        
        return std::monostate{};
    }

private:
    mutable std::ostringstream oss;

    class RuntimeError : public std::runtime_error {
    public:
        Token token;
        RuntimeError(const Token& token, const std::string& message)
            : std::runtime_error(message), token(token) {}
    };

    void checkNumberOperand(const Token& op, std::initializer_list<literal> operands) const {
        for(auto operand : operands){
            if(!std::holds_alternative<double>(operand)){
                throw RuntimeError(op, "Operands must be a number.");
                exit(70);
            }
        }
    }

    bool isTruthy(literal object) const {
        if(std::holds_alternative<std::monostate>(object)){
            return false;
        }
        if(std::holds_alternative<bool>(object)) return std::get<bool>(object);
        return true;
    }
};