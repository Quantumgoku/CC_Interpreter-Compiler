#pragma once
#include<string>
#include<memory>
#include<vector>
#include<sstream>
#include "Expr.hpp"
#include "token.hpp"

class Interpreter : public ExprVisitorEval {
public:
    std::string literal_to_string(const literal& value) const {
        if (std::holds_alternative<std::monostate>(value)) return "nil";
        if (std::holds_alternative<std::string>(value)) return std::get<std::string>(value);
        if (std::holds_alternative<double>(value)) {
            std::ostringstream oss;
            oss << std::get<double>(value);
            return oss.str();
        }
        if (std::holds_alternative<bool>(value)) return std::get<bool>(value) ? "true" : "false";
        return "";

    }

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
                throw std::runtime_error("Operands must be either number or string.....");
            
            case TokenType::MINUS:
                return std::get<double>(left) - std::get<double>(right);
            case TokenType::STAR:
                return std::get<double>(left) * std::get<double>(right);
            case TokenType::SLASH:
                return std::get<double>(left) / std::get<double>(right);
            case TokenType::EQUAL_EQUAL:
                return left == right;
            case TokenType::BANG_EQUAL:
                return left != right;
            case TokenType::GREATER:
                return std::get<double>(left) > std::get<double>(right);
            case TokenType::GREATER_EQUAL:
                return std::get<double>(left) >= std::get<double>(right);
            case TokenType::LESS:
                return std::get<double>(left) < std::get<double>(right);
            case TokenType::LESS_EQUAL:
                return std::get<double>(left) <= std::get<double>(right);
        }
        return std::monostate{};
    }

    literal visit(const Unary& expr) const override {
        literal right = evaluate(*expr.right);

        switch(expr.op.getType()){
            case TokenType::MINUS:
                if(!std::holds_alternative<double>(right)){
                    std::cerr << "Operand must be a number." << std::endl;
                    exit(70);
                }
                return -(std::get<double>(right));
            case TokenType::BANG:
                return !isTruthy(right);
        }  
        
        return std::monostate{};
    }

private:
    mutable std::ostringstream oss;

    bool isTruthy(literal object) const {
        if(std::holds_alternative<std::monostate>(object)){
            return false;
        }
        if(std::holds_alternative<bool>(object)) return std::get<bool>(object);
        return true;
    }
};